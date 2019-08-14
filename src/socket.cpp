/** \file socket.cpp
 *  \brief Implementation of TCPBSocket class
 */

#include <cstdarg>
#include <cstring>
#include <ctime>
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;

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


TCPBSocket::TCPBSocket(string host,
                       int port) {
  host_ = host;
  port_ = port;
  server_ = -1;

  Connect();
}

TCPBSocket::~TCPBSocket() {
  Disconnect();
}

void TCPBSocket::Connect() {
  struct hostent* serverinfo;
  struct sockaddr_in serveraddr;
  struct timeval tv;

  server_ = socket(AF_INET, SOCK_STREAM, 0);

#ifdef SOCKETLOGS
  logFile_ = fopen("socket.log", "a");
#endif

  // Set timeout to 15 seconds
  memset(&tv, 0, sizeof(timeval));
  tv.tv_sec = 15;
  if (setsockopt(server_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    SocketLog("Could not set recv timeout to %d seconds", tv.tv_sec);
    throw runtime_error("Socket timeout setup failed for recv");
  }
  if (setsockopt(server_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
    SocketLog("Could not set send timeout to %d seconds", tv.tv_sec);
    throw runtime_error("Socket timeout setup failed for send");
  }

  // Set up connection
  serverinfo = gethostbyname(host_.c_str());
  if (serverinfo == NULL) {
    SocketLog("Could not lookup hostname %s", host_.c_str());
    throw runtime_error("Could not lookup hostname");
  }
  memset(&serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  memcpy((char *)&serveraddr.sin_addr.s_addr, (char *)serverinfo->h_addr, serverinfo->h_length);
  serveraddr.sin_port = htons(port_);

  // Connect
  if (connect(server_, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
    SocketLog("Could not connect to host %s, port %d on socket %d", host_.c_str(), port_, server_);
    throw runtime_error("Could not connect");
  }
}

void TCPBSocket::Disconnect() {
  shutdown(server_, SHUT_RDWR);
  close(server_);
  server_ = -1;

#ifdef SOCKETLOGS
  fclose(logFile_);
#endif
}

bool TCPBSocket::HandleRecv(char* buf,
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

bool TCPBSocket::HandleSend(const char* buf,
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

int TCPBSocket::RecvN(char* buf,
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

int TCPBSocket::SendN(const char* buf,
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

void TCPBSocket::SocketLog(const char* format, ...) {
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
