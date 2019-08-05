/** \file tcpb.h
 *  \brief Definition of TCPBClient class
 *  \author Stefan Seritan <sseritan@stanford.edu>
 *  \date Jul 2017
 */

#ifndef TCPB_H_
#define TCPB_H_

#include <string>

#include "terachem_server.pb.h"

#ifndef MAX_STR_LEN
#define MAX_STR_LEN 1024
#endif

/**
 * \brief TeraChem Protocol Buffer (TCPB) Client class
 *
 * TCPBClient handles communicating with a TeraChem server through sockets and protocol buffers.
 * This class is based on protobufserver.cpp/.h in the TeraChem source code and tcpb.py (which came first).
 * Direct control of the asynchronous server communication is possible,
 * but the typical use would be the convenience functions like ComputeEnergy().
 * One major difference to the TCPB server code is that the client only needs one active connection.
 * This removes most threading and select logic (but limits communication to one server);
 * however, timeouts do need to be explicitly set on the socket.
 **/
class TCPBClient {
  public:
    //Constructor/Destructor
    /**
     * \brief Constructor for TCPBClient class
     *
     * Sets up the logfile if the #SOCKETLOGS macro is defined.
     *
     * @param host C string of hostname with TCPB server
     * @param port Integer port of TCPB server
     **/
    TCPBClient(const char* host,
               int port);

    /**
     * \brief Destructor for TCPBClient
     *
     * The destructor also handles disconnect and logfile cleanup.
     **/
    ~TCPBClient();

    /***********************
     * JOB INPUT (SETTERS) *
     ***********************/
    /**
     * \brief Set the atom types in the JobInput Protocol Buffer
     *
     * Also clears saved MO coefficients in jobInput_,
     * since changing the atoms invalidates the previous solution.
     *
     * @param atoms Array of C strings for atom types
     * @param num_atoms Integer number of entries in atoms
     **/
    void SetAtoms(const char** atoms,
                  const int num_atoms);

    /**
     * \brief Set the charge in the JobInput Protocol Buffer
     *
     * Also clears saved MO coefficients in jobInput_,
     * since changing the charge invalidates the previous solution.
     *
     * @param charge Molecular charge
     **/
    void SetCharge(const int charge);

    /**
     * \brief Set the spin multiplicity in the JobInput Protocol Buffer
     *
     * Also clears saved MO coefficients in jobInput_,
     * since changing the spin multiplicity invalidates the previous solution.
     *
     * @param spinMult Spin multiplicity
     **/
    void SetSpinMult(const int spinMult);

    /**
     * \brief Set closed or open shell in the JobInput Protocol Buffer
     *
     * Also clears saved MO coefficients in jobInput_,
     * since changing between closed and open shell invalidates the previous solution.
     *
     * @param closed If True, the system is set as closed shell
     **/
    void SetClosed(const bool closed);

    /**
     * \brief Set restricted or unrestricted in the JobInput Protocol Buffer
     *
     * Also clears saved MO coefficients in jobInput_,
     * since changing between restricted and unrestricted invalidates the previous solution.
     *
     * @param restricted If True, the system is set as restricted
     **/
    void SetRestricted(const bool restricted);

    /**
     * \brief Set the TeraChem method in the JobInput Protocol Buffer
     *
     * Clears saved MO coefficients in jobInput_,
     * since changing the method invalidates the previous solution.
     * Also, will error out if not given a valid TeraChem method (as defined in terachem_server.proto).
     *
     * @param method C string of method name (case insensitive)
     **/
    void SetMethod(const char* method);

    /**
     * \brief Set the TeraChem basis set in the JobInput Protocol Buffer
     *
     * Clears saved MO coefficients in jobInput_,
     * since changing the basis set invalidates the previous solution.
     * Also, will error out if not a valid TeraChem basis set (as defined in terachem_server.proto).
     *
     * @param basis C string of basis set name (case insensitive)
     **/
    void SetBasis(const char* basis);

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
     * \brief Connect to the TCPB server
     *
     * Initializes the server_ socket and connects to the given host (host_) and port (port_).
     **/
    void Connect();

    /**
     * \brief Disconnect from the TCPB server
     *
     * Disconnects and discards the server_ socket.
     **/
    void Disconnect();

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
    char host_[MAX_STR_LEN];
    int port_;
    int server_;
    FILE* clientLogFile_;
    // Protocol buffer variables
    terachem_server::JobInput jobInput_;
    terachem_server::JobOutput jobOutput_;
    // State variables, ensuring everything is set prior to sending the job
    bool atomsSet, chargeSet, spinMultSet, closedSet, restrictedSet, methodSet, basisSet;

    /***************************
     * SOCKET HELPER FUNCTIONS *
     ***************************/
    // TODO: These should probably be split out, pretty independent
    // TODO: These functions will not work on Windows at the moment
    /**
     * \brief A high-level socket recv with error checking and clean up for broken connections
     *
     * @param buf Buffer for incoming packet
     * @param len Byte size of incoming packet
     * @param log String message to be printed out as part of SocketLog messages (easier debugging)
     * @return status True if recv'd full packet, False otherwise (indicating server_ socket is now closed)
     **/
    bool HandleRecv(char* buf,
                    int len,
                    const char* log);

    /**
     * \brief A high-level socket send with error checking and clean up for broken connections
     *
     * @param buf Buffer for outgoing packet
     * @param len Byte size of outgoing packet
     * @param log String message to be printed out as part of SocketLog messages (easier debugging)
     * @return status True if sent full packet, False otherwise (indicating server_ socket is now closed)
     **/
    bool HandleSend(const char* buf,
                    int len,
                    const char* log);

    /**
     * \brief A low-level socket recv wrapper to ensure full packet recv
     *
     * @param buf Buffer for incoming packet
     * @param len Byte size of incoming packet
     * @return nsent Number of bytes recv'd
     **/
    int RecvN(char* buf,
              int len);

    /**
     * \brief A low-level socket send wrapper to ensure full packet send
     *
     * @param buf Buffer for outgoing packet
     * @param len Byte size of outgoing packet
     * @return nsent Number of bytes sent
     **/
    int SendN(const char* buf,
              int len);

    /**
     * \brief Verbose logging with timestamps for the client socket into "client.log"
     *
     * With SOCKETLOGS defined, this method is analagous to fprintf(clientLogFile_, format, asctime(), ...).
     *
     * Without SOCKETLOGS defined, this method does nothing.
     *
     * @param format Format string for fprintf
     * @param va_args Variable arguments for fprintf
     **/
    void SocketLog(const char* format, ...);
};

#endif
