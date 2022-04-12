/** \file api.cpp
 *  \brief Houses the functionalities to make TCPB-cpp an API that can be accessed from Fortran
 */

#include <map>
using std::map;
#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include<fstream>
#include<sstream>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <unistd.h>

#include "api.h"
#include "client.h"
#include "input.h"
#include "output.h"
#include "utils.h"

#define BohrToAng 0.52917724924

extern "C" {

  // Variables to be used/modified by all function calls, and to be active in between function calls
  TCPB::Client* TC = nullptr;
  TCPB::Input*  pb_input = nullptr;
  int old_numqmatoms = -1;
  bool useopenmm = false;

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
    // Check if the prmtop flag is in the input options. If true, set a variable in the enviroment and
    // read the prmtop file content and the qmindices
    string prmtopcontent;
    std::vector< int > qmindices;
    if (options.count("prmtop")) {
      useopenmm = true;
      std::ifstream fl(options["prmtop"]);
      std::ostringstream ss;
      if (fl.is_open()) {
        ss << fl.rdbuf();
        prmtopcontent = ss.str();
      } else {
        // prmtop file does not exist
        (*status) = 1;
        return;
      }
      fl.close();
      ss.str("");
      if (!options.count("qmindices")) {
        (*status) = 1;
        return;
      }
      fl.open(options["qmindices"]);
      if (fl.is_open()) {
        int num;
        int cnt = 0;
        while (fl >> num) {
          qmindices.push_back(num);
          cnt++;
        }
        // Check if number of qmindices match the number of qmatoms
        if (cnt != (*numqmatoms)) {
          (*status) = 1;
          return;
        }
      } else {
        // qmindices file does not exist
        (*status) = 1;
        return;
      }
      fl.close();
    } else {
      useopenmm = false;
    }
    options.erase("prmtop");
    options.erase("qmindices");
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
    // Set prmtop file content and qmindices
    if (useopenmm) {
      pb_input->GetMutablePB().clear_prmtop_content();
      pb_input->GetMutablePB().set_prmtop_content(prmtopcontent);
      pb_input->GetMutablePB().mutable_qm_indices()->Resize(qmindices.size(), 0);
      for (i = 0; i<qmindices.size(); i++) {
        pb_input->GetMutablePB().mutable_qm_indices()->mutable_data()[i] = qmindices[i];
      }
    }
    //printf("Debug protobuf input string:\n%s\n", pb_input->GetDebugString().c_str());
    // If all is done, then done
    (*status) = 0;
  }

  void tc_compute_energy_gradient_(const char qmattypes[][5], const double* qmcoords, const int* numqmatoms,
    double* totenergy, double* qmgrad, const double* mmcoords, const double* mmcharges,
    const int* nummmatoms, double* mmgrad, const int* globaltreatment, int* status) {
    int i;
    bool ConsiderMM = (nummmatoms != nullptr && (*nummmatoms) > 0);
    // Check for mistakes in the varibles passed to the function
    if (qmcoords == nullptr || numqmatoms == nullptr || (*numqmatoms) <= 0 || totenergy == nullptr ||
        qmgrad == nullptr || (ConsiderMM && (mmcoords == nullptr || (!useopenmm && mmcharges == nullptr) ||
        mmgrad == nullptr) ) || (globaltreatment != nullptr && ((*globaltreatment) < 0 || (*globaltreatment) > 2))) {
      (*status) = 1;
      return;
    }
    // Sleep a bit to ensure the server will receive the next ComputeGradient call
    if (old_numqmatoms > 0) {
      usleep(110000);
    }
    // Set initial condition
    if (useopenmm) {
      pb_input->GetMutablePB().set_qmmm_type(terachem_server::JobInput_QmmmType::JobInput_QmmmType_TC_OPENMM);
      mmcharges = nullptr;
    } else {
      pb_input->GetMutablePB().set_qmmm_type(terachem_server::JobInput_QmmmType::JobInput_QmmmType_POINT_CHARGE);
    }
    if (globaltreatment == nullptr || (*globaltreatment) == 0) {
      if (old_numqmatoms < 1 || old_numqmatoms !=  (*numqmatoms)) {
        pb_input->GetMutablePB().set_md_global_type(terachem_server::JobInput_MDGlobalTreatment::JobInput_MDGlobalTreatment_NEW_CONDITION);
        old_numqmatoms = (*numqmatoms);
      } else {
        pb_input->GetMutablePB().set_md_global_type(terachem_server::JobInput_MDGlobalTreatment::JobInput_MDGlobalTreatment_CONTINUE);
      }
    } else if ((*globaltreatment) == 1) {
      pb_input->GetMutablePB().set_md_global_type(terachem_server::JobInput_MDGlobalTreatment::JobInput_MDGlobalTreatment_NEW_CONDITION);
    } else if ((*globaltreatment) == 2) {
      pb_input->GetMutablePB().set_md_global_type(terachem_server::JobInput_MDGlobalTreatment::JobInput_MDGlobalTreatment_NORMAL);
    } else {
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
      //std::cout << "QM atom " << i+1 << ": " << qmcoords[i] << "\n";
      pb_input->GetMutablePB().mutable_mol()->mutable_xyz()->mutable_data()[i] = qmcoords[i];
    }
    // Handle coordinates of the MM region
    if (mmcoords == nullptr || !ConsiderMM) {
      pb_input->GetMutablePB().clear_mmatom_position();
    } else {
      pb_input->GetMutablePB().mutable_mmatom_position()->Resize(3*(*nummmatoms), 0.0);
      for (i = 0; i<3*(*nummmatoms); i++) {
        //std::cout << "MM atom " << i+1 << ": " << mmcoords[i] << "\n";
        pb_input->GetMutablePB().mutable_mmatom_position()->mutable_data()[i] = mmcoords[i];
      }
    }
    // Handle charges of the MM region
    if (mmcharges == nullptr || !ConsiderMM) {
      pb_input->GetMutablePB().clear_mmatom_charge();
    } else {
      pb_input->GetMutablePB().mutable_mmatom_charge()->Resize((*nummmatoms), 0.0);
      for (i = 0; i<(*nummmatoms); i++) {
        pb_input->GetMutablePB().mutable_mmatom_charge()->mutable_data()[i] = mmcharges[i];
      }
    }
    //printf("Debug protobuf input string:\n%s\n", pb_input->GetDebugString().c_str());
    // Attempt to create the PB input variable
    try {
      TCPB::Output pb_output = TC->ComputeGradient((*pb_input), (*totenergy), qmgrad, mmgrad);
      //printf("Debug protobuf output string:\n%s\n", pb_output.GetDebugString().c_str());
    }
    catch (...) {
      (*status) = 2;
      return;
    }
    // If all is done, then done
    (*status) = 0;
  }

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
