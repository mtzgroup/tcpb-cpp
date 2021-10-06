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
#include <unistd.h>

#include "tcpb/client.h"
#include "tcpb/input.h"
#include "tcpb/output.h"
#include "tcpb/utils.h"

#define BohrToAng 0.52917724924

int main(int argc, char** argv) {
  string host("localhost");
  int port = 12345;
  string tcf("tc.template");

  TCPB::Client* TC = nullptr;

  try {
    // TCPB::Client TC(host, port);
    TC = new TCPB::Client(host, port);
  }
  catch (...) {
    printf("Could not connect to the host!\n");
    return 1;
  }
  bool avail = TC->IsAvailable();
  printf("Server is available: %s\n", (avail ? "True" : "False"));

  // Explicity setting all input variables
  double qmcoords[9] {-4.4798000,         -2.8400000,          4.2456000,
             -4.8525000,         -3.7649000,          4.3951000,
             -3.6050000,         -2.7568000,          4.9264000 };
  for (int i = 0; i<9; i++) {
    qmcoords[i] /= BohrToAng;
  }
  vector<string> qmattypes = {"O","H","H"};
  int numMMAtoms = 15;
  double mmpositions[45] {   -2.6793000,         -2.1596000,          5.9264000,
    -1.7944000,         -2.5941000,          6.0208000,
    -2.4543000,         -1.2247000,          5.9247000,
    -6.0739000,         -0.8812700,          5.2104000,
    -5.3910000,         -1.5014000,          4.7942000,
    -5.4189000,         -0.3240900,          5.9375000,
    -4.0898000,         -5.6279000,          2.9956000,
    -4.6091000,         -5.6876000,          2.2341000,
    -4.1166000,         -6.5262000,          3.2888000,
    -2.3448000,         -2.6425000,          1.8190000,
    -2.7846000,         -3.1506000,          2.6164000,
    -1.5986000,         -3.2938000,          1.7252000,
    -4.6456000,         -4.4223000,          7.4705000,
    -3.6650000,         -4.5356000,          7.1235000,
    -4.9759000,         -3.5580000,          7.3041000 };
  for (int i = 0; i<numMMAtoms; i++) {
    mmpositions[i] /= BohrToAng;
  }
  double mmcharges[15] { -0.834	,
  	0.417	,
  	0.417	,
  	-0.834	,
  	0.417	,
  	0.417	,
  	-0.834	,
  	0.417	,
  	0.417	,
  	-0.834	,
  	0.417	,
  	0.417	,
  	-0.834	,
  	0.417	,
  	0.417	 };

  // Set TeraChem input options
  map<string, string> options = TCPB::Utils::ReadTCFile(tcf);
  options.erase("coordinates");
  options.erase("pointcharges");
  options.erase("run");
  options.emplace("run","gradient");
  options.erase("pointcharges_self_interaction");
  options.emplace("pointcharges_self_interaction","false");
  // Print options
  //for(map<string, string>::const_iterator it = options.begin(); it != options.end(); ++it)
  //{
  //  std::cout << it->first << " " << it->second << "\n";
  //}

  TCPB::Input input(qmattypes, options, qmcoords, nullptr, mmpositions, mmcharges, numMMAtoms);
  printf("Debug protobuf input string:\n%s\n", input.GetDebugString().c_str());

  // Parameters can be pulled out of protobuf directly
  // In this case, we are using the repeated field's <field>_size() function
  // For more docs, check out https://developers.google.com/protocol-buffers/docs/reference/cpp-generated
  int num_qm_atoms = input.GetInputPB().mol().atoms_size();
  int num_mm_atoms = input.GetInputPB().mmatom_charges_size();

  double energy;
  double* qmgrad = new double[3*num_qm_atoms];
  double* mmgrad = new double[3*num_mm_atoms];

  // Compute call
  const TCPB::Output output = TC->ComputeGradient(input, energy, qmgrad, mmgrad);

  printf("Debug protobuf output string:\n%s\n", output.GetDebugString().c_str());

  printf("From ComputeGradient 'input' getters:\n");
  printf("Energy: %lf\n", energy);

  printf("QM gradient:\n");
  for (int i = 0; i < 3*num_qm_atoms; i++) {
    printf("%lf ", qmgrad[i]);
    if ((i+1)%3 == 0) printf("\n");
  }

  printf("MM gradient:\n");
  for (int i = 0; i < 3*num_mm_atoms; i++) {
    printf("%lf ", mmgrad[i]);
    if ((i+1)%3 == 0) printf("\n");
  }

  printf("\nDone!.\n\n");

  // Convenience compute calls to pull base properties out
  energy = 0.0;
  memset(qmgrad, 0.0, 3*num_qm_atoms*sizeof(double));
  memset(mmgrad, 0.0, 3*num_mm_atoms*sizeof(double));

  // Memory Management
  // TCPBClient will disconnect as it drops off the stack
  delete qmgrad;
  delete mmgrad;
  if (TC != nullptr)
    delete TC;

  return 0;
}
