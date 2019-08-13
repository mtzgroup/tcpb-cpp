/** \file utils.h
 *  \brief Definition of TCPBUtils functions
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <map>
#include <string>
#include <vector>

#include "constants.h"

namespace TCPBUtils {

/**
 * \brief Read old-style TeraChem input file
 *
 * @param tcfile Filename of TeraChem input file
 * @return Key-value pairs of options
 **/
std::map<std::string, std::string> ReadTCFile(std::string tcfile);

/**
 * \brief Read XYZ file
 *
 * @param xyzfile Filename of XYZ file
 * @param atoms Array of atom symbols to be filled
 * @param geom Array of xyz positions to be filled
 * @param scale Unit conversion to use (defaults to ANGSTROM_TO_AU)
 **/
void ReadXYZFile(std::string xyzfile,
                 std::vector<std::string>& atoms,
                 std::vector<double>& geom,
                 double scale = constants::ANGSTROM_TO_AU);

} // end namespace TCPBUtils

#endif
