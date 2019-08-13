/** \file utils.cpp
 *  \brief Utility functions for TCPB C++ client
 */

#include <cstdio> // For printf() debugging
#include <cstdlib> // For atoi()
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
    if (line.empty()) continue;
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
                 vector<double>& geom,
                 double scale) {
  ifstream f(xyzfile.c_str());
  string line;

  getline(f, line);
  printf(line.c_str());
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
    geom.push_back(x * scale);
    geom.push_back(y * scale);
    geom.push_back(z * scale);
  }
}

} // end namespace TCPBUtils
