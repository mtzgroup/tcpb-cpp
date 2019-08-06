/** \file utils.h
 *  \brief Definition of TCPBClient class
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <map>
#include <string>
#include <vector>

namespace TCPBUtils {

/**
 * \brief Read old-style TeraChem input file
 *
 * @param tcfile Filename of TeraChem input file
 * @return Key-value pairs of options
 **/
std::map<std::string, std::string> ReadTCInput(std::string tcfile);

/**
 * \brief Read XYZ file
 *
 * @param xyzfile Filename of XYZ file
 * @param atoms Array of atom symbols to be filled
 * @param geom Array of xyz positions to be filled
 **/
void ReadXYZFile(std::string xyzfile,
                 std::vector<std::string>& atoms,
                 std::vector<double>& geom);

} // end namespace TCPBUtils

#endif
