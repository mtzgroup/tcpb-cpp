/** \file api.cpp
 *  \brief Houses the functionalities to make TCPB-cpp an API that can be accessed from Fortran
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

#include "client.h"
#include "input.h"
#include "output.h"
#include "utils.h"

#define BohrToAng 0.52917724924

extern "C" {

  // Variables to be used/modified by all function calls, and to be active in between function calls
  TCPB::Client* TC = nullptr;
  TCPB::Input*  pb_input = nullptr;

  /**
   * \brief Connects to TeraChem server
   *
   * @param[in] host Address of the host
   * @param[in] port Port number
   * @param[out] status Status of execution: 0, all is good;
   *                                         1, could not connect to server;
   *                                         2, connected to server but it is not available
   **/
  void tc_connect_(const char host[80], const int* port, int* status) {
    try {
      TC = new TCPB::Client(std::string(host), (*port));
    }
    catch (...) {
      (*status) = 1;
      return;
    }
    bool avail = TC->IsAvailable();
    if (!avail)
      (*status) = 2;
    else
      (*status) = 0;
  }

  /**
   * \brief Setup TeraChem protobuf input variable
   *
   * @param[in] tcfile Path to the TeraChem input file
   * @param[in] qmattypes List of atomic types in the QM region
   * @param[in] numqmatoms Number of atoms in the QM region
   * @param[in] nummmatoms Number of atoms in the MM region
   * @param[out] status Status of execution: 0, all is good
   *                                         1, no options read from tcfile
   *                                         2, failed to setup
   **/
  void tc_setup_(const char tcfile[256], const char qmattypes[][5], const int* numqmatoms, int* status) {
    map<string, string> options = TCPB::Utils::ReadTCFile(tcfile);
    if (options.size() == 0) {
      (*status) = 1;
      return;
    }
    // Adjust some options
    options.erase("coordinates");
    options.erase("pointcharges");
    options.erase("run");
    options.emplace("run","gradient");
    // Since this is just a setup call, set all QM coordinates to zero
    double qmcoords[3*(*numqmatoms)];
    memset(qmcoords, 0, 3*(*numqmatoms)*sizeof(double));
    // Change type of array containing atom types
    vector<string> qmatomtypes;
    int i;
    for (i = 0; i<(*numqmatoms); i++) {
      qmatomtypes.push_back(std::string(qmattypes[i]));
    }
    // Attempt to create the PB input variable
    try {
      pb_input = new TCPB::Input(qmatomtypes, options, qmcoords);
    }
    catch (...) {
      (*status) = 2;
      return;
    }
    // If all is gone, then done
    (*status) = 0;
  }

  /**
   * \brief Compute energy and gradient using TeraChem
   *\
   * @param[in] qmattypes List of atomic types in the QM region
   * @param[in] qmcoords Coordinates of the atoms in the QM region (unit: Bohrs)
   * @param[in] numqmatoms Number of atoms in the QM region
   * @param[out] totenergy Total energy of the QM in the presence of the MM region (unit: Hartrees)
   * @param[out] qmgrad Gradient of the atoms in the QM region (unit: Hartree/Bohr)
   * @param[out] status Status of execution: 0, all is good
   *                                         1, mismatch in the variables passed to the function
   *                                         2, calculation failed
   * @param[in] mmcoords Coordinates of the atoms in the MM region, optional (unit: Bohrs)
   * @param[in] nummmatoms Number of atoms in the MM region, optional
   * @param[out] mmgrad Gradient of the atoms in the MM region, optional (unit: Hartree/Bohr)
   **/
  void tc_compute_energy_gradient_(const char qmattypes[][5], const double* qmcoords, const int* numqmatoms,
    double* totenergy, double* qmgrad, const double* mmcoords, const double* mmcharges,
    const int* nummmatoms, double* mmgrad, int* status) {
    int i;
    bool ConsiderMM = (nummmatoms != nullptr && (*nummmatoms) > 0);
    // Check for mistakes in the varibles passed to the function
    if (qmcoords == nullptr || numqmatoms == nullptr || (*numqmatoms) <= 0 || totenergy == nullptr ||
        qmgrad == nullptr || (ConsiderMM && (mmcoords == nullptr || mmcharges == nullptr ||
        mmgrad == nullptr) )) {
      (*status) = 1;
      return;
    }
    // Handle atom types
    pb_input->GetMutablePB().mutable_mol()->clear_atoms();
    for (i = 0; i<(*numqmatoms); i++) {
      pb_input->GetMutablePB().mutable_mol()->add_atoms(std::string(qmattypes[i]));
    }
    // Handle coordinates of the QM region
    pb_input->GetMutablePB().mutable_mol()->mutable_xyz()->Resize(3*(*numqmatoms), 0.0);
    for (i = 0; i<3*(*numqmatoms); i++) {
      pb_input->GetMutablePB().mutable_mol()->mutable_xyz()->mutable_data()[i] = qmcoords[i];
    }
    // Handle coordinates of the MM region
    if (mmcoords == nullptr || !ConsiderMM) {
      pb_input->GetMutablePB().clear_mmatom_position();
    } else {
      pb_input->GetMutablePB().mutable_mmatom_position()->Resize(3*(*nummmatoms), 0.0);
      for (i = 0; i<3*(*nummmatoms); i++) {
        pb_input->GetMutablePB().mutable_mmatom_position()->mutable_data()[i] = mmcoords[i];
      }
    }
    // Handle charges of the MM region
    if (mmcharges == nullptr || !ConsiderMM) {
      pb_input->GetMutablePB().clear_mmatom_charges();
    } else {
      pb_input->GetMutablePB().mutable_mmatom_charges()->Resize((*nummmatoms), 0.0);
      for (i = 0; i<(*nummmatoms); i++) {
        pb_input->GetMutablePB().mutable_mmatom_charges()->mutable_data()[i] = mmcharges[i];
      }
    }
    // Attempt to create the PB input variable
    try {
      TCPB::Output pb_output = TC->ComputeGradient((*pb_input), (*totenergy), qmgrad, mmgrad);
    }
    catch (...) {
      (*status) = 2;
      return;
    }
    // If all is gone, then done
    (*status) = 0;
  }

  /**
   * \brief Deletes from memory variables that are open
   **/
  void tc_finalize_() {
    if (TC != nullptr) {
      delete TC;
      TC = nullptr;
    }
    if (pb_input != nullptr) {
      delete pb_input;
      pb_input = nullptr;
    }
  }

} // extern "C"
