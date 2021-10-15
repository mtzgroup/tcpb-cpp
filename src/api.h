/** \file api.h
 *  \brief Houses the functionalities to make TCPB-cpp an API that can be accessed from Fortran
 */

#ifndef TCPB_API_H_
#define TCPB_API_H_

extern "C" {

  /**
   * \brief Connects to TeraChem server
   *
   * @param[in] host Address of the host
   * @param[in] port Port number
   * @param[out] status Status of execution: 0, all is good;
   *                                         1, could not connect to server;
   *                                         2, connected to server but it is not available
   **/
  void tc_connect_(const char host[80], const int* port, int* status);

  /**
   * \brief Setup TeraChem protobuf input variable
   *
   * @param[in] tcfile Path to the TeraChem input file
   * @param[in] qmattypes List of atomic types in the QM region
   * @param[in] numqmatoms Number of atoms in the QM region
   * @param[in] nummmatoms Number of atoms in the MM region
   * @param[out] status Status of execution: 0, all is good
   *                                         1, no options read from tcfile
   *                                         2, failed to setup
   **/
  void tc_setup_(const char tcfile[256], const char qmattypes[][5], const int* numqmatoms, int* status);

  /**
   * \brief Compute energy and gradient using TeraChem
   *\
   * @param[in] qmattypes List of atomic types in the QM region
   * @param[in] qmcoords Coordinates of the atoms in the QM region (unit: Bohrs)
   * @param[in] numqmatoms Number of atoms in the QM region
   * @param[out] totenergy Total energy of the QM in the presence of the MM region (unit: Hartrees)
   * @param[out] qmgrad Gradient of the atoms in the QM region (unit: Hartree/Bohr)
   * @param[out] status Status of execution: 0, all is good
   *                                         1, mismatch in the variables passed to the function
   *                                         2, calculation failed
   * @param[in] mmcoords Coordinates of the atoms in the MM region, optional (unit: Bohrs)
   * @param[in] nummmatoms Number of atoms in the MM region, optional
   * @param[out] mmgrad Gradient of the atoms in the MM region, optional (unit: Hartree/Bohr)
   **/
  void tc_compute_energy_gradient_(const char qmattypes[][5], const double* qmcoords, const int* numqmatoms,
    double* totenergy, double* qmgrad, const double* mmcoords, const double* mmcharges,
    const int* nummmatoms, double* mmgrad, int* status);

  /**
   * \brief Deletes from memory variables that are open
   **/
  void tc_finalize_();

} // extern "C"

#endif
