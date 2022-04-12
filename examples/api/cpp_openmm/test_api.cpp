/** \file test_api.cpp
 *  \brief Example of API use
 */

#include <stdio.h>

#include <tcpb/api.h>

// Conversion parameter: Bohr to Angstrom
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

  // Information about initial QM region
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

  // Set QM region coordinates, defined in Angstroms and then converted to Bohrs
  double* qmcoords = new double[3*numqmatoms]
              { -4.4798000,  -2.8400000,   4.2456000,
                -4.8525000,  -3.7649000,   4.3951000,
                -3.6050000,  -2.7568000,   4.9264000};
  double* qmgrad = new double[3*numqmatoms];
  for (int i=0; i < 3*numqmatoms; i++) {
    qmcoords[i] /= BohrToAng;
  }

  // MM region, defined in Angstroms and then converted to Bohrs. Charges in atomic units
  int nummmatoms = 3;
  double* mmcoords = new double[3*nummmatoms] 
              { -2.6793000,  -2.1596000,   5.9264000,
                -1.7944000,  -2.5941000,   6.0208000,
                -2.4543000,  -1.2247000,   5.9247000};
  for (int i=0; i < 3*nummmatoms; i++) {
    mmcoords[i] /= BohrToAng;
  }
  double* mmcharges;
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
  printf(" Results from 1st calculation (one water molecule in the QM region and one in the MM region)\n");
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
  printf(" Results from 2nd calculation (one water molecule in the QM region and one in the MM region)\n");
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
  delete[] mmgrad;

  return 0;
}
