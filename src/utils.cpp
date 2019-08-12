/** \file utils.cpp
 *  \brief Utility functions for TCPB C++ client
 */

#include <fstream>
using std::ifstream;
#include <map>
using std::map;
#include <sstream>
using std::istringstream;
#include <string>
using std::string; using std::getline; using std::stoi;
#include <vector>
using std::vector;

#include "utils.h"

namespace TCPBUtils {

// Based on https://stackoverflow.com/a/15206674/3052876
map<string, string> ReadTCFile(string tcfile) {
  ifstream f(tcfile.c_str());
  string line;
  map<string, string> options;

  while (getline(f, line)) {
    // Comment lines
    if (line.rfind("#", 0) == 0) continue;
    if (line.rfind("!", 0) == 0) continue;

    istringstream ss(line);
    string key, value;

    ss >> key >> value;
    options[key] = value;
  }

  return options;
}

void ReadXYZFile(string xyzfile,
                 vector<string>& atoms,
                 vector<double>& geom) {
  ifstream f(xyzfile.c_str());
  string line;

  getline(f, line);
  int natoms = stoi(line);

  // Comment line
  getline(f, line);

  for (int i = 0; i < natoms; i++) {
    getline(f, line);

    istringstream ss(line);
    string sym;
    double x, y, z;

    ss >> sym >> x >> y >> z;
    atoms.push_back(sym);
    geom.push_back(x);
    geom.push_back(y);
    geom.push_back(z);
  }
}

} // end namespace TCPBUtils
