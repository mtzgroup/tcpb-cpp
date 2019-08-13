/** \file input.cpp
 *  \brief Implementation of TCPBInput class
 */

#include <algorithm>
using std::transform;
#include <cctype>
using std::toupper;
#include <map>
using std::map;
#include <string>
using std::string; using std::stoi;
#include <vector>
using std::vector;

#include "input.h"
#include "terachem_server.pb.h"
using terachem_server::JobInput; using terachem_server::Mol;
#include "utils.h"
using TCPBUtils::ReadXYZFile; using TCPBUtils::ReadTCFile;


TCPBInput::TCPBInput(string run,
                     const vector<string>& atoms,
                     const map<string, string>& options,
                     const double* const geom,
                     const double* const geom2) {
  pb_ = JobInput();

  int numAtoms = atoms.size();
  Mol* mol = pb_.mutable_mol();

  map<string, string> parsed_options(options);

  // Runtype
  terachem_server::JobInput_RunType runtype;
  bool valid = JobInput::RunType_Parse(ToUpper(run), &runtype);
  if (!valid) {
    printf("Runtype %s passed in SendJobAsync() is not valid.\n", run.c_str());
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
  if (parsed_options.count("units")) {
    terachem_server::Mol_UnitType units;
    bool valid = Mol::UnitType_Parse(ToUpper(parsed_options["units"]), &units);
    if (!valid) {
      printf("Units %s passed in options map is not valid.\n", parsed_options["units"].c_str());
      printf("Valid units (case-insensitive):\n%s\n",
             Mol::UnitType_descriptor()->DebugString().c_str());
      exit(1);
    }
    mol->set_units(units);

  } else {
    mol->set_units(Mol::BOHR);
  }

  // Handle protocol-specific required keywords
  try {
    int charge = stoi(parsed_options.at("charge"));
    mol->set_charge(charge);
    parsed_options.erase("charge");

    int spinmult = stoi(parsed_options.at("spinmult"));
    mol->set_multiplicity(spinmult);
    parsed_options.erase("spinmult");

    bool closed_shell = (ToUpper(parsed_options.at("closed_shell")).compare("TRUE") == 0);
    mol->set_closed(closed_shell);
    parsed_options.erase("closed_shell");

    bool restricted = (ToUpper(parsed_options.at("restricted")).compare("TRUE") == 0);
    mol->set_restricted(restricted);
    parsed_options.erase("restricted");

    terachem_server::JobInput_MethodType method;
    bool valid_method = JobInput::MethodType_Parse(ToUpper(parsed_options.at("method")), &method);
    if (!valid) {
      printf("Method %s passed in options map is not valid.\n", parsed_options.at("method").c_str());
      printf("Valid methods (case-insensitive):\n%s\n",
             JobInput::MethodType_descriptor()->DebugString().c_str());
      exit(1);
    }
    pb_.set_method(method);
    parsed_options.erase("method");

    string basis = parsed_options.at("basis");
    pb_.set_basis(basis);
    parsed_options.erase("basis");
  } catch (const std::out_of_range& oor) {
    printf("Missing a required keyword in options map:\n");
    printf("charge, spinmult, closed_shell, restricted, method, basis\n");
    printf("Out-of-range error: %s\n", oor.what());
    exit(1);
  }

  // Optional protocol-specific keywords
  if (parsed_options.count("bond_order")) {
    if (ToUpper(parsed_options["bond_order"]).compare("TRUE") == 0) {
      pb_.set_return_bond_order(true);
    }
    parsed_options.erase("bond_order");
  }
  if (geom2 != NULL) {
    pb_.mutable_xyz2()->Resize(3*numAtoms, 0.0);
    memcpy(pb_.mutable_xyz2()->mutable_data(), geom2, 3*numAtoms*sizeof(double));
  }

  // All other options are passed straight through to TeraChem
  for (map<string,string>::iterator it=parsed_options.begin(); it != parsed_options.end(); ++it) {
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

  options = ReadTCFile(tcfile);

  if (options.count("coordinates")) {
    xyzfile = options["coordinates"];
  }

  // TODO: More error checking

  // TODO: Check units for whether to scale or not
  ReadXYZFile(xyzfile, atoms, geom);

  string run = options["run"];
  options.erase("run");

  // TODO: Pull from tcfile like coordinates
  if (!xyzfile2.empty()) {
    TCPBInput(run, atoms, options, geom.data());
  } else {
    ReadXYZFile(xyzfile2, atoms, geom2);

    TCPBInput(run, atoms, options, geom.data(), geom2.data());
  }
}

// Based on https://stackoverflow.com/a/313990/3052876
string TCPBInput::ToUpper(const string& str) {
  string upper(str);

  // Basically lambda to call toupper on each individual character
  transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c){ return toupper(c); });

  return upper;
}
