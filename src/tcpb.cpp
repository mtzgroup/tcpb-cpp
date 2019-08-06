/** \file tcpb.cpp
 *  \brief Implementation of TCPBClient class
 *  \author Stefan Seritan <sseritan@stanford.edu>
 *  \date Jul 2017
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>

#include "socket.h"
#include "tcpb.h"
#include "terachem_server.pb.h"

using std::string;

TCPBClient::TCPBClient(string host,
                       int port) {
  socket_ = new TCPBSocket(host, port);
}

TCPBClient::~TCPBClient() {
  delete socket_;
}

/***********************
 * JOB INPUT (SETTERS) *
 ***********************/

/*
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
*/

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
  sendSuccess = socket_->HandleSend((char *)header, sizeof(header), "IsAvailable() status header");
  if (!sendSuccess) {
    printf("IsAvailable: Could not send status header\n");
    exit(1);
  }
  
  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "IsAvailable() status header");
  if (!recvSuccess) {
    printf("IsAvailable: Could not receive status header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "IsAvailable() status protobuf");
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
  /*
  if (!atomsSet) { printf("Called SendJobAsync() without SetAtoms()\n"); exit(1); }
  if (!chargeSet) { printf("Called SendJobAsync() without SetCharge()\n"); exit(1); }
  if (!spinMultSet) { printf("Called SendJobAsync() without SetSpinMult()\n"); exit(1); }
  if (!closedSet) { printf("Called SendJobAsync() without SetClosed()\n"); exit(1); }
  if (!restrictedSet) { printf("Called SendJobAsync() without SetRestricted()\n"); exit(1); }
  if (!methodSet) { printf("Called SendJobAsync() without SetMethod()\n"); exit(1); }
  if (!basisSet) { printf("Called SendJobAsync() without SetBasis()\n"); exit(1); }
  */

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
  sendSuccess = socket_->HandleSend((char *)header, sizeof(header), "SendJobAsync() job input header");
  if (!sendSuccess) {
    printf("SendJobAsync: Could not send job input header\n");
    exit(1);
  }
  
  if (msgSize) {
    char msg[msgSize];
    memcpy(msg, (void*)msgStr.data(), msgSize);
    sendSuccess = socket_->HandleSend(msg, msgSize, "SendJobAsync() job input protobuf");
    if (!sendSuccess) {
      printf("SendJobAsync: Could not send job input protobuf\n");
      exit(1);
    }
  }

  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "SendJobAsync() status header");
  if (!recvSuccess) {
    printf("SendJobAsync: Could not receive status header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "SendJobAsync() status protobuf");
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
  sendSuccess = socket_->HandleSend((char *)header, sizeof(header), "CheckJobComplete() status header");
  if (!sendSuccess) {
    printf("CheckJobComplete: Could not send status header\n");
    exit(1);
  }
  
  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "CheckJobComplete() status header");
  if (!recvSuccess) {
    printf("CheckJobComplete: Could not receive status header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "CheckJobComplete() status protobuf");
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
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "RecvJobAsync() job output header");
  if (!recvSuccess) {
    printf("RecvJobAsync: Could not receive job output header\n");
    exit(1);
  }

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "RecvJobAsync() job output protobuf");
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
