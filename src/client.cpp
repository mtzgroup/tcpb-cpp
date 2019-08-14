/** \file tcpb.cpp
 *  \brief Implementation of TCPBClient class
 */

#include <arpa/inet.h> // For htonl()/ntohl()
#include <string>
using std::string;
#include <unistd.h> //For sleep()

#include "exceptions.h"
#include "client.h"
#include "input.h"
#include "output.h"
#include "socket.h"
#include "terachem_server.pb.h"
using terachem_server::JobInput; using terachem_server::JobOutput;

TCPBClient::TCPBClient(string host,
                       int port) {
  host_ = host;
  port_ = port;
  socket_ = new TCPBSocket(host, port);

  currJobDir_ = "";
  currJobScrDir_ = "";
  currJobId_ = -1;

  prevResults_ = TCPBOutput(terachem_server::JobOutput());
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
  if (!sendSuccess) throw ServerError(
    "IsAvailable: Could not send status header",
    host_, port_, currJobDir_, currJobId_);
  
  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "IsAvailable() status header");
  if (!recvSuccess) throw ServerError(
    "IsAvailable: Could not recv status header",
    host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "IsAvailable() status protobuf");
    if (!recvSuccess) throw ServerError(
      "IsAvailable: Could not recv status protobuf",
      host_, port_, currJobDir_, currJobId_);
  }

  if (header[0] != terachem_server::STATUS) throw ServerError(
      "IsAvailable: Did not get the expected status message",
      host_, port_, currJobDir_, currJobId_);
  
  terachem_server::Status status;
  if (msgSize > 0) status.ParseFromString(msg);

  return !status.busy();
}

bool TCPBClient::SendJobAsync(const TCPBInput& input) {
  uint32_t header[2];
  bool sendSuccess, recvSuccess;
  int msgType, msgSize;
  string msgStr;
  const JobInput pb = input.GetInputPB();
  
  msgType = terachem_server::JOBINPUT;
  msgSize = pb.ByteSize();
  pb.SerializeToString(&msgStr);

  // Send JobInput Protocol Buffer
  header[0] = htonl((uint32_t)msgType);
  header[1] = htonl((uint32_t)msgSize);
  sendSuccess = socket_->HandleSend((char *)header, sizeof(header), "SendJobAsync() job input header");
  if (!sendSuccess) throw ServerError(
    "SendJobAsync: Could not send job input header",
    host_, port_, currJobDir_, currJobId_);
  
  if (msgSize) {
    char msg[msgSize];
    memcpy(msg, (void*)msgStr.data(), msgSize);
    sendSuccess = socket_->HandleSend(msg, msgSize, "SendJobAsync() job input protobuf");
    if (!sendSuccess) throw ServerError(
      "SendJobAsync: Could not send job input protobuf",
      host_, port_, currJobDir_, currJobId_);
  }

  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "SendJobAsync() status header");
  if (!recvSuccess) throw ServerError(
    "SendJobAsync: Could not recv status header",
    host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "SendJobAsync() status protobuf");
    if (!recvSuccess) throw ServerError(
      "SendJobAsync: Could not recv status protobuf",
      host_, port_, currJobDir_, currJobId_);
  }

  if (msgType != terachem_server::STATUS) throw ServerError(
      "SendJobAsync: Did not get the expected status message",
      host_, port_, currJobDir_, currJobId_);
  
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
  if (!sendSuccess) throw ServerError(
    "CheckJobComplete: Could not send status header",
    host_, port_, currJobDir_, currJobId_);
  
  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "CheckJobComplete() status header");
  if (!recvSuccess) throw ServerError(
    "CheckJobComplete: Could not recv status header",
    host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "CheckJobComplete() status protobuf");
    if (!recvSuccess) throw ServerError(
      "CheckJobComplete: Could not recv status protobuf",
      host_, port_, currJobDir_, currJobId_);
  }

  if (msgType != terachem_server::STATUS) throw ServerError(
    "CheckJobComplete:  Did not get the expected status message",
    host_, port_, currJobDir_, currJobId_);
  
  terachem_server::Status status;
  if (msgSize > 0) status.ParseFromString(msg);

  if (status.job_status_case() == terachem_server::Status::kWorkingFieldNumber) {
    return false;
  } else if (status.job_status_case() != terachem_server::Status::kCompletedFieldNumber) {
    throw ServerError("CheckJobComplete: No valid job status was received",
      host_, port_, currJobDir_, currJobId_);
  }

  return true;
}

const TCPBOutput TCPBClient::RecvJobAsync() {
  uint32_t header[2];
  bool recvSuccess;
  int msgType, msgSize;
  string msgStr;
  JobOutput pb;

  // Receive JobOutput Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header), "RecvJobAsync() job output header");
  if (!recvSuccess) throw ServerError(
    "RecvJobAsync: Could not recv job output header",
    host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg), "RecvJobAsync() job output protobuf");
    if (!recvSuccess) throw ServerError(
    "RecvJobAsync: Could not recv job output protobuf",
    host_, port_, currJobDir_, currJobId_);
  }

  if (msgType != terachem_server::JOBOUTPUT) {
    throw ServerError("RecvJobAsync: Did not get the expected job output message",
    host_, port_, currJobDir_, currJobId_);
  } else if (msgSize == 0) {
    throw ServerError("RecvJobAsync: Got empty job output message",
    host_, port_, currJobDir_, currJobId_);
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

  prevResults_ = RecvJobAsync();

  currJobDir_ = "";
  currJobScrDir_ = "";
  currJobId_ = -1;

  return prevResults_;
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

  output.GetEnergy(energy);

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

  output.GetEnergy(energy);
  output.GetGradient(gradient);

  return output;
}

const TCPBOutput TCPBClient::ComputeForces(const TCPBInput& input,
                                           double& energy,
                                           double* gradient) {
  // Compute energy and gradient
  TCPBOutput output = ComputeGradient(input, energy, gradient);

  // Flip sign on gradient
  const JobInput pb = input.GetInputPB();
  int num_atoms = pb.mol().atoms().size();
  for (int i = 0; i < 3*num_atoms; i++) {
    gradient[i] *= -1.0;
  }

  return output;
}
