/** \file tcpb-example.cpp
 *  \brief Example of TCPBClient use
 */

#include <map>
using std::map;
#include <stdio.h>
#include <stdlib.h>
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "tcpb/client.h"
#include "tcpb/input.h"
#include "tcpb/output.h"
#include "tcpb/utils.h"

int main(int argc, char** argv) {
  string host("localhost");
  int port = 12345;
  string tcf("tc.template");

  TCPB::Client TC(host, port);
  bool avail = TC.IsAvailable();
  printf("Server is available: %s\n", (avail ? "True" : "False"));

  // Creating Input 1: Explicitly processing files separately (full control over options)
  vector<string> atoms;
  vector<double> geom;
  string xyzf("c2h4.xyz");
  TCPB::Utils::ReadXYZFile(xyzf, atoms, geom);

  map<string, string> options = TCPB::Utils::ReadTCFile(tcf);
  string coords = options["coordinates"];
  options.erase("coordinates");

  TCPB::Input input(atoms, options, geom.data());
  printf("Debug protobuf string-1:\n%s\n", input.GetDebugString().c_str());

  // Creating Input 2: Fulling parsing from TC input
  string exyzf("");
  TCPB::Input input2(tcf,xyzf,exyzf);
  printf("Debug protobuf string-2:\n%s\n", input2.GetDebugString().c_str());

  // Parameters can be pulled out of protobuf directly
  // In this case, we are using the repeated field's <field>_size() function
  // For more docs, check out https://developers.google.com/protocol-buffers/docs/reference/cpp-generated
  int num_atoms = input.GetInputPB().mol().atoms_size();

  double energy;
  double* grad = new double[3*num_atoms];

  // Compute call
  const TCPB::Output output = TC.ComputeGradient(input, energy, grad);

  // Convenience accessors after job
  //output.GetEnergy(energy);
  //output.GetGradient(grad);

  //printf("From TCPB::Output getters:\n");
  //printf("Energy: %lf\n", energy);

  //printf("Gradient:\n");
  //for (int i = 0; i < 3*num_atoms; i++) {
  //  printf("%lf ", grad[i]);
  //  if ((i+1)%3 == 0) printf("\n");
  //}
  //printf("\nDone with TCPB::Output getters.\n\n");

  // Convenience compute calls to pull base properties out
  energy = 0.0;
  memset(grad, 0.0, 3*num_atoms*sizeof(double));

  const TCPB::Output output2 = TC.ComputeGradient(input2, energy, grad);

  printf("From ComputeGradient call:\n");
  printf("Energy: %lf\n", energy);

  printf("Gradient:\n");
  for (int i = 0; i < 3*num_atoms; i++) {
    printf("%lf ", grad[i]);
    if ((i+1)%3 == 0) printf("\n");
  }

  // Memory Management
  // TCPBClient will disconnect as it drops off the stack
  delete grad;

  return 0;
}
