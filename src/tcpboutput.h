/** \file tcpboutput.h
 *  \brief Definition of TCPBOutput class
 */

#ifndef TCPBOUTPUT_H_
#define TCPBOUTPUT_H_

#include "terachem_server.pb.h"

/**
 * \brief TeraChem Protocol Buffer (TCPB) Output class
 *
 * TCPBOutput is a lightweight wrapper on top of the JobOutput protocol buffer.
 * Storing directly in protobuf is nice because it serializes and has explicit typing.
 * This class is designed to solidify the TCPB interface with explicit getters
 * and avoid developers needing to learn how to use protobufs.
 **/
class TCPBOutput {
  public:
    /**
     * \brief Constructor for TCPBOutput class
     *
     * @param pb JobOutput protobuf to wrap
     **/
    TCPBOutput(terachem_server::JobOutput pb) : pb_(pb) {}

    /**
     * \brief Alternate constructor for TCPBOutput class
     **/
    TCPBOutput() : pb_(terachem_server::JobOutput()) {}

    /**
     * \brief Destructor for TCPBClient
     **/
    ~TCPBOutput() = default;

    /**
     * \brief Gets the energy from a JobOutput Protocol Buffer
     *
     * @param energy Double reference of computed energy
     * @param state State index (defaults to 0, ground state)
     * @param mult Spin multiplicity (defaults to 1, singlet)
     **/
    void GetEnergy(double& energy,
                   int state=0,
                   int mult=1);

    /**
     * \brief Gets the gradient from a JobOutput Protocol Buffer
     *
     * @param gradient Double array to store computed gradient (user-allocated)
     * @param state State index (defaults to 0, ground state)
     * @param mult Spin multiplicity (defaults to 1, singlet)
     **/
    void GetGradient(double* gradient,
                     int state=0,
                     int mult=1);

    /**
     * \brief Accessor for internal protobuf object
     *
     * @return Reference to internal protobuf object
     **/
    const terachem_server::JobOutput& GetOutputPB() { return pb_; }   

  private:
    terachem_server::JobOutput pb_; //!< Internal protobuf for advanced manipulation
};

#endif