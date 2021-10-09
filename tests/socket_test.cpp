/** \file socket_test.cpp
 *  \brief Tests of Socket and derived classes
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <functional>
using std::function;
#include <future>
using std::future;
#include <stdexcept>
using std::exception;
#include <string>
using std::string;
#include <thread>
using std::thread;
using std::async;
#include <vector>
using std::vector;

#include "tcpb/socket.h"
using TCPB::Socket;
using TCPB::ClientSocket;
using TCPB::SelectServerSocket;

string host = "localhost";
int port = 12346;

class IncrementServer : public SelectServerSocket {
public:
  IncrementServer(int port) : SelectServerSocket(port) {}

private:
  bool HandleClientMessage(int sfd) {
    Socket client(sfd, "server_handle.log", false);

    int buf;
    if (!client.HandleRecv((char *)&buf, sizeof(buf), "int from client")) {
      return false;
    }

    buf++;

    if (!client.HandleSend((char *)&buf, sizeof(buf), "int to client")) {
      return false;
    }

    return true;
  }
};

// Assume this will be called on std::async
int ClientRun(int start, int loop)
{
  int buf;
  int val = start;
  ClientSocket client(host, port);
  for (int i = 0; i < loop; ++i) {
    client.HandleSend((char *)&val, sizeof(int), "int to server");
    client.HandleRecv((char *)&val, sizeof(int), "int from server");

    // Sleep some short amount of time to mix up server requests
    usleep(rand() % 1000);
  }

  return val;
}

/**********/
// TESTS //
/**********/

bool testSimpleClientServer()
{
  printf("Testing simple one-to-one client server... ");
  fflush(stdout);

  IncrementServer server(port);

  int start = rand() % 100;
  int loops = rand() % 8 + 2;

  future<int> val = async(&ClientRun, start, loops);

  if (val.get() != start + loops) {
    printf("FAILED. buf value is %d\n", val.get());
    return false;
  }

  printf("SUCCESS\n");
  return true;
}

bool testMultiClientServer()
{
  printf("Testing multiple clients to one server... ");
  fflush(stdout);

  IncrementServer server(port);

  int start = rand() % 100;
  int loops = rand() % 8 + 2;
  int nthreads = rand() % 8 + 2;

  vector<future<int>> vals;
  for (int i = 0; i < nthreads; ++i) {
    vals.push_back(async(std::launch::async, &ClientRun, start, loops));
  }

  for (int i = 0; i < nthreads; ++i) {
    int val = vals[i].get();
    if (val != start + loops) {
      printf("FAILED. buf value is %d\n for thread %d\n", val, i);
      return false;
    }
  }

  printf("SUCCESS\n");
  return true;
}

int main(int argc, char **argv)
{
  int failed = 0;
  printf("TEST1: %d\n",failed);

  srand(time(NULL));

  if (!testSimpleClientServer()) {
    failed++;
  }
  printf("TEST2: %d\n",failed);
  if (!testMultiClientServer()) {
    failed++;
  }
  printf("TEST3: %d\n",failed);

  if (failed) {
    printf("FAILED %d SOCKET TESTS\n\n", failed);
  } else {
    printf("PASSED ALL SOCKET TESTS\n\n");
  }

  return failed;
}
