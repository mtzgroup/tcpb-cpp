/** \file exceptions.cpp
 *  \brief Implementation of exception classes
 */

#include <fstream>
using std::ifstream;
#include <string>
using std::string; using std::getline; using std::to_string;

#include "exceptions.h"

ServerError::ServerError(string msg,
                         string host,
                         int port,
                         string jobDir,
                         int jobId) : runtime_error("TCPB Server Error") {
  msg_ = msg;
  msg_ += "\n\n";

  msg_ += "Server Hostname: " + host + "\n";
  msg_ += "Server Port: " + to_string(port) + "\n";

  ifstream logfile;
  logfile.exceptions(ifstream::failbit | ifstream::badbit);
  string lfname = jobDir + "/" + to_string(jobId) + ".log";
  try {
    logfile.open(lfname);

    // Grab last ten lines using circular buffer
    // Based on https://www.daniweb.com/programming/software-development/threads/235695/read-last-ten-lines-from-text
    string line, buffer[10];
    const size_t size = sizeof(buffer)/sizeof(*buffer);
    size_t i = 0;

    while ( getline(logfile, line) ) {
      buffer[i] = line;
      if (++i >= size) i=0;
    }

    msg_ += "Last 10 lines from logfile (" + lfname + ") :\n";
    for (size_t j = 0; j < size; ++j) {
      msg += buffer[i] + "\n";
      if (++i >= size) i=0;
    }
  } catch (const ifstream::failure& e) {
    msg_ += "Could not open logfile (" + lfname + ")\n";
  }
}
