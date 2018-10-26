/** \file tcpb-example.cpp
 *  \brief Example of TCPBClient use
 *  \author Stefan Seritan <sseritan@stanford.edu>
 *  \date Aug 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "tcpb.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: %s host port\n", argv[0]);
  }

  int port = atoi(argv[2]);
  TCPBClient* TC = new TCPBClient(argv[1], port);

  TC->Connect();

  // Set up water system
  int num_atoms = 3;
  const char* atoms[3] = {"O", "H", "H"};
  double geom[9] = {0.00000,  0.00000, -0.12948,
                    0.00000, -1.49419,  1.02744,
                    0.00000,  1.49419,  1.02744};
  TC->SetAtoms(atoms, num_atoms);
  TC->SetCharge(0);
  TC->SetSpinMult(1);
  TC->SetClosed(true);
  TC->SetRestricted(true);
  TC->SetMethod("pbe0");
  TC->SetBasis("6-31g");

  bool avail = TC->IsAvailable();
  printf("Server is available: %s\n", (avail ? "True" : "False"));

  double energy;
  double* grad = new double[9];

  TC->ComputeEnergy(geom, num_atoms, false, energy);
  printf("H2O Energy: %lf\n", energy);

  TC->ComputeGradient(geom, num_atoms, false, energy, grad);
  printf("H2O Gradient:\n");
  for (int i = 0; i < 3*num_atoms; i++) {
    printf("%lf ", grad[i]);
    if ((i+1)%3 == 0) printf("\n");
  }

  // Memory Management
  delete TC; //Handles disconnect
  delete grad;

  return 0;
}
