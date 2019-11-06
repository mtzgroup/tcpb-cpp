/** \file output.cpp
 *  \brief Implementation of Output class
 */

#include <string>

#include "output.h"

#include "terachem_server.pb.h"
using terachem_server::JobOutput;

namespace TCPB {

void Output::GetEnergy(double &energy,
  int state,
  int mult) const
{
  // TODO: Add state/mult logic
  energy = pb_.energy(0);
}

void Output::GetGradient(double *gradient,
  int state,
  int mult) const
{
  // TODO: Add state/mult logic
  int grad_size = pb_.gradient_size();
  memcpy(gradient, pb_.gradient().data(), grad_size * sizeof(double));
}

} // end namespace TCPB
