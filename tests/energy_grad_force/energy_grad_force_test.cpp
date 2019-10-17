/** \file energy_grad_force_test.cpp
 *  \brief Test convenience functions (also tests input/output objects)
 */

#include <map>
using std::map;
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "tcpb/client.h"
using TCPB::Client;
#include "tcpb/input.h"
using TCPB::Input;
#include "tcpb/output.h"
using TCPB::Output;
#include "tcpb/utils.h"

bool FuzzyEqual(const double* arr1,
                const double* arr2,
                const int arrSize,
                const double tol) {
  bool fail = false;

  for (int i = 0; i < arrSize; i++) {
    if (abs(arr1[i] - arr2[i]) > tol) {
      fail = true;
      break;
    }
  }

  return !fail;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: %s host port\n", argv[0]);
  }
  string host(argv[1]);
  int port = atoi(argv[2]);
  string xyzf("h2o.xyz");
  string tcf("tc.template");

  Client TC(host, port);

  // Creating Input 1: Explicitly processing files separately (full control over options)
  vector<string> atoms;
  vector<double> geom;
  TCPB::Utils::ReadXYZFile(xyzf, atoms, geom);

  // Truncate for matching old protobuf
  // Needed because there is no fuzzy equals in the binary PB comparison
  double scale = 0.00001;
  for (int i = 0; i < 3*atoms.size(); ++i) {
    double sign = ( geom[i] > 0.0 ? 1.0 : -1.0);
    geom[i] = (int)(fabs(geom[i]) / scale + 0.5) * scale * sign;
  }

  map<string, string> options = TCPB::Utils::ReadTCFile(tcf);
  string run = options["run"];
  options.erase("run");
  options.erase("coordinates");

  Input input(run, atoms, options, geom.data());

  // Creating Input 2: Fully parsing from TC input
  Input input2(tcf);

  // Variables for output
  int num_atoms = input.GetInputPB().mol().atoms_size();
  double energy;
  double* grad = new double[3*num_atoms];
  double* forces = new double[3*num_atoms];

  // Expected outputs
  double tol = 1e-5;
  double expected_energy = -76.300505;
  double expected_grad[9] = {  0.0000002903,    0.0000000722,   -0.033101313,
                              -0.0000000608,   -0.0141756697,    0.016550727,
                              -0.0000002294,    0.0141755976,    0.016550585};

  // Most general compute call
  const Output output = TC.ComputeJobSync(input);
  output.GetEnergy(energy);

  if (!FuzzyEqual(&energy, &expected_energy, 1, tol)) {
    printf("Failed energy test\n");
    printf("Expected energy: %lf\nGot energy (by accessor): %lf\n", expected_energy, energy);
    return 1;
  }

  // Reset for ComputeGradient() test
  energy = 0.0;
  memset(grad, 0.0, 3*num_atoms*sizeof(double));

  const Output output2 = TC.ComputeGradient(input, energy, grad);
  if (!FuzzyEqual(&energy, &expected_energy, 1, tol) ||
      !FuzzyEqual(grad, expected_grad, 3*num_atoms, tol)) {
    printf("Failed gradient test\n");
    printf("Expected energy: %lf\nGot energy (by reference): %lf\n", expected_energy, energy);
    printf("Expected gradient: \n");
    for (int i = 1; i < num_atoms; i++) {
      printf("%lf %lf %lf\n", expected_grad[i], expected_grad[i+1], expected_grad[i+2]);
    }
    printf("Got gradient (by reference): \n");
    for (int i = 1; i < num_atoms; i++) {
      printf("%lf %lf %lf\n", grad[i], grad[i+1], grad[i+2]);
    }
    return 1;
  }
 
  // Reset for ComputeForces() test
  energy = 0.0;
  memset(grad, 0.0, 3*num_atoms*sizeof(double));
  memset(forces, 0.0, 3*num_atoms*sizeof(double));

  const Output output3 = TC.ComputeForces(input, energy, forces);
  output3.GetGradient(grad);
  for (int i = 0; i < 3*num_atoms; i++) {
    forces[i] *= -1.0;
  }
  if (!FuzzyEqual(&energy, &expected_energy, 1, tol) ||
      !FuzzyEqual(grad, expected_grad, 3*num_atoms, tol) ||
      !FuzzyEqual(forces, expected_grad, 3*num_atoms, tol)) {
    printf("Failed force test\n");
    printf("Expected energy: %lf\nGot energy (by reference): %lf\n", expected_energy, energy);
    printf("Expected gradient: \n");
    for (int i = 1; i < num_atoms; i++) {
      printf("%lf %lf %lf\n", expected_grad[i], expected_grad[i+1], expected_grad[i+2]);
    }
    printf("Got gradient (-1*force, by reference): \n");
    for (int i = 1; i < num_atoms; i++) {
      printf("%lf %lf %lf\n", forces[i], forces[i+1], forces[i+2]);
    }
    printf("Got gradient (by accessor): \n");
    for (int i = 1; i < num_atoms; i++) {
      printf("%lf %lf %lf\n", grad[i], grad[i+1], grad[i+2]);
    }
    return 1;
  }

  // Memory Management
  // TCPBClient will disconnect as it drops off the stack
  delete grad;
  delete forces;

  return 0;
}
