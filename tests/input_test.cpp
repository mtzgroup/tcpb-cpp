/** \file input_test.cpp
 *  \brief Tests of Input class
 */
#include <map>
using std::map;
#include <stdio.h>
#include <stdexcept>
using std::runtime_error;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "tcpb/input.h"
using TCPB::Input;
#include "tcpb/constants.h"

/**********/
// TESTS //
/**********/

bool testTCFileLoadPartial(const Input &ref)
{
  printf("Testing building Input from TC input and coordfile... ");
  fflush(stdout);

  Input input("input/tc.template", "input/water.xyz");
  if (!input.IsApproxEqual(ref)) {
    printf("FAILED. Input(tcfile, xyzfile):\n%s\nReference:\n%s\n",
      input.GetDebugString().c_str(), ref.GetDebugString().c_str());
    return false;
  }

  printf("SUCCESS\n");
  return true;
}

bool testTCFileLoadFull(const Input &ref)
{
  printf("Testing building Input fully from TC input... ");
  fflush(stdout);

  Input input("input/tc.template");
  if (!input.IsApproxEqual(ref)) {
    printf("FAILED. Input(tcfile):\n%s\nReference:\n%s\n",
      input.GetDebugString().c_str(), ref.GetDebugString().c_str());
    return false;
  }

  printf("SUCCESS\n");
  return true;
}

bool testTCFileLoadBroken(const Input &ref)
{
  printf("Testing building Input from broken TC input... ");
  fflush(stdout);

  try {
    Input input("input/tc.broken");
  } catch (const runtime_error& err) {
    printf("SUCCESS\n");
    return true;
  }

  printf("FAILED. Input did not throw proper exception\n");
  return false;
}

bool testTCFileLoadBohr(const Input &ref)
{
  printf("Testing building Input with a.u. units... ");
  fflush(stdout);

  Input input("input/tc.bohr");
  if (!input.IsApproxEqual(ref)) {
    printf("FAILED. Input(tcfile):\n%s\nReference:\n%s\n",
      input.GetDebugString().c_str(), ref.GetDebugString().c_str());
    return false;
  }

  printf("SUCCESS\n");
  return true;
}

bool testTCFileLoadMethod(const Input &ref)
{
  printf("Testing method edge cases in Input... ");
  fflush(stdout);

  Input input("input/tc.method");
  if (!input.IsApproxEqual(ref)) {
    printf("FAILED. Input(tcfile):\n%s\nReference:\n%s\n",
      input.GetDebugString().c_str(), ref.GetDebugString().c_str());
    return false;
  }

  printf("SUCCESS\n");
  return true;
}

bool testTCFileSave(const Input &ref)
{
  printf("Testing building Input from TC inputfile... ");
  fflush(stdout);

  printf("SUCCESS\n");
}

int main(int argc, char **argv)
{
  int failed = 0;

  // Create reference
  vector<string> atoms{"O", "H", "H"};
  double geom[9];
  geom[0] = -0.22968;
  geom[1] = -0.22984;
  geom[2] = -0.22951;
  geom[3] =  0.73821;
  geom[4] = -0.19699;
  geom[5] = -0.19671;
  geom[6] = -0.50853;
  geom[7] =  0.42683;
  geom[8] =  0.42622;
  for (int i = 0; i < 9; ++i) {
    geom[i] *= TCPB::constants::ANGSTROM_TO_AU;
  }

  map<string, string> options = {
    {"run", "gradient"}, {"method", "ub3lyp"}, {"basis", "6-31g**"},
    {"charge", "0"}, {"spinmult", "1"}, {"guess", "scr/ca0 scr/cb0"},
    {"precision", "double"}
  };

  Input ref(atoms, options, geom);

  // Run tests
  if (!testTCFileLoadPartial(ref)) {
    failed++;
  }
  if (!testTCFileLoadFull(ref)) {
    failed++;
  }
  if (!testTCFileLoadBohr(ref)) {
    failed++;
  }

  options["method"] = "revpbe";
  Input ref2(atoms, options, geom);
  if (!testTCFileLoadMethod(ref2)) {
    failed++;
  }

  if (failed) {
    printf("FAILED %d INPUT TESTS\n\n", failed);
  } else {
    printf("PASSED ALL INPUT TESTS\n\n");
  }

  return failed;
}
