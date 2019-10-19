/** \file socket.cpp
 *  \brief Implementation of Socket class
 */

#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <algorithm>
using std::max;
#include <functional>
using std::function;
#include <mutex>
using std::lock_guard; using std::mutex;
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
#include <thread>
using std::thread;
#include <utility>
using std::swap;

//Socket includes
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#include "socket.h"

//#define SOCKETLOGS
#define MAX_STR_LEN 1024

namespace TCPB {

Socket::Socket(
  int sfd,
  const string& logName,
  bool cleanOnDestroy) :
  socket_(sfd),
  logFile_(NULL),
  cleanOnDestroy_(cleanOnDestroy)
{
  if (socket_ == -1) {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
  }

#ifdef SOCKETLOGS
  logFile_ = fopen(logName.c_str(), "a");
#endif
}

Socket::~Socket() {
  if (cleanOnDestroy_) {
    shutdown(socket_, SHUT_RDWR);
    close(socket_);
    SocketLog("Successfully closed socket %d", socket_);
  }

#ifdef SOCKETLOGS
  fclose(logFile_);
#endif
}

// Rule of 5 boiler plate
void Socket::swap(Socket& other) noexcept {
  using std::swap;
  swap(socket_, other.socket_);
  swap(logFile_, other.logFile_);
  swap(cleanOnDestroy_, other.cleanOnDestroy_);
}

Socket::Socket(Socket&& move) noexcept : Socket() { 
  move.swap(*this);
}

Socket& Socket::operator=(Socket&& move) noexcept {
  move.swap(*this);
  return *this;
}

bool Socket::HandleRecv(char* buf,
                        int len,
                        const char* log) const {
  int nrecv;

  // Try to recv
  nrecv = RecvN(buf, len);
  if (nrecv < 0) {
    if (errno == EINTR || errno == EAGAIN) {
      SocketLog("Packet read for %s on socket %d was interrupted, trying again\n", log, socket_);
      nrecv = RecvN(buf, len);
    }
  }

  if (nrecv < 0) {
    SocketLog("Could not properly recv packet for %s on socket %d. Errno: %d (%s)\n", log, socket_, errno, strerror(errno));
    return false;
  } else if (nrecv == 0) {
    SocketLog("Received shutdown signal for %s on socket %d\n", log, socket_);
    return false;
  } else if (nrecv != len) {
    SocketLog("Only recv'd %d bytes of %d expected bytes for %s on socket %d,\n", nrecv, len, log, socket_);
    return false;
  }
  
  SocketLog("Successfully recv'd packet of %d bytes for %s on socket %d\n", nrecv, log, socket_);
  return true;
}

bool Socket::HandleSend(const char* buf,
                        int len,
                        const char* log) const {
  int nsent;

  if (len == 0) {
    SocketLog("Trying to send packet of 0 length for %s on socket %d, skipping send\n", log, socket_);
    return true;
  }

  // Try to send
  nsent = SendN(buf, len);
  if (nsent < 0) {
    if (errno == EINTR || errno == EAGAIN) {
      SocketLog("Packet send for %s on socket %d was interrupted, trying again\n", log, socket_);
      nsent = SendN(buf, len);
    }
  }

  if (nsent <= 0) {
    SocketLog("Could not properly send packet for %s on socket %d. Errno: %d (%s)\n", log, socket_, errno, strerror(errno));
    return false;
  } else if (nsent != len) {
    SocketLog("Only sent %d bytes of %d expected bytes for %s on socket %d\n", nsent, len, log, socket_);
    return false;
  }
  
  SocketLog("Successfully sent packet of %d bytes for %s on socket %d\n", nsent, log, socket_);
  return true;
}

int Socket::RecvN(char* buf,
                  int len) const {
  int nleft, nrecv;

  nleft = len;
  while (nleft) {
    nrecv = recv(socket_, buf, len, 0);
    if (nrecv < 0) return nrecv;
    else if (nrecv == 0) break;

    nleft -= nrecv; 
    buf += nrecv;
  }

  return len - nleft;
}

int Socket::SendN(const char* buf,
                  int len) const {
  int nleft, nsent;

  nleft = len;
  while (nleft) {
    nsent = send(socket_, buf, len, 0);
    if (nsent < 0) return nsent;
    else if (nsent == 0) break;

    nleft -= nsent; 
    buf += nsent;
  }

  return len - nleft;
}

void Socket::SocketLog(const char* format, ...) const {
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
  fprintf(logFile_, "%.24s: %s\n", asctime(t), logStr);
  fflush(logFile_);

  va_end(args);
#endif
}

/***************
 * ClientSocket
 ***************/

ClientSocket::ClientSocket(const string& host, int port) :
  Socket(-1, "client.log", true)
{
  struct hostent* serverinfo;
  struct sockaddr_in serveraddr;
  struct timeval tv;

  // Set timeout to 15 seconds
  memset(&tv, 0, sizeof(timeval));
  tv.tv_sec = 15;
  if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    SocketLog("Could not set recv timeout to %d seconds", tv.tv_sec);
    throw runtime_error("Socket timeout setup failed for recv");
  }
  if (setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
    SocketLog("Could not set send timeout to %d seconds", tv.tv_sec);
    throw runtime_error("Socket timeout setup failed for send");
  }

  // Set up connection
  serverinfo = gethostbyname(host.c_str());
  if (serverinfo == NULL) {
    SocketLog("Could not lookup hostname %s", host.c_str());
    throw runtime_error("Could not lookup hostname");
  }
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  memcpy((char *)&serveraddr.sin_addr.s_addr, (char *)serverinfo->h_addr, serverinfo->h_length);
  serveraddr.sin_port = htons(port);

