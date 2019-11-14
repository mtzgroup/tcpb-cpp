/** \file server.cpp
 *  \brief Implementation of TCPB::Server class
 */

#include <arpa/inet.h> // For htonl()/ntohl()
#include <mutex>
using std::lock_guard;
using std::mutex;
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
#include <sys/stat.h> // For mkdir()
#include <time.h> // For strftime()
#include <unistd.h> //For sleep(), dup()

#include "exceptions.h"
#include "server.h"
#include "input.h"
using TCPB::Input;
#include "output.h"
using TCPB::Output;
#include "socket.h"
using TCPB::Socket;
using TCPB::SelectServerSocket;
#include "terachem_server.pb.h"
using terachem_server::JobInput;
using terachem_server::JobOutput;
using terachem_server::Status;

namespace TCPB {

Server::Server(int port) : SelectServerSocket(port),
  stdoutFD_(dup(STDOUT_FILENO)),
  currJobSFD_(-1),
  currJobId_(0),
  currInput_(NULL),
  currOutput_(NULL),
  acceptJob_(false),
  jobCompleted_(false)
{
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  char buffer[100];
  strftime(buffer, 80, "%F-%T", timeinfo);

  char cwd[MAX_STR_LEN];
  getcwd(cwd, MAX_STR_LEN);

  snprintf(serverDir_, MAX_STR_LEN, "%s/server_%d_%s", cwd, port, buffer);
  if (mkdir(serverDir_, S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH | S_IXOTH) !=
    0) { // mkdir with 777 permissions
    throw runtime_error("Failed to make server directory");
  }

  //Overwrite default server logfile
#ifdef SOCKETLOGS
  fclose(logFile_);
  char logName[MAX_STR_LEN];
  snprintf(logName, MAX_STR_LEN, "%s/server.log", serverDir_);
  logFile_ = fopen(logName, "a");
#endif
}

Server::~Server()
{
  if (currInput_ != NULL) {
    delete currInput_;
  }
  if (currOutput_ != NULL) {
    delete currOutput_;
  }
}


const Input Server::RecvJobInput()
{
  // Wait for select() thread to populate currInput_
  acceptJob_ = true;
  while (acceptJob_) {
    usleep(selectSleep_);
  }

  // Make job directory
  currJobId_++;
  snprintf(currJobDir_, MAX_STR_LEN, "%s/job_%d", serverDir_, currJobId_);
  if (mkdir(currJobDir_, S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH | S_IXOTH) !=
    0) { // mkdir with 777 permissions
    throw runtime_error("Failed to make job directory");
  }

  // Redirect stdout
  char jobLog[MAX_STR_LEN];
  snprintf(jobLog, MAX_STR_LEN, "%s/%d.log", currJobDir_, currJobId_);
  fflush(stdout);
  freopen(jobLog, "w", stdout);

  lock_guard<mutex> guard(listenMutex_);
  return *currInput_;
}

void Server::SendJobOutput(const Output &out)
{
  {
    lock_guard<mutex> guard(listenMutex_);
    // Skip sending if client is no longer active
    if (FD_ISSET(currJobSFD_, &activefds_)) {
      currJobSFD_ = -1;
      return;
    }

    currOutput_ = new Output(out);
  }

  // Wait for select() to send currOutput_
  jobCompleted_ = true;
  while (currJobSFD_ != -1) {
    usleep(selectSleep_);
  }

  // Redirect stdout back
  fflush(stdout);
  dup2(stdoutFD_, STDOUT_FILENO);

  lock_guard<mutex> guard(listenMutex_);
  delete currOutput_;
  currOutput_ == NULL;
}

bool Server::HandleClientMessage(int sfd)
{
  char handleLog[MAX_STR_LEN];
  snprintf(handleLog, MAX_STR_LEN, "%s/server_handler.log", serverDir_);
  Socket client(sfd, handleLog, false);

  uint32_t header[2];
  terachem_server::MessageType msgType;
  int msgSize;

  if (!client.HandleRecv((char *)header, sizeof(header),
      "HandleMessage() header")) {
    return false;
  }
  msgType = (terachem_server::MessageType)ntohl(header[0]);
  msgSize = ntohl(header[1]);

  if (acceptJob_ && currJobSFD_ == -1
    && msgType == terachem_server::MessageType::JOBINPUT) {

    char msg[msgSize];
    if (!client.HandleRecv((char *)msg, sizeof(msg), "HandleMessage() protobuf")) {
      return false;
    }

    // Cast char* to string, but I want to avoid null character termination
    string msgStr;
    msgStr.resize(msgSize);
    memcpy((void *)msgStr.data(), msg, msgSize);

    terachem_server::JobInput input;
    if (!input.ParseFromString(msgStr)) {
      return false;
    }
    {
      lock_guard<mutex> guard(listenMutex_);
      if (currInput_ != NULL) {
        delete currInput_;
      }
      currInput_ = new Input(input);
    }

    // Accepted job, set state accordingly
    currJobSFD_ = sfd;
    acceptJob_ = false;

    // Send back acceptance
    if(!SendStatus(client, Status::kAcceptedFieldNumber)) {
      return false;
    }
  } else if (jobCompleted_ && currJobSFD_ == sfd) { // Send Output
    // Reset job, even if output send fails
    currJobSFD_ = -1;
    jobCompleted_ = false;

    // Send back status to get client ready
    Status status;
    if (!SendStatus(client, Status::kCompletedFieldNumber)) {
      return false;
    }

    // Send output
    string msgStr;
    const terachem_server::JobOutput output = currOutput_->GetOutputPB();
    output.SerializeToString(&msgStr);
    msgSize = output.ByteSize();

    header[0] = htonl((uint32_t)terachem_server::JOBOUTPUT);
    header[1] = htonl((uint32_t)msgSize);

    if (!client.HandleSend((char *)header, sizeof(header), "output header")) {
      return false;
    }

    char outMsg[msgSize];
    memcpy(outMsg, (void *)msgStr.data(), msgSize);
    if (!client.HandleSend(outMsg, msgSize, "output protobuf")) {
      return false;
    }
  } else {
    // Send a status message
    int status = 1; // Default busy
    if (acceptJob_ && currJobSFD_ == -1) {
      status = 0;
    } else if (sfd == currJobSFD_) {
      status = Status::kWorkingFieldNumber;
    }

    if (!SendStatus(client, status)) {
      return false;
    }
  }

  return true;
}

bool Server::SendStatus(const Socket &client, int code)
{
  Status status;
  string msgStr;
  int msgSize;

  status.set_busy(true);
  switch (code) {
  case 0:
    status.set_busy(false);
    break;
  case 1:
    break;
  case Status::kAcceptedFieldNumber:
    status.set_accepted(true);
    break;
  case Status::kWorkingFieldNumber:
    status.set_working(true);
    break;
  case Status::kCompletedFieldNumber:
    status.set_completed(true);
    break;
  default:
    throw runtime_error("Improper code to SendStatus");
  }

  if (code > 1) {
    char scrDir[MAX_STR_LEN];
    snprintf(scrDir, MAX_STR_LEN, "%s/scr");

    status.set_job_dir(currJobDir_);
    status.set_job_scr_dir(scrDir);
    status.set_server_job_id(currJobId_);
  }

  status.SerializeToString(&msgStr);
  msgSize = status.ByteSize();

  // Send header
  uint32_t header[2];
  header[0] = htonl((uint32_t)terachem_server::STATUS);
  header[1] = htonl((uint32_t)msgSize);
  if (!client.HandleSend((char *)header, 2 * sizeof(uint32_t), "status header")) {
    return false;
  }

  // Send protobuf
  char msg[msgSize];
  memcpy(msg, (void *)msgStr.data(), msgSize);
  if (!client.HandleSend(msg, msgSize, "status protobuf")) {
    return false;
  }

  return true;
}

} // end namespace TCPB
