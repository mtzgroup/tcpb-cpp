/** \file tcpb.cpp
 *  \brief Implementation of TCPBClient class
 */

#include <arpa/inet.h> // For htonl()/ntohl()
#include <stdio.h>
#include <stdlib.h>
#include <string>
using std::string;

#include "socket.h"
#include "client.h"
#include "input.h"
#include "output.h"



TCPBClient::TCPBClient(string host,
                       int port) {
  socket_ = new TCPBSocket(host, port);
  currJobDir_ = "";
  currJobScrDir_ = "";
  currJobId_ = -1;
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

bool TCPBClient::SendJobAsync(const TCPBInput& input) {
  uint32_t header[2];
  bool sendSuccess, recvSuccess;
  int msgType, msgSize;
  string msgStr;
  JobInput pb = input.GetInputPB();
  
  msgType = terachem_server::JOBINPUT;
  msgSize = pb.ByteSize();
  pb.SerializeToString(&msgStr);

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

  currJobDir_ = status.job_dir();
  currJobScrDir_ = status.job_scr_dir();
  currJobId_ = status.server_job_id();

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

const TCPBOutput TCPBClient::RecvJobAsync() {
  uint32_t header[2];
  int msgType, msgSize;
  string msgStr;
  JobOutput pb;

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
  
  pb.ParseFromString(msgStr);
  //printf("Received job output:\n%s\n", pb.DebugString().c_str());

  return TCPBOutput(pb);
}

const TCPBOutput TCPBClient::ComputeJobSync(const TCPBInput& input) {
  // Try to submit job
  while (!SendJobAsync(input)) {
    sleep(1);
  }

  // Check for job completion
  while (!CheckJobComplete()) {
    sleep(1);
  }

  TCPBOutput output = RecvJobAsync();

  currJobDir_ = "";
  currJobScrDir_ = "";
  currJobId_ = -1;

  return output;
}

/*************************
 * CONVENIENCE FUNCTIONS *
 *************************/

const TCPBOutput TCPBClient::ComputeEnergy(const TCPBInput& input,
                                           double& energy) {
  // Reset runtype to energy
  TCPBInput new_input(input);
  terachem_server::JobInput pb = new_input.GetInputPB();
  pb.set_run(terachem_server::JobInput::ENERGY);

  TCPBOutput output = ComputeJobSync(new_input);

  energy = output.GetEnergy();

  return output;
}

const TCPBOutput TCPBClient::ComputeGradient(const TCPBInput& input,
                                             double& energy,
                                             double* gradient) {
  // Reset runtype to gradient
  TCPBInput new_input(input);
  terachem_server::JobInput pb = new_input.GetInputPB();
  pb.set_run(terachem_server::JobInput::GRADIENT);

  TCPBOutput output = ComputeJobSync(new_input);

  energy = output.GetEnergy();
  gradient = output.GetGradient();

  return output;
}

const TCPBOutput TCPBClient::ComputeForces(const TCPBInput& input,
                                           double& energy,
                                           double* gradient) {
  // Compute energy and gradient
  TCPBOutput output = ComputeGradient(input, energy, gradient);

  // Flip sign on gradient
  for (int i = 0; i < 3*num_atoms; i++) {
    gradient[i] *= -1.0;
  }

  return output;
}
