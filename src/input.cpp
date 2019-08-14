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

#include "constants.h"
using constants::ANGSTROM_TO_AU;
#include "input.h"
#include "terachem_server.pb.h"
using terachem_server::JobInput; using terachem_server::Mol;
#include "utils.h"
using TCPBUtils::ReadXYZFile; using TCPBUtils::ReadTCFile;
using TCPBUtils::ParseMethod;

TCPBInput::TCPBInput(string run,
                     const vector<string>& atoms,
                     const map<string, string>& options,
                     const double* const geom,
                     const double* const geom2) {
  pb_ = InitInputPB(run, atoms, options, geom, geom2);
}

TCPBInput::TCPBInput(string tcfile,
                     string xyzfile,
                     string xyzfile2) {
  float scale = ANGSTROM_TO_AU;
  vector<string> atoms;
  vector<double> geom;
  vector<double> geom2;
  map<string, string> options;

  options = ReadTCFile(tcfile);

  // Preprocess some options
  string run = options["run"];
  options.erase("run");
  if (options.count("coordinates")) {
    xyzfile = options["coordinates"];
    options.erase("coordinates");
  }
  if (options.count("units")) {
    if (ToUpper(options["units"]).compare("BOHR") == 0) scale = 1.0;
    options.erase("units");
  }
  if (options.count("old_coors")) {
    xyzfile2 = options["old_coors"];
    options.erase("old_coors");
  }

  ReadXYZFile(xyzfile, atoms, geom, scale);

  if (xyzfile2.empty()) {
    pb_ = InitInputPB(run, atoms, options, geom.data());
  } else {
    ReadXYZFile(xyzfile2, atoms, geom2, scale);
    pb_ = InitInputPB(run, atoms, options, geom.data(), geom2.data());
  }
}

// Convenience function to enable both constructors
JobInput TCPBInput::InitInputPB(string run,
                                const vector<string>& atoms,
                                const map<string, string>& options,
                                const double* const geom,
                                const double* const geom2) {
  JobInput pb = JobInput();
  Mol* mol = pb.mutable_mol();
  int numAtoms = atoms.size();
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
  pb.set_run(runtype);

  // Geometry and atoms
  mol->mutable_xyz()->Resize(3*numAtoms, 0.0);
  memcpy(mol->mutable_xyz()->mutable_data(), geom, 3*numAtoms*sizeof(double));

  for (int i = 0; i < numAtoms; i++) {
    mol->add_atoms(atoms[i]);
  }

  // Units (legacy, most interfaces are now designed for coordinates to be in bohr)
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

    //Handle method prefix
    string methodStr = parsed_options.at("method");
    bool closed = true;
    bool restricted = true;
    ParseMethod(methodStr, closed, restricted);

    mol->set_closed(closed);
    mol->set_restricted(restricted);

    terachem_server::JobInput_MethodType method;
    bool valid_method = JobInput::MethodType_Parse(ToUpper(methodStr), &method);
    if (!valid) {
      printf("Method %s passed in options map is not valid.\n", parsed_options.at("method").c_str());
      printf("Valid methods (case-insensitive):\n%s\n",
             JobInput::MethodType_descriptor()->DebugString().c_str());
      exit(1);
    }
    pb.set_method(method);
    parsed_options.erase("method");

    string basis = parsed_options.at("basis");
    pb.set_basis(basis);
    parsed_options.erase("basis");
  } catch (const std::out_of_range& oor) {
    // TODO: Should probably have some exception for this and other errors in this block
    printf("Missing a required keyword in options map:\n");
    printf("charge, spinmult, closed_shell, restricted, method, basis\n");
    exit(1);
  }

  // Optional protocol-specific keywords
  if (parsed_options.count("bond_order")) {
    if (ToUpper(parsed_options["bond_order"]).compare("TRUE") == 0) {
      pb.set_return_bond_order(true);
    }
    parsed_options.erase("bond_order");
  }
  if (geom2 != NULL) {
    pb.mutable_xyz2()->Resize(3*numAtoms, 0.0);
    memcpy(pb.mutable_xyz2()->mutable_data(), geom2, 3*numAtoms*sizeof(double));
  }

  // All other options are passed straight through to TeraChem
  for (map<string,string>::iterator it=parsed_options.begin(); it != parsed_options.end(); ++it) {
    pb.add_user_options(it->first);
    pb.add_user_options(it->second);
  }

  return pb;
}

// Based on https://stackoverflow.com/a/313990/3052876
string TCPBInput::ToUpper(const string& str) {
  string upper(str);

  // Basically lambda to call toupper on each individual character
  transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c){ return toupper(c); });

  return upper;
}
