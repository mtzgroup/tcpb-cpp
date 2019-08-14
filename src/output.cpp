/** \file output.cpp
 *  \brief Implementation of TCPBOutput class
 */

#include <string>

#include "output.h"
#include "terachem_server.pb.h"
using terachem_server::JobOutput;

void TCPBOutput::GetEnergy(double& energy,
                           int state,
                           int mult) const {
  // TODO: Add state/mult logic
  energy = pb_.energy(0);
}

void TCPBOutput::GetGradient(double* gradient,
                             int state,
                             int mult) const {
  // TODO: Add state/mult logic
  int grad_size = pb_.gradient_size();
  memcpy(gradient, pb_.gradient().data(), grad_size*sizeof(double));
}
