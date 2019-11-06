/** \file utils.h
 *  \brief Definition of TCPBUtils functions
 */

#ifndef TCPB_UTILS_H_
#define TCPB_UTILS_H_

#include <map>
#include <string>
#include <vector>

#include "constants.h"

namespace TCPB {

namespace Utils {

/**
 * \brief Read old-style TeraChem input file
 *
 * @param tcfile Filename of TeraChem input file
 * @return Key-value pairs of options
 **/
std::map<std::string, std::string> ReadTCFile(std::string tcfile);

/**
 * \brief Write old-style TeraChem input file
 *
 * @param tcfile Filename of TeraChem input file
 * @param options Key-value pairs
 **/
void WriteTCFile(std::string tcfile,
  const std::map<std::string, std::string> &options);

/**
 * \brief Convert between TC input and protobuf for closed/restricted
 *
 * Sets closed/restricted based on method prefix (which is removed)
 * Prefix r: closed/restricted = true
 * Prefix ro: closed = false, restricted = true
 * Prefix u: closed/restricted = false
 * No prefix is treated as prefix r
 *
 * @param method TC-style method to parse
 * @param closed Whether or not orbitals are closed-shell
 * @param restricted Whether or not orbitals are restricted
 **/
void ParseMethod(std::string &method,
  bool &closed,
  bool &restricted);

/**
 * \brief Read XYZ file
 *
 * @param xyzfile Filename of XYZ file
 * @param atoms Array of atom symbols to be filled
 * @param geom Array of xyz positions to be filled
 * @param scale Unit conversion to use (defaults to ANGSTROM_TO_AU)
 **/
void ReadXYZFile(std::string xyzfile,
  std::vector<std::string> &atoms,
  std::vector<double> &geom,
  double scale = constants::ANGSTROM_TO_AU);

/**
 * \brief Write XYZ file
 *
 * @param xyzfile Filename of XYZ file
 * @param atoms Array of atom symbols to be filled
 * @param geom Array of xyz positions to be filled
 * @param comment Comment line
 * @param scale Unit conversion to use (defaults to 1/ANGSTROM_TO_AU)
 **/
void WriteXYZFile(std::string xyzfile,
  const std::vector<std::string> &atoms,
  const std::vector<double> &geom,
  std::string comment,
  double scale = 1 / constants::ANGSTROM_TO_AU);

/**
 * \brief Helper function to uppercase a C++ string
 *
 * @param str String to uppercase
 * @return Uppercased string
 **/
std::string ToUpper(const std::string &str);

/**
 * \brief Helper function to lowercase a C++ string
 *
 * @param str String to lowercase
 * @return Lowercased string
 **/
std::string ToLower(const std::string &str);

} // end namespace Utils

} // end namespace TCPB

#endif
