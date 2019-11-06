/** \file input.cpp
 *  \brief Implementation of TCPB::Input class
 */


#include <map>
using std::map;
#include <string>
using std::string;
using std::stoi;
#include <vector>
using std::vector;

#include <google/protobuf/util/message_differencer.h>

#include "input.h"

#include "constants.h"
#include "terachem_server.pb.h"
using terachem_server::JobInput;
using terachem_server::Mol;
#include "utils.h"

namespace TCPB {

Input::Input(const vector<string> &atoms,
  const map<string, string> &options,
  const double *geom,
  const double *geom2)
{
  pb_ = InitInputPB(atoms, options, geom, geom2);
}

Input::Input(string tcfile,
  string xyzfile,
  string xyzfile2)
{
  double scale = constants::ANGSTROM_TO_AU;
  vector<string> atoms;
  vector<double> geom;
  vector<double> geom2;
  map<string, string> options;

  options = Utils::ReadTCFile(tcfile);

  // Preprocess some options
  if (options.count("units")) {
    if (Utils::ToUpper(options["units"]).compare("BOHR") == 0) {
      scale = 1.0;
    }
    options.erase("units");
  }
  if (options.count("coordinates")) {
    if (xyzfile.empty()) {
      // Make sure to get relative path
      size_t dirPos = tcfile.find_last_of('/');
      string dir = tcfile.substr(0, dirPos);
      xyzfile = dir + "/" + options["coordinates"];
    }
    options.erase("coordinates");
  }
  if (options.count("old_coors")) {
    if (xyzfile2.empty()) {
      // Make sure to get relative path
      size_t dirPos = tcfile.find_last_of('/');
      string dir = tcfile.substr(0, dirPos);
      xyzfile2 = dir + "/" + options["old_coors"];
    }
    options.erase("old_coors");
  }

  Utils::ReadXYZFile(xyzfile, atoms, geom, scale);

  if (xyzfile2.empty()) {
    pb_ = InitInputPB(atoms, options, geom.data());
  } else {
    Utils::ReadXYZFile(xyzfile2, atoms, geom2, scale);
    pb_ = InitInputPB(atoms, options, geom.data(), geom2.data());
  }
}

void Input::WriteTCFile(string tcfile, string xyzfile) const
{

}

bool Input::IsApproxEqual(const Input &other) const
{
  using namespace google::protobuf::util;
  return MessageDifferencer::ApproximatelyEquals(pb_, other.pb_);
}

// Convenience function to enable both constructors
JobInput Input::InitInputPB(const vector<string> &atoms,
  const map<string, string> &options,
  const double *geom,
  const double *geom2)
{
  JobInput pb = JobInput();
  Mol *mol = pb.mutable_mol();
  int numAtoms = atoms.size();
  map<string, string> parsed_options(options);

  // Geometry and atoms
  mol->mutable_xyz()->Resize(3 * numAtoms, 0.0);
  memcpy(mol->mutable_xyz()->mutable_data(), geom, 3 * numAtoms * sizeof(double));

  for (int i = 0; i < numAtoms; i++) {
    mol->add_atoms(atoms[i]);
  }

  // Units (legacy, internally we only use a.u. now)
  string units;
  try {
    units = parsed_options.at("units");
  } catch (const std::out_of_range &err) {
    units = "BOHR";
  }
  if (!Utils::ToUpper(units).compare("ANGSTROM")) {
    for (int i = 0; i < 3 * numAtoms; ++i) {
      double *g = mol->mutable_xyz()->mutable_data();
      g[i] *= constants::ANGSTROM_TO_AU;
    }
  }
  mol->set_units(Mol::BOHR);

  // Handle protocol-specific required keywords
  try {
    // Runtype
    string run = parsed_options["run"];
    terachem_server::JobInput_RunType runtype;
    bool valid = JobInput::RunType_Parse(Utils::ToUpper(run), &runtype);
    if (!valid) {
      printf("Runtype %s passed is not valid.\n", run.c_str());
      printf("Valid runtypes (case-insensitive):\n%s\n",
        JobInput::RunType_descriptor()->DebugString().c_str());
      exit(1);
    }
    pb.set_run(runtype);
    parsed_options.erase("run");

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
    Utils::ParseMethod(methodStr, closed, restricted);
    mol->set_closed(closed);
    mol->set_restricted(restricted);

    terachem_server::JobInput_MethodType method;
    bool valid_method = JobInput::MethodType_Parse(Utils::ToUpper(methodStr),
        &method);
    if (!valid) {
      printf("Method %s passed in options map is not valid.\n",
        parsed_options.at("method").c_str());
      printf("Valid methods (case-insensitive):\n%s\n",
        JobInput::MethodType_descriptor()->DebugString().c_str());
      exit(1);
    }
    pb.set_method(method);
    parsed_options.erase("method");

    string basis = parsed_options.at("basis");
    pb.set_basis(basis);
    parsed_options.erase("basis");
  } catch (const std::out_of_range &oor) {
    string errMsg = "Missing a required keyword in options map:\n";
    errMsg += "run, charge, spinmult, closed_shell, restricted, method, basis\n";
    throw std::runtime_error(errMsg.c_str());
  }

  // Optional protocol-specific keywords
  if (parsed_options.count("bond_order")) {
    if (Utils::ToUpper(parsed_options["bond_order"]).compare("TRUE") == 0) {
      pb.set_return_bond_order(true);
    }
    parsed_options.erase("bond_order");
  }
  if (geom2 != NULL) {
    pb.mutable_xyz2()->Resize(3 * numAtoms, 0.0);
    memcpy(pb.mutable_xyz2()->mutable_data(), geom2, 3 * numAtoms * sizeof(double));

    if (!Utils::ToUpper(units).compare("ANGSTROM")) {
      for (int i = 0; i < 3 * numAtoms; ++i) {
        double *g2 = pb.mutable_xyz2()->mutable_data();
        g2[i] *= constants::ANGSTROM_TO_AU;
      }
    }
  }

  // All other options are passed straight through to TeraChem
  for (map<string, string>::iterator it = parsed_options.begin();
    it != parsed_options.end(); ++it) {
    pb.add_user_options(it->first);
    pb.add_user_options(it->second);
  }

  return pb;
}

} // end namespace TCPB
