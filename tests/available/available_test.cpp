/** \file available_test.cpp
 *  \brief Test IsAvailable() against mock server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "tcpb/client.h"

const int expected_cycles = 6;

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: %s host port\n", argv[0]);
  }

  int port = atoi(argv[2]);
  TCPBClient TC(argv[1], port);

  int count = 0;
  while (!TC.IsAvailable()) {
    count++;
  }

  if (count != expected_cycles) {
    printf("Expected %d cycles, but only got %d\n", expected_cycles, count);
    return 1;
  }

  return 0;
}
