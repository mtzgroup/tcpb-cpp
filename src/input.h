/** \file input.h
 *  \brief Definition of Input class
 */

#ifndef TCPB_INPUT_H_
#define TCPB_INPUT_H_

#include <map>
#include <string>
#include <vector>

#include "terachem_server.pb.h"

namespace TCPB {

/**
 * \brief TeraChem Protocol Buffer (TCPB) Input class
 *
 * Input is a lightweight wrapper on top of the JobInput protocol buffer.
 * Storing directly in protobuf is nice because it serializes and has explicit typing.
 * This class is designed to solidify the TCPB interface with explicit setters
 * and avoid developers needing to learn how to use protobufs.
 **/
class Input {
  public:
    /**
     * \brief Direct protobuf constructor for Input class
     *
     * @param pb JobInput protobuf object
     **/
    Input(const terachem_server::JobInput& pb) : pb_(pb) {};

    /**
     * \brief Constructor for Input class
     *
     * @param atoms Atomic symbols
     * @param options Map of key-value pairs for TCPB or TeraChem options
     * @param geom 1D array of atomic positions in a.u.
     * @param geom2 1D array of atomic positions in a.u. (default to NULL, needed for overlap jobs)
     **/
    Input(const std::vector<std::string>& atoms,
          const std::map<std::string, std::string>& options,
          const double* const geom,
          const double* const geom2 = NULL);

    /**
     * \brief Alternate file-based constructor for Input class
     *
     * @param tcfile Template TeraChem input deck
     * @param xyzfile Geometry file (default to "", will try to read coordinates option from input deck)
     * @param xyzfile2 Second geometry file for overlap jobs (default to "", will try to read old_coors option from input deck)
     **/
    Input(std::string tcfile,
          std::string xyzfile = "",
          std::string xyzfile2 = "");

    /**
     * \brief Accessor for internal protobuf object
     *
     * @return Reference to internal protobuf object
     **/
    const terachem_server::JobInput& GetInputPB() const { return pb_; }

    /**
     * \brief Getter of protobuf string for debugging
     *
     * Note: This function does not print default values
     * (e.g. method: HF or closed: false would not appear in the printout)
     *
     * @return Debug string of internal protobuf object
     **/
    std::string GetDebugString() { return pb_.DebugString(); }

  private:
    terachem_server::JobInput pb_; //!< Internal protobuf object for advanced manipulation

    /**
     * \brief Helper function initialize protobuf object
     *
     * @param atoms Atomic symbols
     * @param options Map of key-value pairs for TCPB or TeraChem options
     * @param geom 1D array of atomic positions
     * @param geom2 1D array of atomic positions (default to NULL, needed for overlap jobs)
     * @return Initialized JobInput protobuf object
     **/
    terachem_server::JobInput InitInputPB(const std::vector<std::string>& atoms,
                                          const std::map<std::string, std::string>& options,
                                          const double* const geom,
                                          const double* const geom2 = NULL);

    /**
     * \brief Helper function to uppercase a C++ string
     *
     * @param str String to uppercase
     * @return Uppercased string
     **/
    std::string ToUpper(const std::string& str);
}; // end class Input

} // end namespace TCPB
#endif
