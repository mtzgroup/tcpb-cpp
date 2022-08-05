/** \file api.h
 *  \brief Houses the functionalities to make TCPB-cpp an API that can be accessed from Fortran
 */

#ifndef TCPB_API_H_
#define TCPB_API_H_

extern "C" {

  /**
   * \brief Connects to TeraChem server
   *
   * @param[in]  host Address of the host
   * @param[in]  port Port number
   * @param[out] status Status of execution: 0, all is good;
   *                                         1, could not connect to server;
   *                                         2, connected to server but it is not available
   **/
  void tc_connect_(const char host[80], const int* port, int* status);

  /**
   * \brief Setup TeraChem protobuf input variable
   *
   * @param[in]  tcfile Path to the TeraChem input file
   * @param[in]  qmattypes List of atomic types in the QM region
   * @param[in]  numqmatoms Number of atoms in the QM region
   * @param[out] status Status of execution: 0, all is good
   *                                         1, no options read from tcfile or mismatch in the input variables
   *                                         2, failed to setup
   **/
  void tc_setup_(const char tcfile[256], const char qmattypes[][5], const int* numqmatoms, int* status);

  /**
   * \brief Compute energy and gradient using TeraChem
   *\
   * @param[in]  qmattypes List of atomic types in the QM region
   * @param[in]  qmcoords Coordinates of the atoms in the QM region (unit: Bohrs)
   * @param[in]  numqmatoms Number of atoms in the QM region
   * @param[out] totenergy Total energy of the QM in the presence of the MM region (unit: Hartrees)
   * @param[out] qmgrad Gradient of the atoms in the QM region (unit: Hartree/Bohr)
   * @param[in]  mmcoords Coordinates of the atoms in the MM region, optional (unit: Bohrs)
   * @param[in]  mmccharges Charges of the atoms in the MM region, optional (unit: atomic units)
   * @param[in]  nummmatoms Number of atoms in the MM region, optional
   * @param[out] mmgrad Gradient of the atoms in the MM region, optional (unit: Hartree/Bohr)
   * @param[in]  globaltreatment Global treatment: 0, when the number of QM atoms is the same as in the
   *                                                  previous call, the mode of execution is automatically
   *                                                  switched from NEW CONDITION to CONTINUE; if number of
   *                                                  QM atoms changes, switch to NEW_CONDITION (Default)
   *                                               1, always use NEW_CONDITION as the mode of execution
   *                                               2, always use NORMAL as the mode of execution
   * @param[out] status Status of execution: 0, all is good
   *                                         1, mismatch in the variables passed to the function
   *                                         2, calculation failed
   **/
  void tc_compute_energy_gradient_(const char qmattypes[][5], const double* qmcoords, const int* numqmatoms,
    double* totenergy, double* qmgrad, const double* mmcoords, const double* mmcharges,
    const int* nummmatoms, double* mmgrad, const int* globaltreatment, int* status);

  /**
   * \brief Gets the charges of the atoms in the QM region. Must be ran after tc_compute_energy_gradient_.
   *\
   * @param[out] qmccharges Charges of the atoms in the QM region (unit: atomic units)
   * @param[out] status Status of execution: 0, all is good
   *                                         1, calculation failed
   **/
  void tc_get_qm_charges_(double* qmcharges, int* status);

  /**
   * \brief Deletes from memory variables that are allocated
   **/
  void tc_finalize_();

} // extern "C"

#endif
