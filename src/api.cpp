/** \file api.cpp
 *  \brief Houses the functionalities to make TCPB-cpp an API that can be accessed from Fortran
 */

#include <map>
using std::map;
#include <stdio.h>
#include <stdlib.h>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <unistd.h>

#include "tcpb/client.h"
#include "tcpb/input.h"
#include "tcpb/output.h"
#include "tcpb/utils.h"

#define BohrToAng 0.52917724924

extern "C" {

  TCPB::Client* TC = nullptr;

  /**
   * \brief Connects to TeraChem server
   *
   * @param[in] host Address of the host
   * @param[in] port Port number
   * @param[out] status Status of execution: 0, all is good; 1, could not connect to server; 2, connected to server but it is not available
   **/
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

  /**
   * \brief Deletes from memory variables that are open
   **/
  void tc_finalize_() {
    if (TC != nullptr)
      delete TC;
  }

} // extern "C"
