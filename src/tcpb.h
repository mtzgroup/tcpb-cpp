/** \file tcpb.h
 *  \brief Definition of TCPBClient class
 *  \author Stefan Seritan <sseritan@stanford.edu>
 *  \date Jul 2017
 */

#ifndef TCPB_H_
#define TCPB_H_

#include <string>

#include "socket.h"
#include "terachem_server.pb.h"

/**
 * \brief TeraChem Protocol Buffer (TCPB) Client class
 *
 * TCPBClient handles communicating with a TeraChem server through sockets and protocol buffers.
 * Direct control of the asynchronous server communication is possible,
 * but the typical use would be the convenience functions like ComputeEnergy().
 **/
class TCPBClient {
  public:
    //Constructor/Destructor
    /**
     * \brief Constructor for TCPBClient class
     *
     * @param host Hostname of TCPB server
     * @param port Integer port of TCPB server
     **/
    TCPBClient(std::string host,
               int port);

    /**
     * \brief Destructor for TCPBClient
     **/
    ~TCPBClient();

    /************************
     * JOB OUTPUT (GETTERS) *
     ************************/
    /**
     * \brief Gets the energy from the JobOutput Protocol Buffer
     *
     * Takes the energy from the current jobOutput_,
     * and stores it in the double passed in by reference.
     * This is to stay consistent with how the getters for arrays is coded.
     *
     * @param energy Double reference of computed energy
     **/
    void GetEnergy(double& energy);

    /**
     * \brief Gets the gradient from the JobOutput Protocol Buffer
     *
     * Takes the gradient array from the current jobOutput_.
     * It currently does not check the size of the array, which must be allocated by the user.
     *
     * @param gradient Double array of computed gradient (user-allocated)
     **/
    void GetGradient(double* gradient);

    //TODO: Add more getters

    /************************
     * SERVER COMMUNICATION *
     ************************/
    /**
     * \brief Checks whether the server is available
     *
     * Does not reserve the server, only returns the current availability.
     *
     * @return True if server has no running job, False otherwise
     **/
    bool IsAvailable();

    /**
     * \brief Send the JobInput Protocol Buffer to the TCPB server
     *
     * The client sends a JobInput protobuf and waits for a Status protobuf,
     * which indicates whether the server has accepted or declined the job.
     * The send and recv are actually blocking;
     * however, this is asynchronous in sense that the function does not wait for job completion.
     * Status responses should be immediate from the server.
     *
     * @param runType TeraChem run type, as defined in the JobInput_RunType enum 
     * @param geom Double array of XYZs for each atom
     * @param num_atoms Integer number of atoms stored in geom
     * @param unitType Geometry units, as defined in the Mol_UnitType enum
     * @return True if job was submitted, False if server was busy
     **/
    bool SendJobAsync(const terachem_server::JobInput_RunType runType,
                      const double* geom,
                      const int num_atoms,
                      const terachem_server::Mol_UnitType unitType);

    /**
     * \brief Send a Status Protocol Buffer to the TCPB server to check on a submitted job
     *
     * The client sends a Status protobuf and waits for Status protobuf,
     * which indicates whether the server is still working on or has completed the submitted job.
     * The send and recv are actually blocking;
     * however, this is asynchronous in sense that the function does not wait for job completion.
     * Status responses should be immediate from the server.
     *
     * @return True if job is complete, False if job is still in progress
     **/
    bool CheckJobComplete();

    /**
     * \brief Receive the JobOutput Protocol Buffer from the TCPB server
     *
     * The client receives a JobOutput protobuf from the server,
     * overwritting jobOutput_ with the new protobuf.
     * The MO coefficients are taken from jobOutput_ 
     * and placed as guess MO coefficients in jobInput_ for the next job.
     **/
    void RecvJobAsync();

    /**
     * \brief Blocking wrapper for SendJobAsync(), CheckJobComplete(), and RecvJobAsync()
     *
     * The client repeatedly tries to submit and check on the status of the job, until job completion.
     * Called exactly like SendJobAsync(), but blocks until the job is finished and stored in jobOutput_.
     *
     * @param runType TeraChem run type, as defined in the JobInput_RunType enum 
     * @param geom Double array of XYZs for each atom
     * @param num_atoms Integer number of atoms stored in geom
     * @param unitType Geometry units, as defined in the Mol_UnitType enum
     **/
    void ComputeJobSync(const terachem_server::JobInput_RunType runType,
                        const double* geom,
                        const int num_atoms,
                        const terachem_server::Mol_UnitType unitType);

    /*************************
     * CONVENIENCE FUNCTIONS *
     *************************/
    /**
     * \brief Blocking wrapper for an energy ComputeJobSync() call
     *
     * Calls ComputeJobSync for an energy calculation,
     * and abstracts away the Protobuf-specific runtype/unit code
     *
     * @param geom Double array of XYZs for each atom
     * @param num_atoms Integer number of atoms stored in geom
     * @param angstrom If True, geometry units are Angstrom instead of Bohr
     * @param energy Double for storing the computed energy
     **/
    void ComputeEnergy(const double* geom,
                       const int num_atoms,
                       const bool angstrom,
                       double& energy);

    /**
     * \brief Blocking wrapper for a gradient ComputeJobSync() call
     *
     * Calls ComputeJobSync for a gradient calculation,
     * and abstracts away the Protobuf-specific runtype/unit code
     *
     * @param geom Double array of XYZs for each atom
     * @param num_atoms Integer number of atoms stored in geom
     * @param angstrom If True, geometry units are Angstrom instead of Bohr
     * @param energy Double for storing the computed energy
     * @param gradient Double array for storing the computed gradient (user-allocated)
     **/
    void ComputeGradient(const double* geom,
                         const int num_atoms,
                         const bool angstrom,
                         double& energy,
                         double* gradient);

    /**
     * \brief Blocking wrapper for a gradient ComputeJobSync() call
     *
     * Exactly the same as ComputeGradient(), but returns -gradient as forces
     *
     * @param geom Double array of XYZs for each atom
     * @param num_atoms Integer number of atoms stored in geom
     * @param angstrom If True, geometry units are Angstrom instead of Bohr
     * @param energy Double for storing the computed energy
     * @param forces Double array for storing the negative of the computed gradient (user-allocated)
     **/
    void ComputeForces(const double* geom,
                       const int num_atoms,
                       const bool angstrom,
                       double& energy,
                       double* forces);

  private:
    TCPBSocket* socket_;
    terachem_server::JobInput jobInput_;
    terachem_server::JobOutput jobOutput_;
};

#endif
