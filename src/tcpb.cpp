/** \file tcpb.cpp
 *  \brief Implementation of TCPBClient class
 *  \author Stefan Seritan <sseritan@stanford.edu>
 *  \date Jul 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <time.h>
#include <stdarg.h>

//Socket includes
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#include "tcpb.h"
// terachem_server.pb.cpp/h must be generated from terachem_server.proto
#include "terachem_server.pb.h"

using namespace std;

// SOCKETLOGS gives the option to turn on detailed socket communication information
// Logs will be written to clientLogFile_, which is usually opened as client.log
#define SOCKETLOGS

TCPBClient::TCPBClient(const char* host,
                       int port) {
  snprintf(host_, MAX_STR_LEN, "%s", host);
  port_ = port;
  server_ = -1;

  atomsSet = false;
  chargeSet = false;
  spinMultSet = false;
  closedSet = false;
  restrictedSet = false;
  methodSet = false;
  basisSet = false;

#ifdef SOCKETLOGS
  clientLogFile_ = fopen("client.log", "w");
#endif
}

TCPBClient::~TCPBClient() {
  Disconnect();

#ifdef SOCKETLOGS
  fclose(clientLogFile_);
#endif
}

/***********************
 * JOB INPUT (SETTERS) *
 ***********************/

void TCPBClient::SetAtoms(const char** atoms,
                          const int num_atoms) {
  terachem_server::Mol* mol = jobInput_.mutable_mol();
  mol->clear_atoms();
  for (int i = 0; i < num_atoms; i++) {
    mol->add_atoms(atoms[i]);
  }
  atomsSet = true;

  // Clear MO coeffs
  jobInput_.clear_orb1afile();
  jobInput_.clear_orb1bfile();
}

void TCPBClient::SetCharge(const int charge) {
  terachem_server::Mol* mol = jobInput_.mutable_mol();
  mol->set_charge(charge);
  chargeSet = true;

  // Clear MO coeffs
  jobInput_.clear_orb1afile();
  jobInput_.clear_orb1bfile();
}

void TCPBClient::SetSpinMult(const int spinMult) {
  terachem_server::Mol* mol = jobInput_.mutable_mol();
  mol->set_multiplicity(spinMult);
  spinMultSet = true;

  // Clear MO coeffs
  jobInput_.clear_orb1afile();
  jobInput_.clear_orb1bfile();
}

void TCPBClient::SetClosed(const bool closed) {
  terachem_server::Mol* mol = jobInput_.mutable_mol();
  mol->set_closed(closed);
  closedSet = true;

  // Clear MO coeffs
  jobInput_.clear_orb1afile();
  jobInput_.clear_orb1bfile();
}

void TCPBClient::SetRestricted(const bool restricted) {
  terachem_server::Mol* mol = jobInput_.mutable_mol();
  mol->set_restricted(restricted);
  restrictedSet = true;

  // Clear MO coeffs
  jobInput_.clear_orb1afile();
  jobInput_.clear_orb1bfile();
}

void TCPBClient::SetMethod(const char* method) {
  bool valid;
  terachem_server::JobInput_MethodType methodType;

  // Convert to uppercase
  char* methodUpper = strdup(method);
  char* l = methodUpper;
  while (*l) {
    *l = toupper((unsigned char) *l);
    l++;
  }
  
  valid = jobInput_.MethodType_Parse(methodUpper, &methodType);
  if (!valid) {
    printf("Method %s passed to SetMethod() is not valid.\n", method);
    printf("Valid methods (case-insensitive):\n%s\n",
           jobInput_.MethodType_descriptor()->DebugString().c_str());
    exit(1);
  }

  jobInput_.set_method(methodType);
  methodSet = true;

  // Clear MO coeffs
  jobInput_.clear_orb1afile();
  jobInput_.clear_orb1bfile();

  free(methodUpper);
}

void TCPBClient::SetBasis(const char* basis) {
  //TODO: Should do same check as method with enum in .proto
  jobInput_.set_basis(basis);
  basisSet = true;

  // Clear MO coeffs
  jobInput_.clear_orb1afile();
  jobInput_.clear_orb1bfile();
}

/***********************
 * JOB INPUT (SETTERS) *
 ***********************/

void TCPBClient::GetEnergy(double& energy) {
  energy = jobOutput_.energy(0);
}

void TCPBClient::GetGradient(double* gradient) {
  int grad_size = jobOutput_.gradient_size();
  memcpy(gradient, jobOutput_.mutable_gradient()->mutable_data(), grad_size*sizeof(double));
}

/************************
 * SERVER COMMUNICATION *
 ************************/

