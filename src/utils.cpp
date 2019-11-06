/** \file utils.cpp
 *  \brief Utility functions for TCPB C++ client
 */

#include <stdio.h> // For printf() debugging
#include <stdlib.h> // For atoi()
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
#include "constants.h"

namespace TCPB {

namespace Utils {

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
    string key, value, temp;
    size_t pos;

    ss >> key;
    ss >> value;
    while (ss >> temp) {
      if (temp[0] == '#' || temp[0] == '!') break;
      value += " " + temp;
    }

    options[key] = value;
  }

  return options;
}

// Code to check string start from https://stackoverflow.com/a/40441240/3052876
void ParseMethod(string& method,
                 bool& closed,
                 bool& restricted) {
  //printf("Parsing method %s\n", method.c_str());
  string r("r");
  string ro("ro");
  string u("u");

  // Need to be careful with revpbe/revpbe0
  if (method.rfind("rev", 0) == 0) {
    r += "rev";
    ro += "rev";
    u += "rev";
  }

  if (method.rfind(ro, 0) == 0) {
    closed = true;
    restricted = false;
    method.erase(method.begin(), method.begin()+2);
  } else if (method.rfind(u, 0) == 0) {
    closed = false;
    restricted = false;
    method.erase(method.begin());
  } else {
    closed = true;
    restricted = true;
    
    if (method.rfind(r, 0) == 0) method.erase(method.begin());
  }

  //printf("Parsed to %s (closed: %d, restricted: %d)\n", method.c_str(), closed, restricted);
}

void ReadXYZFile(string xyzfile,
                 vector<string>& atoms,
                 vector<double>& geom,
                 double scale) {
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
    geom.push_back(x * scale);
    geom.push_back(y * scale);
    geom.push_back(z * scale);
  }
}

} // end namespace Utils

} // end namespace TCPB
