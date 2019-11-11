/** \file input.cpp
 *  \brief Implementation of TCPB::Input class
 */


#include <fstream>
using std::ofstream;
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
  strmap options;

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
  ofstream f(tcfile);
  int natoms = pb_.mol().atoms_size();
  vector<string> atoms;
  vector<double> geom, geom2;

  // Pull required keywords from internal protobuf
  f << "run " << Utils::ToLower(JobInput_RunType_Name(pb_.run())) << "\n";
  f << "basis " << Utils::ToLower(pb_.basis()) << "\n";
  f << "charge " << pb_.mol().charge() << "\n";
  f << "spinmult " << pb_.mol().multiplicity() << "\n";

  string method = "";
  if (!pb_.mol().restricted()) {
    method = "u";
  } else if (pb_.mol().restricted() && !pb_.mol().closed()) {
    method = "ro";
  }
  method += Utils::ToLower(JobInput_MethodType_Name(pb_.method()));
  f << "method " << method << "\n";

  //Handle coordinate dumps
  for (int i = 0; i < natoms; ++i) {
    atoms.push_back(pb_.mol().atoms(i));
    geom.push_back(pb_.mol().xyz(3*i  ));
    geom.push_back(pb_.mol().xyz(3*i+1));
    geom.push_back(pb_.mol().xyz(3*i+2));
  }
  Utils::WriteXYZFile(xyzfile, atoms, geom);
  f << "coordinates " << xyzfile << "\n";

  if (pb_.xyz2_size()) {
    for (int i = 0; i < natoms; ++i) {
      geom2.push_back(pb_.xyz2(3*i  ));
      geom2.push_back(pb_.xyz2(3*i+1));
      geom2.push_back(pb_.xyz2(3*i+2));
    }
    Utils::WriteXYZFile(xyzfile + ".old", atoms, geom2);
    f << "old_coors " << xyzfile << ".old" << "\n";
  }

  // Do all user options
  for (int i = 0; i < pb_.user_options_size()/2; ++i) {
    f << pb_.user_options(2*i) << " " << pb_.user_options(2*i+1) << "\n";
  }
}

bool Input::IsApproxEqual(const Input &other) const
{
  using namespace google::protobuf::util;
  return MessageDifferencer::ApproximatelyEquals(pb_, other.pb_);
}

// Convenience function to enable both constructors
JobInput Input::InitInputPB(const vector<string> &atoms,
  const strmap &options,
  const double *geom,
  const double *geom2)
{
  JobInput pb = JobInput();
  Mol *mol = pb.mutable_mol();
  int numAtoms = atoms.size();
  strmap parsed_options(options);

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
      string errMsg = "Runtype '" + parsed_options.at("run") + "' is not valid.\n";
      errMsg += "Valid runtypes (case-insensitive):\n" + JobInput::RunType_descriptor()->DebugString();
      throw std::runtime_error(errMsg);
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
      string errMsg = "Method '" + parsed_options.at("method") + "' is not valid.\n";
      errMsg += "Valid methods (case-insensitive):\n" + JobInput::MethodType_descriptor()->DebugString();
      throw std::runtime_error(errMsg);
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
  for (strmap::iterator it = parsed_options.begin();
    it != parsed_options.end(); ++it) {
    pb.add_user_options(it->first);
    pb.add_user_options(it->second);
  }

  return pb;
}

} // end namespace TCPB