bool TCPBClient::IsAvailable() {
  uint32_t header[2];
  int msgType, msgSize;
  bool sendSuccess, recvSuccess;

  // Send Status Protocol Buffer
  msgType = terachem_server::STATUS;
  msgSize = 0;
  header[0] = htonl((uint32_t)msgType);
  header[1] = htonl((uint32_t)msgSize);
  sendSuccess = HandleSend((char *)header, sizeof(header), "IsAvailable() status header");
  if (!sendSuccess) {
    printf("IsAvailable: Could not send status header\n");
    exit(1);
  }
  
  // Receive Status Protocol Buffer
  recvSuccess = HandleRecv((char *)header, sizeof(header), "IsAvailable() status header");
  if (!recvSuccess) {
    printf("IsAvailable: Could not receive status header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = HandleRecv(msg, sizeof(msg), "IsAvailable() status protobuf");
    if (!recvSuccess) {
      printf("IsAvailable: Could not receive status protobuf\n");
      exit(1);
    }
  }

  if (header[0] != terachem_server::STATUS) {
    printf("IsAvailable: Did not receive the expected status message\n");
    exit(1);
  }
  
  terachem_server::Status status;
  if (msgSize > 0) status.ParseFromString(msg);

  return !status.busy();
}

bool TCPBClient::SendJobAsync(const terachem_server::JobInput_RunType runType,
                              const double* geom,
                              const int num_atoms,
                              const terachem_server::Mol_UnitType unitType) {
  uint32_t header[2];
  bool sendSuccess, recvSuccess;
  int msgType, msgSize;
  string msgStr;

  // Sanity checks
  if (!atomsSet) { printf("Called SendJobAsync() without SetAtoms()\n"); exit(1); }
  if (!chargeSet) { printf("Called SendJobAsync() without SetCharge()\n"); exit(1); }
  if (!spinMultSet) { printf("Called SendJobAsync() without SetSpinMult()\n"); exit(1); }
  if (!closedSet) { printf("Called SendJobAsync() without SetClosed()\n"); exit(1); }
  if (!restrictedSet) { printf("Called SendJobAsync() without SetRestricted()\n"); exit(1); }
  if (!methodSet) { printf("Called SendJobAsync() without SetMethod()\n"); exit(1); }
  if (!basisSet) { printf("Called SendJobAsync() without SetBasis()\n"); exit(1); }

  // Finish up JobInput Protocol Buffer
  jobInput_.set_run(runType);

  terachem_server::Mol* mol = jobInput_.mutable_mol();
  mol->mutable_xyz()->Resize(3*num_atoms, 0.0);
  memcpy(mol->mutable_xyz()->mutable_data(), geom, 3*num_atoms*sizeof(double));
  mol->set_units(unitType);

  msgType = terachem_server::JOBINPUT;
  msgSize = jobInput_.ByteSize();
  jobInput_.SerializeToString(&msgStr);

  // Send JobInput Protocol Buffer
  header[0] = htonl((uint32_t)msgType);
  header[1] = htonl((uint32_t)msgSize);
  sendSuccess = HandleSend((char *)header, sizeof(header), "SendJobAsync() job input header");
  if (!sendSuccess) {
    printf("SendJobAsync: Could not send job input header\n");
    exit(1);
  }
  
  if (msgSize) {
    char msg[msgSize];
    memcpy(msg, (void*)msgStr.data(), msgSize);
    sendSuccess = HandleSend(msg, msgSize, "SendJobAsync() job input protobuf");
    if (!sendSuccess) {
      printf("SendJobAsync: Could not send job input protobuf\n");
      exit(1);
    }
  }

  // Receive Status Protocol Buffer
  recvSuccess = HandleRecv((char *)header, sizeof(header), "SendJobAsync() status header");
  if (!recvSuccess) {
    printf("SendJobAsync: Could not receive status header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = HandleRecv(msg, sizeof(msg), "SendJobAsync() status protobuf");
    if (!recvSuccess) {
      printf("SendJobAsync: Could not receive status protobuf\n");
      exit(1);
    }
  }

  if (msgType != terachem_server::STATUS) {
    printf("SendJobAsync: Did not receive the expected status message\n");
    exit(1);
  }
  
  terachem_server::Status status;
  if (msgSize > 0) status.ParseFromString(msg);

  if (status.job_status_case() != terachem_server::Status::kAcceptedFieldNumber) {
    return false;
  }

  return true;
}

bool TCPBClient::CheckJobComplete() {
  uint32_t header[2];
  int msgType, msgSize;
  bool sendSuccess, recvSuccess;
  string msgStr;

  // Send Status Protocol Buffer
  msgType = terachem_server::STATUS;
  msgSize = 0;

  header[0] = htonl((uint32_t)msgType);
  header[1] = htonl((uint32_t)msgSize);
  sendSuccess = HandleSend((char *)header, sizeof(header), "CheckJobComplete() status header");
  if (!sendSuccess) {
    printf("CheckJobComplete: Could not send status header\n");
    exit(1);
  }
  
  // Receive Status Protocol Buffer
  recvSuccess = HandleRecv((char *)header, sizeof(header), "CheckJobComplete() status header");
  if (!recvSuccess) {
    printf("CheckJobComplete: Could not receive status header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = HandleRecv(msg, sizeof(msg), "CheckJobComplete() status protobuf");
    if (!recvSuccess) {
      printf("CheckJobComplete: Could not receive status protobuf\n");
      exit(1);
    }
  }

  if (msgType != terachem_server::STATUS) {
    printf("CheckJobComplete: Did not receive the expected status message\n");
    exit(1);
  }
  
  terachem_server::Status status;
  if (msgSize > 0) status.ParseFromString(msg);

  if (status.job_status_case() == terachem_server::Status::kWorkingFieldNumber) {
    return false;
  } else if (status.job_status_case() != terachem_server::Status::kCompletedFieldNumber) {
    printf("CheckJobComplete: No valid job status was received\n");
    exit(1);
  }

  return true;
}

void TCPBClient::RecvJobAsync() {
  uint32_t header[2];
  int msgType, msgSize;
  string msgStr;
  bool recvSuccess;
  int aSize, bSize;

  // Receive JobOutput Protocol Buffer
  recvSuccess = HandleRecv((char *)header, sizeof(header), "RecvJobAsync() job output header");
  if (!recvSuccess) {
    printf("RecvJobAsync: Could not receive job output header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = HandleRecv(msg, sizeof(msg), "RecvJobAsync() job output protobuf");
    if (!recvSuccess) {
      printf("RecvJobAsync: Could not receive job output protobuf\n");
      exit(1);
    }
  }

  if (msgType != terachem_server::JOBOUTPUT) {
    printf("RecvJobAsync: Did not receive the expected job output message\n");
    exit(1);
  } else if (msgSize == 0) {
    printf("RecvJobAsync: Received empty job output message\n");
    exit(1);
  }

  // Cast char* to string, avoiding binary 0 being counted as null termination
  msgStr.resize(msgSize);
  memcpy((void*)msgStr.data(), msg, msgSize);
  
  // Overwrite Job Output Protocol Buffer
  jobOutput_.ParseFromString(msgStr);
  //printf("Received job output:\n%s\n", jobOutput_.DebugString().c_str());

  // Save MO coeffs
  jobInput_.set_orb1afile(jobOutput_.orb1afile());
  jobInput_.set_orb1bfile(jobOutput_.orb1bfile());
}

void TCPBClient::ComputeJobSync(const terachem_server::JobInput_RunType runType,
                                const double* geom,
                                const int num_atoms,
                                const terachem_server::Mol_UnitType unitType) {
  // Try to submit job
  while (!SendJobAsync(runType, geom, num_atoms, unitType)) {
    //Sleep for 0.1 second 
    //usleep(1000000);
  }

  // Check for job completion
  while (!CheckJobComplete()) {
    //Sleep for 0.1 second 
    //usleep(1000000);
  }

  RecvJobAsync();
}

/*************************
 * CONVENIENCE FUNCTIONS *
 *************************/

void TCPBClient::ComputeEnergy(const double* geom,
                               const int num_atoms,
                               const bool angstrom,
                               double& energy) {
  // Run energy job
  terachem_server::JobInput_RunType runType = terachem_server::JobInput::ENERGY;
  terachem_server::Mol_UnitType unitType;
  if (angstrom) {
    unitType = terachem_server::Mol::ANGSTROM;
  } else {
    unitType = terachem_server::Mol::BOHR;
  }
  ComputeJobSync(runType, geom, num_atoms, unitType);

  // Extract energy from jobOutput_
  GetEnergy(energy);
}

void TCPBClient::ComputeGradient(const double* geom,
                                 const int num_atoms,
                                 const bool angstrom,
                                 double& energy,
                                 double* gradient) {
  // Run gradient job
  terachem_server::JobInput_RunType runType = terachem_server::JobInput::GRADIENT;
  terachem_server::Mol_UnitType unitType;
  if (angstrom) {
    unitType = terachem_server::Mol::ANGSTROM;
  } else {
    unitType = terachem_server::Mol::BOHR;
  }
  ComputeJobSync(runType, geom, num_atoms, unitType);

  // Extract energy and gradient from jobOutput_
  GetEnergy(energy);
  GetGradient(gradient);
}

void TCPBClient::ComputeForces(const double* geom,
                               const int num_atoms,
                               const bool angstrom,
                               double& energy,
                               double* gradient) {
  // Compute energy and gradient
  ComputeGradient(geom, num_atoms, angstrom, energy, gradient);

  // Flip sign on gradient
  for (int i = 0; i < 3*num_atoms; i++) {
    gradient[i] *= -1.0;
  }
}

/***************************
 * SOCKET HELPER FUNCTIONS *
 ***************************/

void TCPBClient::Connect() {
  struct hostent* serverinfo;
  struct sockaddr_in serveraddr;
  struct timeval tv;

  server_ = socket(AF_INET, SOCK_STREAM, 0);

  // Set timeout to 15 seconds
  memset(&tv, 0, sizeof(timeval));
  tv.tv_sec = 15;
  if (setsockopt(server_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    SocketLog("Could not set recv timeout to %d seconds", tv.tv_sec);
    exit(1);
  }
  if (setsockopt(server_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
    SocketLog("Could not set send timeout to %d seconds", tv.tv_sec);
    exit(1);
  }

  // Set up connection
  serverinfo = gethostbyname(host_);
  if (serverinfo == NULL) {
    SocketLog("Could not lookup hostname %s", host_);
    exit(1);
  }
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  memcpy((char *)&serveraddr.sin_addr.s_addr, (char *)serverinfo->h_addr, serverinfo->h_length);
  serveraddr.sin_port = htons(port_);

  // Connect
  if (connect(server_, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
    SocketLog("Could not connect to host %s, port %d on socket %d", host_, port_, server_);
    exit(1);
  }
}

void TCPBClient::Disconnect() {
  shutdown(server_, SHUT_RDWR);
  close(server_);
  server_ = -1;
}


bool TCPBClient::HandleRecv(char* buf,
                            int len,
                            const char* log) {
  int nrecv;

  // Try to recv
  nrecv = RecvN(buf, len);
  if (nrecv < 0) {
    if (errno == EINTR || errno == EAGAIN) {
      SocketLog("Packet read for %s on socket %d was interrupted, trying again\n", log, server_);
      nrecv = RecvN(buf, len);
    }
  }

  if (nrecv < 0) {
    SocketLog("Could not properly recv packet for %s on socket %d, closing socket. Errno: %d (%s)\n", log, server_, errno, strerror(errno));
    Disconnect();
    return false;
  } else if (nrecv == 0) {
    SocketLog("Received shutdown signal for %s on socket %d, closing socket\n", log, server_);
    Disconnect();
    return false;
  } else if (nrecv != len) {
    SocketLog("Only recv'd %d bytes of %d expected bytes for %s on socket %d, closing socket\n", nrecv, len, log, server_);
    Disconnect();
    return false;
  }
  
  SocketLog("Successfully recv'd packet of %d bytes for %s on socket %d\n", nrecv, log, server_);
  return true;
}

bool TCPBClient::HandleSend(const char* buf,
                            int len,
                            const char* log) {
  int nsent;

  if (len == 0) {
    SocketLog("Trying to send packet of 0 length for %s on socket %d, skipping send\n", log, server_);
    return true;
  }

  // Try to send
  nsent = SendN(buf, len);
  if (nsent < 0) {
    if (errno == EINTR || errno == EAGAIN) {
      SocketLog("Packet send for %s on socket %d was interrupted, trying again\n", log, server_);
      nsent = SendN(buf, len);
    }
  }

  if (nsent <= 0) {
    SocketLog("Could not properly send packet for %s on socket %d, closing socket. Errno: %d (%s)\n", log, server_, errno, strerror(errno));
    Disconnect();
    return false;
  } else if (nsent != len) {
    SocketLog("Only sent %d bytes of %d expected bytes for %s on socket %d, closing socket\n", nsent, len, log, server_);
    Disconnect();
    return false;
  }
  
  SocketLog("Successfully sent packet of %d bytes for %s on socket %d\n", nsent, log, server_);
  return true;
}

int TCPBClient::RecvN(char* buf,
                      int len) {
  int nleft, nrecv;

  nleft = len;
  while (nleft) {
    nrecv = recv(server_, buf, len, 0);
    if (nrecv < 0) return nrecv;
    else if (nrecv == 0) break;

    nleft -= nrecv; 
    buf += nrecv;
  }

  return len - nleft;
}

int TCPBClient::SendN(const char* buf,
                      int len) {
  int nleft, nsent;

  nleft = len;
  while (nleft) {
    nsent = send(server_, buf, len, 0);
    if (nsent < 0) return nsent;
    else if (nsent == 0) break;

    nleft -= nsent; 
    buf += nsent;
  }

  return len - nleft;
}

void TCPBClient::SocketLog(const char* format, ...) {
#ifdef SOCKETLOGS
  // Get time info
  time_t now = time(NULL);
  struct tm* t = localtime(&now);

  // Get full log string from variable arguments
  va_list args;
  va_start(args, format);
  char logStr[MAX_STR_LEN];
  vsnprintf(logStr, MAX_STR_LEN, format, args);

  // Print to logfile with timestamp
  fprintf(clientLogFile_, "%.24s: %s\n", asctime(t), logStr);
  fflush(clientLogFile_);

  va_end(args);
#endif
}
