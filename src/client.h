/** \file client.h
 *  \brief Definition of Client class
 */

#ifndef TCPB_CLIENT_H_
#define TCPB_CLIENT_H_

#include <string>

#include "socket.h"
#include "input.h"
#include "output.h"

namespace TCPB {

/**
 * \brief TeraChem Protocol Buffer (TCPB) Client class
 *
 * Client handles communicating with a TeraChem server through sockets and protocol buffers.
 * Direct control of the asynchronous server communication is possible,
 * but the typical use would be the convenience functions like ComputeEnergy().
 **/
class Client {
public:
  /**
   * \brief Constructor for Client class
   *
   * @param host Hostname of TCPB server
   * @param port Integer port of TCPB server
   **/
  Client(std::string host,
    int port);

  /**
   * \brief Destructor for Client
   **/
  ~Client();

  /**
   * \brief Accessor for previous job output
   *
   * @return Output object for last job
   **/
  const Output GetPrevResults() {
    return prevResults_;
  }

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
   * \brief Send a JobInput Protocol Buffer to the TCPB server
   *
   * The client sends a JobInput protobuf and waits for a Status protobuf,
   * which indicates whether the server has accepted or declined the job.
   * This is asynchronous in sense that the function does not wait for job completion.
   *
   * @param input Input with JobInput protocol buffer
   * @return True if job was submitted, False if server was busy
   **/
  bool SendJobAsync(const Input &input);

  /**
   * \brief Send a Status Protocol Buffer to the TCPB server to check on a submitted job
   *
   * The client sends a Status protobuf and waits for Status protobuf,
   * which indicates whether the server is still working on or has completed the submitted job.
   *
   * @return True if job is complete, False if job is still in progress
   **/
  bool CheckJobComplete();

  /**
   * \brief Receive the JobOutput Protocol Buffer from the TCPB server
   *
   * @return Output wrapping JobOutput protocol buffer
   **/
  const Output RecvJobAsync();

  /**
   * \brief Blocking wrapper for SendJobAsync(), CheckJobComplete(), and RecvJobAsync()
   *
   * The client repeatedly tries to submit and check on the status of the job, until job completion.
   * Called exactly like SendJobAsync(), but blocks until the job is finished and stored in jobOutput_.
   *
   * @param input Input with JobInput protocol buffer
   * @return Output wrapping JobOutput protocol buffer
   **/
  const Output ComputeJobSync(const Input &input);

  /*************************
   * CONVENIENCE FUNCTIONS *
   *************************/
  /**
   * \brief Blocking wrapper for an energy ComputeJobSync() call
   *
   * Calls ComputeJobSync for an energy calculation,
   * and abstracts away the Protobuf-specific runtype/unit code
   *
   * @param input Input with JobInput protocol buffer
   * @param energy Double for storing the computed energy
   * @return Copy of job Output data
   **/
  const Output ComputeEnergy(const Input &input,
    double &energy);

  /**
   * \brief Blocking wrapper for a gradient ComputeJobSync() call
   *
   * Calls ComputeJobSync for a gradient calculation,
   * and abstracts away the Protobuf-specific runtype/unit code
   *
   * @param input Input with JobInput protocol buffer
   * @param energy Double for storing the computed energy
   * @param qmgradient Double array for storing the computed gradient in the QM region (user-allocated)
   * @param mmgradient Double array for storing the computed gradient in the MM region (user-allocated)
   * @return Copy of job Output data
   **/
  const Output ComputeGradient(const Input &input,
    double &energy,
    double *qmgradient,
    double *mmgradient = nullptr);

  /**
   * \brief Blocking wrapper for a gradient ComputeJobSync() call
   *
   * Exactly the same as ComputeGradient(), but returns -gradient as forces
   *
   * @param input Input with JobInput protocol buffer
   * @param energy Double for storing the computed energy
   * @param qmforces Double array for storing the negative of the computed gradient of the QM region (user-allocated)
   * @param mmforces Double array for storing the negative of the computed gradient of the MM region (user-allocated)
   * @return Copy of job Output data
   **/
  const Output ComputeForces(const Input &input,
    double &energy,
    double *qmforces,
    double *mmforces = nullptr);

private:
  std::string host_;
  int port_;
  ClientSocket *socket_;

  std::string currJobDir_;
  std::string currJobScrDir_;
  int currJobId_;

  Output prevResults_;
}; // end class Client

} // end namespace TCPB

#endif
