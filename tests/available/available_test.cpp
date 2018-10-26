/** \file available_test.cpp
 *  \brief C++ version of available_test.py
 *  \author Stefan Seritan <sseritan@stanford.edu>
 *  \date Aug 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "tcpb.h"

const int expected_cycles = 6;

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: %s host port\n", argv[0]);
  }

  int port = atoi(argv[2]);
  TCPBClient* TC = new TCPBClient(argv[1], port);

  TC->Connect();

  int count = 0;
  while (!TC->IsAvailable()) {
    count++;
  }

  if (count != expected_cycles) {
    printf("Expected %d cycles, but only got %d\n", expected_cycles, count);
    return 1;
  }
 
  // Memory Management
  delete TC; //Handles disconnect

  return 0;
}
