/** \file client.cpp
 *  \brief Implementation of TCPB::Client class
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
using terachem_server::JobInput;
using terachem_server::JobOutput;
using terachem_server::Status;

namespace TCPB {

Client::Client(string host,
  int port)
{
  host_ = host;
  port_ = port;
  socket_ = new ClientSocket(host, port);

  currJobDir_ = "";
  currJobScrDir_ = "";
  currJobId_ = -1;

  prevResults_ = Output(terachem_server::JobOutput());
}

Client::~Client()
{
  delete socket_;
}

/************************
 * SERVER COMMUNICATION *
 ************************/

bool Client::IsAvailable()
{
  uint32_t header[2];
  int msgType, msgSize;
  string msgStr;
  bool sendSuccess, recvSuccess;

  // Send Status Protocol Buffer
  msgType = terachem_server::STATUS;
  msgSize = 0;
  header[0] = htonl((uint32_t)msgType);
  header[1] = htonl((uint32_t)msgSize);
  sendSuccess = socket_->HandleSend((char *)header, sizeof(header),
      "IsAvailable() status header");
  if (!sendSuccess) throw ServerCommError(
      "IsAvailable: Could not send status header",
      host_, port_, currJobDir_, currJobId_);

  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header),
      "IsAvailable() status header");
  if (!recvSuccess) throw ServerCommError(
      "IsAvailable: Could not recv status header",
      host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg),
        "IsAvailable() status protobuf");
    if (!recvSuccess) throw ServerCommError(
        "IsAvailable: Could not recv status protobuf",
        host_, port_, currJobDir_, currJobId_);
  }

  if (header[0] != terachem_server::STATUS) throw ServerCommError(
      "IsAvailable: Did not get the expected status message",
      host_, port_, currJobDir_, currJobId_);

  Status status;
  if (msgSize > 0) {
    msgStr.resize(msgSize);
    memcpy((void *)msgStr.data(), msg, msgSize);
    status.ParseFromString(msgStr);
  }

  return !status.busy();
}

bool Client::SendJobAsync(const Input &input)
{
  uint32_t header[2];
  bool sendSuccess, recvSuccess;
  int msgType, msgSize;
  string msgStr;
  const JobInput pb = input.GetPB();

  msgType = terachem_server::JOBINPUT;
  msgSize = pb.ByteSize();
  pb.SerializeToString(&msgStr);

  // Send JobInput Protocol Buffer
  header[0] = htonl((uint32_t)msgType);
  header[1] = htonl((uint32_t)msgSize);
  sendSuccess = socket_->HandleSend((char *)header, sizeof(header),
      "SendJobAsync() job input header");
  if (!sendSuccess) throw ServerCommError(
      "SendJobAsync: Could not send job input header",
      host_, port_, currJobDir_, currJobId_);

  if (msgSize) {
    char msg[msgSize];
    memcpy(msg, (void *)msgStr.data(), msgSize);
    sendSuccess = socket_->HandleSend(msg, msgSize,
        "SendJobAsync() job input protobuf");
    if (!sendSuccess) throw ServerCommError(
        "SendJobAsync: Could not send job input protobuf",
        host_, port_, currJobDir_, currJobId_);
  }

  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header),
      "SendJobAsync() status header");
  if (!recvSuccess) throw ServerCommError(
      "SendJobAsync: Could not recv status header",
      host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg),
        "SendJobAsync() status protobuf");
    if (!recvSuccess) throw ServerCommError(
        "SendJobAsync: Could not recv status protobuf",
        host_, port_, currJobDir_, currJobId_);
  }

  if (msgType != terachem_server::STATUS) throw ServerCommError(
      "SendJobAsync: Did not get the expected status message",
      host_, port_, currJobDir_, currJobId_);

  Status status;
  if (msgSize > 0) {
    msgStr.resize(msgSize);
    memcpy((void *)msgStr.data(), msg, msgSize);
    status.ParseFromString(msgStr);
  }

  if (status.job_status_case() != Status::JobStatusCase::kAccepted) {
    return false;
  }

  currJobDir_ = status.job_dir();
  currJobScrDir_ = status.job_scr_dir();
  currJobId_ = status.server_job_id();

  return true;
}

bool Client::CheckJobComplete()
{
  uint32_t header[2];
  int msgType, msgSize;
  bool sendSuccess, recvSuccess;
  string msgStr;

  // Send Status Protocol Buffer
  msgType = terachem_server::STATUS;
  msgSize = 0;

  header[0] = htonl((uint32_t)msgType);
  header[1] = htonl((uint32_t)msgSize);
  sendSuccess = socket_->HandleSend((char *)header, sizeof(header),
      "CheckJobComplete() status header");
  if (!sendSuccess) throw ServerCommError(
      "CheckJobComplete: Could not send status header",
      host_, port_, currJobDir_, currJobId_);

  // Receive Status Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header),
      "CheckJobComplete() status header");
  if (!recvSuccess) throw ServerCommError(
      "CheckJobComplete: Could not recv status header",
      host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg),
        "CheckJobComplete() status protobuf");
    if (!recvSuccess) throw ServerCommError(
        "CheckJobComplete: Could not recv status protobuf",
        host_, port_, currJobDir_, currJobId_);
  }

  if (msgType != terachem_server::STATUS) throw ServerCommError(
      "CheckJobComplete:  Did not get the expected status message",
      host_, port_, currJobDir_, currJobId_);

  Status status;
  if (msgSize > 0) {
    msgStr.resize(msgSize);
    memcpy((void *)msgStr.data(), msg, msgSize);
    status.ParseFromString(msgStr);
  }

  if (status.job_status_case() == Status::JobStatusCase::kWorking) {
    return false;
  } else if (status.job_status_case() != Status::JobStatusCase::kCompleted) {
    throw ServerCommError("CheckJobComplete: No valid job status was received",
      host_, port_, currJobDir_, currJobId_);
  }

  return true;
}

