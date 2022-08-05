/** \file output.cpp
 *  \brief Implementation of Output class
 */

#include <string>

#include <google/protobuf/util/message_differencer.h>

#include "output.h"

#include "terachem_server.pb.h"
using terachem_server::JobOutput;

namespace TCPB {

void Output::GetEnergy(double &energy,
  int state) const
{
  energy = pb_.energy(state);
}

void Output::SetEnergy(double energy)
{
  pb_.add_energy(energy);
}

void Output::GetGradient(double *qmgradient,
  double *mmgradient) const
{
  int qmgrad_size = pb_.gradient_size();
  memcpy(qmgradient, pb_.gradient().data(), qmgrad_size * sizeof(double));
  if (mmgradient != nullptr) {
    int mmgrad_size = pb_.mmatom_gradient_size();
    memcpy(mmgradient, pb_.mmatom_gradient().data(), mmgrad_size * sizeof(double));
  }
}

void Output::GetCharges(double *qmcharges) const
{
  int charges_size = pb_.charges_size();
  memcpy(qmcharges, pb_.charges().data(), charges_size * sizeof(double));
}

bool Output::IsApproxEqual(const Output &other) const
{
  using namespace google::protobuf::util;
  return MessageDifferencer::ApproximatelyEquals(pb_, other.pb_);
}

} // end namespace TCPB
