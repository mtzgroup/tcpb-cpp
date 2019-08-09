/** \file tcpb.cpp
 *  \brief Implementation of TCPBClient class
 */

#include <arpa/inet.h>
//#include <errno.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "socket.h"
#include "tcpb.h"
#include "tcpbinput.h"
#include "tcpboutput.h"

using std::string, std::vector, std::map;

TCPBClient::TCPBClient(string host,
                       int port) {
  socket_ = new TCPBSocket(host, port);
}

TCPBClient::~TCPBClient() {
  delete socket_;
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

bool TCPBClient::SendJobAsync(string run,
                              const vector<double>& atoms,
                              const map<string, string>& options,
                              const double* const geom,
                              const double* const geom2) {
  uint32_t header[2];
  bool sendSuccess, recvSuccess;
  int msgType, msgSize;
  string msgStr;
  int numAtoms = atoms.size();
  terachem_server::Mol* mol = jobInput_.mutable_mol();

  // Runtype
  terachem_server::JobInput_RunType runtype;
  bool valid = jobInput_.RunType_Parse(run.upper().c_str(), &runtype);
  if (!valid) {
    printf("Runtype %s passed in SendJobAsync() is not valid.\n", run);
    printf("Valid runtypes (case-insensitive):\n%s\n",
           jobInput_.RunType_descriptor()->DebugString().c_str());
    exit(1);
  }
  jobInput_.set_run(runtype);

  // Geometry and atoms
  mol->mutable_xyz()->Resize(3*numAtoms, 0.0);
  memcpy(mol->mutable_xyz()->mutable_data(), geom, 3*numAtoms*sizeof(double));

  for (int i = 0; i < numAtoms; i++) {
    mol->add_atoms(atoms[i].c_str());
  }

  // Units
  if (options.count("units")) {
    terachem_server::Mol_UnitType units;
    bool valid = mol.UnitType_Parse(options["units"].upper().c_str(), &units);
    if (!valid) {
      printf("Units %s passed in options map is not valid.\n", options["units"]);
      printf("Valid units (case-insensitive):\n%s\n",
             mol.UnitType_descriptor()->DebugString().c_str());
      exit(1);
    }
    mol->set_units(units);

  } else {
    mol->set_units(terachem_server::Mol::BOHR);
  }

  // Handle protocol-specific required keywords
  try {
    int charge = std::stoi(options.at("charge"));
    mol->set_charge(charge);
    options.erase("charge");

    int spinmult = std::stoi(options.at("spinmult"));
    mol->set_multiplicity(spinmult);
    options.erase("spinmult");

    bool closed_shell = (options.at("closed_shell").lower().compare("true") == 0);
    mol->set_closed(closed_shell);
    options.erase("closed_shell");

    bool restricted = (options.at("restricted").lower().compare("true") == 0);
    mol->set_restricted(restricted);
    options.erase("restricted");

    terachem_server::JobInput_MethodType method;
    bool valid_method = jobInput_.MethodType_Parse(options.at("method").upper().c_str(), &method);
    if (!valid) {
      printf("Method %s passed in options map is not valid.\n", options.at("method"));
      printf("Valid methods (case-insensitive):\n%s\n",
             jobInput_.MethodType_descriptor()->DebugString().c_str());
      exit(1);
    }
    jobInput_.set_method(method);
    options.erase("method");

    string basis = options.at("basis");
    jobInput_.set_basis(basis.c_str());
    options.erase("basis");
  } catch (const std::out_of_range& oor) {
    printf("Missing a required keyword in options map:\n");
    printf("charge, spinmult, closed_shell, restricted, method, basis\n");
    printf("Out-of-range error: %s\n", oor.what().c_str());
    exit(1);
  }

  // Optional protocol-specific keywords
  if (options.count("bond_order")) {
    if (options["bond_order"].lower().compare("true") == 0) {
      jobInput_.set_return_bond_order(true);
    }
    options.erase("bond_order");
  }
  if (geom2 != NULL) {
    mol->mutable_xyz2()->Resize(3*numAtoms, 0.0);
    memcpy(mol->mutable_xyz2()->mutable_data(), geom2, 3*numAtoms*sizeof(double));
  }

  // All other options are passed straight through to TeraChem
  for (map<string,string>::iterator it=options.begin(); it != options.end(); ++it) {
    jobInput_.add_user_options(it->first);
    jobInput_.add_user_options(it->second);
  }

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
}

bool TCPBClient::ComputeJobSync(string run,
                                const vector<double>& atoms,
                                const map<string, string>& options,
                                const double* const geom,
                                const double* const geom2) {
  // Try to submit job
  while (!SendJobAsync(runType, geom, num_atoms, unitType)) {
    sleep(1);
  }

  // Check for job completion
  while (!CheckJobComplete()) {
    sleep(1);
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
