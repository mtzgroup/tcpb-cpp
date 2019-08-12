/** \file input.cpp
 *  \brief Implementation of TCPBInput class
 */

#include <map>
using std::map;
#include <string>
using std::string, stoi;
#include <vector>
using std::vector;

#include "input.h"
#include "terachem_server.pb.h"
using terachem_server::JobInput, terachem_server::Mol;
#include "utils.h"
using TCPBUtils::ReadXYZFile, TCPBUtils::ReadTCFile;


TCPBInput::TCPBInput(string run,
                     const vector<string>& atoms,
                     const map<string, string>& options,
                     const double* const geom,
                     const double* const geom2) {
  pb_ = JobInput();

  int numAtoms = atoms.size();
  Mol* mol = pb_.mutable_mol();

  // Runtype
  terachem_server::JobInput_RunType runtype;
  bool valid = JobInput::RunType_Parse(run.upper(), &runtype);
  if (!valid) {
    printf("Runtype %s passed in SendJobAsync() is not valid.\n", run);
    printf("Valid runtypes (case-insensitive):\n%s\n",
           JobInput::RunType_descriptor()->DebugString().c_str());
    exit(1);
  }
  pb_.set_run(runtype);

  // Geometry and atoms
  mol->mutable_xyz()->Resize(3*numAtoms, 0.0);
  memcpy(mol->mutable_xyz()->mutable_data(), geom, 3*numAtoms*sizeof(double));

  for (int i = 0; i < numAtoms; i++) {
    mol->add_atoms(atoms[i]);
  }

  // Units
  if (options.count("units")) {
    terachem_server::Mol_UnitType units;
    bool valid = Mol::UnitType_Parse(options["units"].upper(), &units);
    if (!valid) {
      printf("Units %s passed in options map is not valid.\n", options["units"]);
      printf("Valid units (case-insensitive):\n%s\n",
             Mol::UnitType_descriptor()->DebugString().c_str());
      exit(1);
    }
    mol->set_units(units);
    options.erase("units");

  } else {
    mol->set_units(Mol::BOHR);
  }

  // Handle protocol-specific required keywords
  try {
    int charge = stoi(options.at("charge"));
    mol->set_charge(charge);
    options.erase("charge");

    int spinmult = stoi(options.at("spinmult"));
    mol->set_multiplicity(spinmult);
    options.erase("spinmult");

    bool closed_shell = (options.at("closed_shell").lower().compare("true") == 0);
    mol->set_closed(closed_shell);
    options.erase("closed_shell");

    bool restricted = (options.at("restricted").lower().compare("true") == 0);
    mol->set_restricted(restricted);
    options.erase("restricted");

    terachem_server::JobInput_MethodType method;
    bool valid_method = JobInput::MethodType_Parse(options.at("method").upper(), &method);
    if (!valid) {
      printf("Method %s passed in options map is not valid.\n", options.at("method"));
      printf("Valid methods (case-insensitive):\n%s\n",
             JobInput::MethodType_descriptor()->DebugString().c_str());
      exit(1);
    }
    pb_.set_method(method);
    options.erase("method");

    string basis = options.at("basis");
    pb_.set_basis(basis);
    options.erase("basis");
  } catch (const std::out_of_range& oor) {
    printf("Missing a required keyword in options map:\n");
    printf("charge, spinmult, closed_shell, restricted, method, basis\n");
    printf("Out-of-range error: %s\n", oor.what().c_str());
    exit(1);
  }

  // Optional protocol-specific keywords
  if (options.count("bond_order")) {
    if (options["bond_order"].lower().compare("true") == 0) {
      pb_.set_return_bond_order(true);
    }
    options.erase("bond_order");
  }
  if (geom2 != NULL) {
    mol->mutable_xyz2()->Resize(3*numAtoms, 0.0);
    memcpy(mol->mutable_xyz2()->mutable_data(), geom2, 3*numAtoms*sizeof(double));
  }

  // All other options are passed straight through to TeraChem
  for (map<string,string>::iterator it=options.begin(); it != options.end(); ++it) {
    pb_.add_user_options(it->first);
    pb_.add_user_options(it->second);
  }
}

TCPBInput::TCPBInput(string tcfile,
                     string xyzfile,
                     string xyzfile2) {
  vector<string> atoms;
  vector<double> geom;
  vector<double> geom2;
  map<string, string> options;

  ReadXYZFile(xyzfile, atoms, geom);

  options = ReadTCFile(tcfile);

  string run = options["run"];
  options.erase("run");

  if (!xyzfile2.empty()) {
    TCPBInput(run, atoms, options, geom);
  } else {
    ReadXYZFile(xyzfile2, atoms, geom2);

    TCPBInput(run, atoms, options, geom, geom2);
  }
}
