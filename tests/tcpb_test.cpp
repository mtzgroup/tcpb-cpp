/** \file tcpb_test.cpp
 *  \brief Tests of Client and Server classes
 */

#include <stdio.h>
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
#include <thread>
using std::thread;

#include "tcpb/client.h"
using TCPB::Client;
#include "tcpb/input.h"
using TCPB::Input;
#include "tcpb/output.h"
using TCPB::Output;
#include "tcpb/server.h"
using TCPB::Server;

const string host = "localhost";
const int port = 54321;

// Mock server thread
void RunServer(const Input &ref_in,
  const Output &ref_out,
  int port)
{
  Server server(port);

  bool failed = false;

  while (true) {
    const Input in = server.RecvJobInput();

    // TODO: Figure out how to verify server side
    // if (!ref_in.IsApproxEqual(in)) {
    //   string errMsg = "FAILED. Ref Input:\n" + ref_in.GetDebugString();
    //   errMsg += "Recv'd Input:\n" + in.GetDebugString();
    //   throw runtime_error(errMsg);
    // }

    server.SendJobOutput(ref_out);
  }
}

/**********/
// TESTS //
/**********/

bool testAvailable()
{
  printf("Testing Client::IsAvailable()... ");
  fflush(stdout);

  Client client(host, port);

  if (!client.IsAvailable()) {
    printf("FAILED. IsAvailable() returned busy\n");
    return false;
  }

  printf("SUCCESS\n");
  return true;
}

bool testSingleClient(const Input &ref_in, const Output &ref_out)
{
  printf("Testing single Client::ComputeJobSync()... ");
  fflush(stdout);

  Client client(host, port);

  const Output out = client.ComputeJobSync(ref_in);

  if (!ref_out.IsApproxEqual(out)) {
    printf("FAILED. Ref Output:\n%s\nRecv'd Output:\n%s\n",
      ref_out.GetDebugString().c_str(), out.GetDebugString().c_str());
    return false;
  }

  printf("SUCCESS\n");
  return true;
}


int main(int argc, char **argv)
{
  int failed = 0;

  // Create reference
  Input in("input/tc.template");
  Output out;
  out.SetEnergy(42.0);

  thread sthread = thread(&RunServer, in, out, port);

  // Run tests
  if (!testAvailable()) {
    failed++;
  }
  if (!testSingleClient(in, out)) {
    failed++;
  }

  // Manually kill server
  sthread.std::thread::~thread();

  if (failed) {
    printf("FAILED %d TCPB CLIENT/SERVER TESTS\n\n", failed);
  } else {
    printf("PASSED ALL TCPB CLIENT/SERVER TESTS\n\n");
  }

  return failed;
}
