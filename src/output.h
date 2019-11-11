/** \file output.h
 *  \brief Definition of Output class
 */

#ifndef TCPB_OUTPUT_H_
#define TCPB_OUTPUT_H_

#include "terachem_server.pb.h"

namespace TCPB {

/**
 * \brief TeraChem Protocol Buffer (TCPB) Output class
 *
 * Output is a lightweight wrapper on top of the JobOutput protocol buffer.
 * Storing directly in protobuf is nice because it serializes and has explicit typing.
 * This class is designed to solidify the TCPB interface with explicit getters
 * and avoid developers needing to learn how to use protobufs.
 **/
class Output {
public:
  /**
   * \brief Constructor for Output class
   *
   * @param pb JobOutput protobuf to wrap
   **/
  Output(terachem_server::JobOutput pb) : pb_(pb) {}

  /**
   * \brief Alternate constructor for Output class
   **/
  Output() : pb_(terachem_server::JobOutput()) {}

  /**
   * \brief Gets the energy from a JobOutput Protocol Buffer
   *
   * For now, will assume either CIS or CAS being populated
   *
   * @param energy Double reference of computed energy
   * @param state State index (defaults to 0, ground state)
   * @param mult Spin multiplicity (defaults to 1, singlet)
   **/
  void GetEnergy(double &energy,
    int state = 0,
    int mult = 1) const;

  /**
   * \brief Gets the gradient from a JobOutput Protocol Buffer
   *
   * @param gradient Double array to store computed gradient (user-allocated)
   * @param state State index (defaults to 0, ground state)
   * @param mult Spin multiplicity (defaults to 1, singlet)
   **/
  void GetGradient(double *gradient,
    int state = 0,
    int mult = 1) const;

  /**
   * \brief Accessor for internal protobuf object
   *
   * @return Reference to internal protobuf object
   **/
  const terachem_server::JobOutput &GetOutputPB() const {
    return pb_;
  }

  /**
   * \brief Check internal deserialized data is equivalent
   *
   * Uses fuzzy equality for numbers
   *
   * @return True if Output objects are equivalent
   **/
  bool IsApproxEqual(const Output &other) const;

private:
  terachem_server::JobOutput pb_; //!< Internal protobuf for advanced manipulation
}; // end class Output

} // end namespace TCPB

#endif
