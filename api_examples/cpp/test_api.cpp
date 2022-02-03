/** \file test_api.cpp
 *  \brief Example of API use
 */

#include <stdio.h>

#include <tcpb/api.h>

#define BohrToAng 0.52917724924

int main(int argc, char** argv) {

  double totenergy;

  // Set information about the server
  char host[80] = "localhost";
  int port = 12345;

  // Other input variables
  char tcfile[256] = "terachem.inp";

  // Set global treatment
  int globaltreatment = 0;

  // Information about initial QM and MM region
  int numqmatoms = 3;
  char (*qmattypes)[5] = new char[numqmatoms][5] {"O","H","H"};

  // Attempts to connect to the TeraChem server
  printf(" Attempting to connect to TeraChem server using host %s and port %d.\n", host, port);
  int status = -1;
  tc_connect_(host, &port, &status);
  if (status == 0) {
    printf(" Successfully connected to TeraChem server.\n");
  } else if (status == 1) {
    printf(" ERROR: Connection to TeraChem server failed!\n");
    return 1;
  } else if (status == 2) {
    printf(" ERROR: Connection to TeraChem server succeed, but the \n"
           "        server is not available!\n");
    return 1;
  } else {
    printf(" ERROR: Status on tc_connect function is not recognized!\n");
    return 1;
  }

  // Setup TeraChem
  status = -1;
  tc_setup_(tcfile,qmattypes,&numqmatoms,&status);
  if (status == 0) {
    printf(" TeraChem setup completed with success.\n");
  } else if (status == 1) {
    printf(" ERROR: No options read from TeraChem input file!\n");
    return 1;
  } else if (status == 2) {
    printf(" ERROR: Failed to setup TeraChem.\n");
    return 1;
  } else {
    printf(" ERROR: Status on tc_setup function is not recognized!\n");
    return 1;
  }

  // Set QM region coordinates
  double* qmcoords = new double[3*numqmatoms]
              { -4.4798000,  -2.8400000,   4.2456000,
                -4.8525000,  -3.7649000,   4.3951000,
                -3.6050000,  -2.7568000,   4.9264000};
  double* qmgrad = new double[3*numqmatoms];
  for (int i=0; i < 3*numqmatoms; i++) {
    qmcoords[i] /= BohrToAng;
  }

  // No MM region at the moment
  int nummmatoms = 0;

  // Compute energy and gradient
  printf("\n");
  status = -1;
  tc_compute_energy_gradient_(qmattypes,qmcoords,&numqmatoms,&totenergy,qmgrad,nullptr,nullptr,&nummmatoms,nullptr,&globaltreatment,&status);
  if (status == 0) {
    printf(" Computed energy and gradient with success.\n");
  } else if (status == 1) {
    printf(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!\n");
    return 1;
  } else if (status == 2) {
    printf(" ERROR: Problem to compute energy and gradient!\n");
    return 1;
  } else {
    printf(" ERROR: Status on tc_compute_energy_gradient function is not recognized!\n");
    return 1;
  }

  // Print results
  printf(" Results from 1st calculation (only one water molecule in the QM region)\n");
  printf("E = %16.10f Hartrees\n", totenergy);
  for (int i=0; i < numqmatoms; i++) {
    printf("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]);
  }

  // We now add an MM region
  nummmatoms = 15;
  double* mmcoords = new double[3*nummmatoms] 
              { -2.6793000,  -2.1596000,   5.9264000,
                -1.7944000,  -2.5941000,   6.0208000,
                -2.4543000,  -1.2247000,   5.9247000,
                -6.0739000,  -0.8812700,   5.2104000,
                -5.3910000,  -1.5014000,   4.7942000,
                -5.4189000,  -0.3240900,   5.9375000,
                -4.0898000,  -5.6279000,   2.9956000,
                -4.6091000,  -5.6876000,   2.2341000,
                -4.1166000,  -6.5262000,   3.2888000,
                -2.3448000,  -2.6425000,   1.8190000,
                -2.7846000,  -3.1506000,   2.6164000,
                -1.5986000,  -3.2938000,   1.7252000,
                -4.6456000,  -4.4223000,   7.4705000,
                -3.6650000,  -4.5356000,   7.1235000,
                -4.9759000,  -3.5580000,   7.3041000 };
  for (int i=0; i < 3*nummmatoms; i++) {
    mmcoords[i] /= BohrToAng;
  }
  double* mmcharges = new double[nummmatoms]
               { -0.834,
                  0.417,
                  0.417,
                 -0.834,
                  0.417,
                  0.417,
                 -0.834,
                  0.417,
                  0.417,
                 -0.834,
                  0.417,
                  0.417,
                 -0.834,
                  0.417,
                  0.417 };
  double* mmgrad = new double[3*nummmatoms];

  // Compute energy and gradient
  printf("\n");
  status = -1;
  tc_compute_energy_gradient_(qmattypes,qmcoords,&numqmatoms,&totenergy,qmgrad,mmcoords,mmcharges,&nummmatoms,mmgrad,&globaltreatment,&status);
  if (status == 0) {
    printf(" Computed energy and gradient with success.\n");
  } else if (status == 1) {
    printf(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!\n");
    return 1;
  } else if (status == 2) {
    printf(" ERROR: Problem to compute energy and gradient!\n");
    return 1;
  } else {
    printf(" ERROR: Status on tc_compute_energy_gradient function is not recognized!\n");
    return 1;
  }

  // Print results
  printf(" Results from 2nd calculation (one water molecule in the QM region and five in the MM region)\n");
  printf("E = %16.10f Hartrees\n", totenergy);
  for (int i=0; i < numqmatoms; i++) {
    printf("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]);
  }
  for (int i=0; i < nummmatoms; i++) {
    printf("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]);
  }

  // Compute energy and gradient
  printf("\n");
  status = -1;
  tc_compute_energy_gradient_(qmattypes,qmcoords,&numqmatoms,&totenergy,qmgrad,mmcoords,mmcharges,&nummmatoms,mmgrad,&globaltreatment,&status);
  if (status == 0) {
    printf(" Computed energy and gradient with success.\n");
  } else if (status == 1) {
    printf(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!\n");
    return 1;
  } else if (status == 2) {
    printf(" ERROR: Problem to compute energy and gradient!\n");
    return 1;
  } else {
    printf(" ERROR: Status on tc_compute_energy_gradient function is not recognized!\n");
    return 1;
  }

  // Print results
  printf(" Results from 3rd calculation (just repeating the 2nd calculation)\n");
  printf("E = %16.10f Hartrees\n", totenergy);
  for (int i=0; i < numqmatoms; i++) {
    printf("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]);
  }
  for (int i=0; i < nummmatoms; i++) {
    printf("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]);
  }

  // Change coordinates of the QM region
  delete[] qmcoords;
  qmcoords = new double[3*numqmatoms]
             { -4.4748000,  -2.8700000,   4.5456000,
               -4.8525000,  -3.7649000,   4.3951000,
               -3.6050000,  -2.7568000,   4.9264000 };
  for (int i=0; i < 3*numqmatoms; i++) {
    qmcoords[i] /= BohrToAng;
  }

  // Compute energy and gradient
  printf("\n");
  status = -1;
  tc_compute_energy_gradient_(qmattypes,qmcoords,&numqmatoms,&totenergy,qmgrad,mmcoords,mmcharges,&nummmatoms,mmgrad,&globaltreatment,&status);
  if (status == 0) {
    printf(" Computed energy and gradient with success.\n");
  } else if (status == 1) {
    printf(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!\n");
    return 1;
  } else if (status == 2) {
    printf(" ERROR: Problem to compute energy and gradient!\n");
    return 1;
  } else {
    printf(" ERROR: Status on tc_compute_energy_gradient function is not recognized!\n");
    return 1;
  }

  // Print results
  printf(" Results from 4th calculation (changed coordinates of the QM region)\n");
  printf("E = %16.10f Hartrees\n", totenergy);
  for (int i=0; i < numqmatoms; i++) {
    printf("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]);
  }
  for (int i=0; i < nummmatoms; i++) {
    printf("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]);
  }

  // Move one water molecule from the MM region to the QM region
  numqmatoms = 6;
  delete[] qmattypes;
  qmattypes = new char[numqmatoms][5] {"O","H","H","O","H","H"};
  delete[] qmcoords;
  qmcoords = new double[3*numqmatoms]
              { -4.4798000,  -2.8400000,   4.2456000,
                -4.8525000,  -3.7649000,   4.3951000,
                -3.6050000,  -2.7568000,   4.9264000,
                -2.6793000,  -2.1596000,   5.9264000,
                -1.7944000,  -2.5941000,   6.0208000,
                -2.4543000,  -1.2247000,   5.9247000 };
  delete[] qmgrad;
  qmgrad = new double[3*numqmatoms];
  for (int i=0; i < 3*numqmatoms; i++) {
    qmcoords[i] /= BohrToAng;
  }
  nummmatoms = 12;
  delete[] mmcoords;
  mmcoords = new double[3*nummmatoms]
              { -6.0739000,  -0.8812700,   5.2104000,
                -5.3910000,  -1.5014000,   4.7942000,
                -5.4189000,  -0.3240900,   5.9375000,
                -4.0898000,  -5.6279000,   2.9956000,
                -4.6091000,  -5.6876000,   2.2341000,
                -4.1166000,  -6.5262000,   3.2888000,
                -2.3448000,  -2.6425000,   1.8190000,
                -2.7846000,  -3.1506000,   2.6164000,
                -1.5986000,  -3.2938000,   1.7252000,
                -4.6456000,  -4.4223000,   7.4705000,
                -3.6650000,  -4.5356000,   7.1235000,
                -4.9759000,  -3.5580000,   7.3041000 };
  delete[] mmgrad;
  mmgrad = new double[3*nummmatoms];
  for (int i=0; i < 3*nummmatoms; i++) {
    mmcoords[i] /= BohrToAng;
  }
  delete[] mmcharges;
  mmcharges = new double[nummmatoms]
               { -0.834,
                  0.417,
                  0.417,
                 -0.834,
                  0.417,
                  0.417,
                 -0.834,
                  0.417,
                  0.417,
                 -0.834,
                  0.417,
                  0.417 };

  // Compute energy and gradient
  printf("\n");
  status = -1;
  tc_compute_energy_gradient_(qmattypes,qmcoords,&numqmatoms,&totenergy,qmgrad,mmcoords,mmcharges,&nummmatoms,mmgrad,&globaltreatment,&status);
  if (status == 0) {
    printf(" Computed energy and gradient with success.\n");
  } else if (status == 1) {
    printf(" ERROR: Mismatch in the variables passed to the function to compute energy and gradient!\n");
    return 1;
  } else if (status == 2) {
    printf(" ERROR: Problem to compute energy and gradient!\n");
    return 1;
  } else {
    printf(" ERROR: Status on tc_compute_energy_gradient function is not recognized!\n");
    return 1;
  }

  // Print results
  printf(" Results from 5th calculation (moved one molecule from the MM to the QM region)\n");
  printf("E = %16.10f Hartrees\n", totenergy);
  for (int i=0; i < numqmatoms; i++) {
    printf("QM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,qmgrad[3*i], qmgrad[3*i+1], qmgrad[3*i+2]);
  }
  for (int i=0; i < nummmatoms; i++) {
    printf("MM Grad(%3d,:) = %16.10f%16.10f%16.10f Hartree/Bohr\n",i+1,mmgrad[3*i], mmgrad[3*i+1], mmgrad[3*i+2]);
  }

  // Finalizes variables on the TeraChem side
  tc_finalize_();

  // Delete variables that have been allocated on heap
  delete[] qmattypes;
  delete[] qmcoords;
  delete[] qmgrad;
  delete[] mmcoords;
  delete[] mmcharges;
  delete[] mmgrad;

  return 0;
}
