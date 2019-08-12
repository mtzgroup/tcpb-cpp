/** \file input.h
 *  \brief Definition of TCPBInput class
 */

#ifndef TCPBINPUT_H_
#define TCPBINPUT_H_

#include <map>
#include <string>
#include <vector>

#include "terachem_server.pb.h"

/**
 * \brief TeraChem Protocol Buffer (TCPB) Input class
 *
 * TCPBInput is a lightweight wrapper on top of the JobInput protocol buffer.
 * Storing directly in protobuf is nice because it serializes and has explicit typing.
 * This class is designed to solidify the TCPB interface with explicit setters
 * and avoid developers needing to learn how to use protobufs.
 **/
class TCPBInput {
  public:
    /**
     * \brief Constructor for TCPBInput class
     * 
     * The following keywords are required in options:
     * charge, spinmult, closed_shell, restricted, method, basis
     *
     * @param run TeraChem run type as defined in the JobInput_RunType enum (e.g. "energy", "gradient", etc.)
     * @param atoms Atomic symbols
     * @param options Map of key-value pairs for TCPB or TeraChem options
     * @param geom 1D array of atomic positions
     * @param geom2 1D array of atomic positions (default to NULL, needed for overlap jobs)
     **/
    TCPBInput(std::string run,
              const std::vector<std::string>& atoms,
              const std::map<std::string, std::string>& options,
              const double* const geom,
              const double* const geom2 = NULL);

    /**
     * \brief Alternate file-based constructor for TCPBInput class
     * 
     * The following keywords are required in tcfile:
     * charge, spinmult, closed_shell, restricted, method, basis
     *
     * @param tcfile Template TeraChem input deck
     * @param xyzfile Geometry file
     * @param xyzfile2 Second geometry file (default to "", needed for overlap jobs)
     **/
    TCPBInput(std::string tcfile,
              std::string xyzfile,
              std::string xyzfile2 = "");

    /**
     * \brief Destructor for TCPBClient
     **/
    ~TCPBInput() = default;

    /**
     * \brief Accessor for internal protobuf object
     *
     * @return Reference to internal protobuf object
     **/
    const terachem_server::JobInput& GetInputPB() const { return pb_; }

  private:
    terachem_server::JobInput pb_; //!< Internal protobuf object for advanced manipulation

    /**
     * \brief Helper function to uppercase a C++ string
     *
     * @param str String to uppercase
     * @return Uppercased string
     **/
    std::string ToUpper(const std::string& str);
};

#endif
