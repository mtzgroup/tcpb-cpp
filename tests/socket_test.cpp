/** \file socket_test.cpp
 *  \brief Tests of Socket and derived classes
 */

#include <stdio.h>
#include <unistd.h>

#include <functional>
using std::function;
#include <stdexcept>
using std::exception;
#include <string>
using std::string;
#include <thread>
using std::thread;

#include "tcpb/socket.h"
using TCPB::Socket;
using TCPB::ClientSocket;
using TCPB::SelectServerSocket;

bool shutdown = false;
string host = "localhost";
int port = 12346;

void busyResponse(const Socket& client) {
  int buf;
  client.HandleRecv((char*)&buf, sizeof(buf), "int from client in busy response");
  printf("Receiving %d from client in busy response\n", buf);

  usleep(100000);

  buf = -1;
  printf("Sending %d to client in busy response\n", buf);
  client.HandleSend((char*)&buf, sizeof(buf), "int to client in busy response");
}

void ServerLoop() {
 int buf; 

  SelectServerSocket server(port, busyResponse);

  while ( !shutdown ) {
    Socket client = server.AcceptClient();

    client.HandleRecv((char*)&buf, sizeof(buf), "int from client");
    printf("Received %d from client\n", buf);
    
    buf++;
    usleep(100000);

    printf("Sending %d to client\n", buf);
    client.HandleSend((char*)&buf, sizeof(buf), "int to client");
    
    fflush(stdout);
  }
}

/**********/
// TESTS //
/**********/

bool testServerInit() {
  printf("Testing SelectServerSocket initialization...\n");

  try {
    SelectServerSocket server(port, busyResponse);
  } catch (exception& err) {
    printf("FAILED: %s\n", err.what());
    return false;
  }

  printf("SUCCESS\n");

  return true;
}

bool testSimpleClientServer() {
  printf("Testing simple one-to-one client server...\n");

  int buf = 1;

  shutdown = false;
  thread sthread(&ServerLoop);

  usleep(1000000); // Sleep 1 second
  ClientSocket client(host, port);

  try {
    client.HandleSend((char*)&buf, sizeof(int), "int to server");
    client.HandleRecv((char*)&buf, sizeof(int), "int from server");
  } catch (exception& err) {
    printf("Error in client send/recv: %s\n", err.what());
  }

  shutdown = true;
  sthread.join();

  if (buf != 2) {
    printf("FAILED. buf value is %d\n", buf);
    return false;
  }

  printf("SUCCESS\n");
  return true;
}

int main(int argc, char** argv) {
  int failed = 0;

  if (!testServerInit()) failed++;
  printf("---\n");

  if (!testSimpleClientServer()) failed++;
  printf("---\n");

  if (failed) {
    printf("\nFAILED %d SOCKET TESTS\n", failed);
  } else {
    printf("\nPASSED ALL SOCKET TESTS\n");
  }

  return failed;
}