  // Connect
  if (connect(socket_, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
    SocketLog("Could not connect to host %s, port %d on socket %d", host.c_str(), port, socket_);
    throw runtime_error("Could not connect");
  }

  SocketLog("Successfully connected to host %s, port %d on socket %d", host.c_str(), port, socket_);
}

/***************
 * SelectServerSocket
 ***************/

SelectServerSocket::SelectServerSocket(int port, function<void(const Socket&)> replyCB) :
  Socket(-1, "server.log", true),
  NonactiveReplyCB_(replyCB),
  exitFlag_(false),
  accept_(false),
  activeClient_(-1)
{
  struct sockaddr_in listenaddr;

  // Try to bind port
  listenaddr.sin_family = AF_INET;
  listenaddr.sin_port = htons((uint16_t)port);
  listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(socket_, (struct sockaddr*)&listenaddr, sizeof(listenaddr)) < 0) {
    SocketLog("Could not bind socket %d for connections on port %d", socket_, port);
    throw runtime_error("Could not bind socket for connections");
  }

  // Open port for listening
  if (listen(socket_, 1) < 0) {
    SocketLog("Could not listen for connections on socket %d", socket_);
    throw runtime_error("Could not listen on socket for connections");
  }

  SocketLog("Successfully bound and listening on port %d with socket %d", port, socket_);

  // Launch select() loop
  listenThread_ = thread(&SelectServerSocket::RunSelectLoop, this);

  SocketLog("Successfully launched select() loop thread");
}

SelectServerSocket::~SelectServerSocket() {
  {
    lock_guard<mutex> guard(listenMutex_);
    exitFlag_ = true;
  }

  listenThread_.join();

  // Socket cleanup
  for (int i = 0; i < maxfd_; i++) {
    if (FD_ISSET(i, &activefds_)) {
      shutdown(i, SHUT_RDWR);
      close(i);
    }
  }
}

Socket SelectServerSocket::AcceptClient() {
  ReleaseClient();

  {
    lock_guard<mutex> guard(listenMutex_);
    accept_ = true;
  }

  bool localAccept = true;
  while ( localAccept ) {
    // Sleep and wait for select to populate a new client
    usleep(1e4); // 0.01 seconds (matching select timeout)
    
    lock_guard<mutex> guard(listenMutex_);
    localAccept = accept_;
  }

  // Return socket that logs with server socket and doesn't close socket on destruction
  return Socket(activeClient_, "server.log", false);
}

void SelectServerSocket::ReleaseClient() {
  lock_guard<mutex> guard(listenMutex_);
  activeClient_ = -1;
}

void SelectServerSocket::RunSelectLoop() {
  int newsock; // Socket to listen for connections, and new socket for accepting connections
  struct sockaddr_in clientaddr; // Address for new active client
  size_t size; //Used for sizeof(clientaddr) in accept()
  fd_set readfds, writefds; // Set of sockets that are active, reading, and writing, respectively
  // By default, all new sockets will be in the member activefds fd set, so that we can listen for status requests
  // At the start of each loop, readfds will be set to activefds (this is so that we don't try to read from a new connection)
  // I will only keep one socket in the writefds set, the client whose job we are currently processing
  struct timeval tv; //Timeout struct for select()
  int maxfd; // Local copy of maxfd

  // Set up file descriptor set
  FD_ZERO(&activefds_);
  FD_SET(socket_, &activefds_);
  maxfd_ = socket_ + 1;

  bool localExit = false;
  while ( !localExit ) {
    // Reset fd sets
    FD_ZERO(&readfds);
    FD_ZERO(&writefds); // Currently basically unused

    //Reset timeout for .1 second
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    {
      lock_guard<mutex> guard(listenMutex_);
      readfds = activefds_;
      maxfd = maxfd_;
    }

    // Block until action on any socket
    if (select(maxfd, &readfds, &writefds, NULL, &tv) < 0) {
      SocketLog("Error in select: %d (%s)", errno, strerror(errno));
      throw runtime_error("Error in select()");
    }

    // Loop through all sockets and handle reading and writing
    for (int i = 0; i < maxfd_; i++) {
      if (FD_ISSET(i, &readfds)) {
        lock_guard<mutex> guard(listenMutex_); // Generous guard for member variable touches

        if (i == socket_) { // We have activity on listening socket, must be a new connection
          size = sizeof(clientaddr);
          newsock = accept(socket_, (struct sockaddr*)&clientaddr, (socklen_t*)&size);
          if (newsock < 0) {
            SocketLog("Error in accepting connection from host %s, port %d", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
          } else {
            SocketLog("Accepting connection from host %s, port %d", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            FD_SET(newsock, &activefds_); //Set as active for next select, but do not read now
            maxfd_ = max(maxfd_, newsock + 1);
          }
        } else if (i != activeClient_) { // We have activity on a client socket
          if (accept_) {
            // Take this client as the new active since we are accepting
            activeClient_ = i;
            accept_ = false;
          } else {
            Socket s(i, "server.log", false);
            NonactiveReplyCB_(s); // Use callback to send right busy response
          }
        }
      } // End FD_ISSET(readfds) + mutex guard dropping out of scope
    } // End socket for loop
    
    {
      lock_guard<mutex> guard(listenMutex_);
      localExit = exitFlag_;
    }

    fflush(stdout);
  }
}

} // end namespace TCPB
