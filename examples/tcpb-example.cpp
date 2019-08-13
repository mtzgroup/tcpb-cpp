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
  int port = 54321;
  string xyzf("c2h4.xyz");
  string tcf("tc.template");

  TCPBClient TC(host, port);
  bool avail = TC.IsAvailable();
  printf("Server is available: %s\n", (avail ? "True" : "False"));

  vector<string> atoms;
  vector<double> geom;
  TCPBUtils::ReadXYZFile(xyzf, atoms, geom);
  
  int num_atoms = atoms.size();
  for (int i = 0; i < num_atoms; i++) {
    printf("%s %lf %lf %lf\n", atoms[i].c_str(), geom[3*i+0], geom[3*i+1], geom[3*i+2]);
  }

  map<string, string> options = TCPBUtils::ReadTCFile(tcf);
  printf("Options:\n");
  for (map<string, string>::iterator it = options.begin(); it != options.end(); ++it) {
    printf("%s: %s\n", it->first.c_str(), it->second.c_str());
  }

  string run = options["run"];
  options.erase("run");
  TCPBInput input(run, atoms, options, geom.data());
  printf("Debug protobuf string:\n%s\n", input.GetDebugString().c_str());

  TCPBInput input2(tcf);
  printf("Debug protobuf string:\n%s\n", input2.GetDebugString().c_str());

  //// Set up water system
  //int num_atoms = 3;
  //const char* atoms[3] = {"O", "H", "H"};
  //double geom[9] = {0.00000,  0.00000, -0.12948,
  //                  0.00000, -1.49419,  1.02744,
  //                  0.00000,  1.49419,  1.02744};
  //TC->SetAtoms(atoms, num_atoms);
  //TC->SetCharge(0);
  //TC->SetSpinMult(1);
  //TC->SetClosed(true);
  //TC->SetRestricted(true);
  //TC->SetMethod("pbe0");
  //TC->SetBasis("6-31g");

  //bool avail = TC->IsAvailable();
  //printf("Server is available: %s\n", (avail ? "True" : "False"));

  //double energy;
  //double* grad = new double[9];

  //TC->ComputeEnergy(geom, num_atoms, false, energy);
  //printf("H2O Energy: %lf\n", energy);

  //TC->ComputeGradient(geom, num_atoms, false, energy, grad);
  //printf("H2O Gradient:\n");
  //for (int i = 0; i < 3*num_atoms; i++) {
  //  printf("%lf ", grad[i]);
  //  if ((i+1)%3 == 0) printf("\n");
  //}

  //// Memory Management
  //delete TC; //Handles disconnect
  //delete grad;

  //return 0;
}