const Output Client::RecvJobAsync()
{
  uint32_t header[2];
  bool recvSuccess;
  int msgType, msgSize;
  string msgStr;
  JobOutput pb;

  // Receive JobOutput Protocol Buffer
  recvSuccess = socket_->HandleRecv((char *)header, sizeof(header),
      "RecvJobAsync() job output header");
  if (!recvSuccess) throw ServerCommError(
      "RecvJobAsync: Could not recv job output header",
      host_, port_, currJobDir_, currJobId_);

  msgType = ntohl(header[0]);
  msgSize = ntohl(header[1]);

  char msg[msgSize];
  if (msgSize > 0) {
    recvSuccess = socket_->HandleRecv(msg, sizeof(msg),
        "RecvJobAsync() job output protobuf");
    if (!recvSuccess) throw ServerCommError(
        "RecvJobAsync: Could not recv job output protobuf",
        host_, port_, currJobDir_, currJobId_);
  }

  if (msgType != terachem_server::JOBOUTPUT) {
    throw ServerCommError("RecvJobAsync: Did not get the expected job output message",
      host_, port_, currJobDir_, currJobId_);
  } else if (msgSize == 0) {
    throw ServerCommError("RecvJobAsync: Got empty job output message",
      host_, port_, currJobDir_, currJobId_);
  }

  // Cast char* to string, avoiding binary 0 being counted as null termination
  msgStr.resize(msgSize);
  memcpy((void *)msgStr.data(), msg, msgSize);

  pb.ParseFromString(msgStr);
  //printf("Received job output:\n%s\n", pb.DebugString().c_str());

  return Output(pb);
}

const Output Client::ComputeJobSync(const Input &input)
{
  // Try to submit job
  if (!SendJobAsync(input)) throw ServerCommError(
    "ComputeJobSync: problem to submit the job",
    host_, port_, currJobDir_, currJobId_);

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

const Output Client::ComputeEnergy(const Input &input,
  double &energy)
{
  // Reset runtype to energy
  Input new_input(input);
  terachem_server::JobInput pb = new_input.GetPB();
  pb.set_run(terachem_server::JobInput::ENERGY);

  Output output = ComputeJobSync(new_input);

  output.GetEnergy(energy);

  return output;
}

const Output Client::ComputeGradient(const Input &input,
  double &energy,
  double *qmgradient,
  double *mmgradient)
{
  // Reset runtype to gradient
  Input new_input(input);
  terachem_server::JobInput pb = new_input.GetPB();
  pb.set_run(terachem_server::JobInput::GRADIENT);

  // Get target state, if needed
  int state = 0;
  std::string targetkeyword = "";
  for (int i = 0; i < pb.user_options_size()/2; ++i) {
    //printf(" %d: %s = %s\n", i, pb.user_options(2*i).c_str(), pb.user_options(2*i+1).c_str());
    if ((pb.user_options(2*i) == "casscf" || pb.user_options(2*i) == "casci") && pb.user_options(2*i+1) == "yes") {
      targetkeyword = "castarget";
      break;
    } else if (pb.user_options(2*i) == "cis" && pb.user_options(2*i+1) == "yes") {
      targetkeyword = "cistarget";
      break;
    }
  }
  if (targetkeyword.size() > 0) {
    for (int i = 0; i < pb.user_options_size()/2; ++i) {
      if (pb.user_options(2*i) == targetkeyword) {
        state = std::stoi(pb.user_options(2*i+1));
        break;
      }
    }
  }

  Output output = ComputeJobSync(new_input);

  output.GetEnergy(energy,state);
  output.GetGradient(qmgradient,mmgradient);

  return output;
}

const Output Client::ComputeForces(const Input &input,
  double &energy,
  double *qmgradient,
  double *mmgradient)
{
  // Compute energy and gradient
  Output output = ComputeGradient(input, energy, qmgradient, mmgradient);

  // Flip sign on gradient
  const JobInput pb = input.GetPB();
  int num_qm_atoms = pb.mol().atoms().size();
  for (int i = 0; i < 3 * num_qm_atoms; i++) {
    qmgradient[i] *= -1.0;
  }
  int num_mm_atoms = pb.mmatom_charge().size();
  for (int i = 0; i < 3 * num_mm_atoms; i++) {
    mmgradient[i] *= -1.0;
  }

  return output;
}

} // end namespace TCPB
