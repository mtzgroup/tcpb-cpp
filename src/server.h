/** \file server.h
 *  \brief Definition of Server class
 */

#ifndef TCPB_SERVER_H_
#define TCPB_SERVER_H_

#include <atomic>
#include <string>

#include "socket.h"
#include "input.h"
#include "output.h"
#include "terachem_server.pb.h"

namespace TCPB {

/**
 * \brief TeraChem Protocol Buffer (TCPB) Server class
 *
 * Server handles responding to clients passed on by select() multiplexing thread
 * Takes one active client for computing and responds with busy to all others
 **/
class Server : public SelectServerSocket {
  public:
    //Constructor/Destructor
    /**
     * \brief Constructor for Server class
     *
     * @param port Integer port of TCPB server
     **/
    Server(int port);

    /**
     * \brief Destructor for Server
     **/
    ~Server();

    /**
     * \brief Blocking call for new job input
     *
     * @return Input object for newly accepted job
     **/
    const Input RecvJobInput();

    /**
     * \brief Blocking call for sending job output
     *
     * @param Output object for completed job
     **/
    void SendJobOutput(const Output& out);

  private:
    char serverDir_[MAX_STR_LEN];     //!< Server directory
    int stdoutFD_;                    //!< File descriptor for stdout

    int currJobId_;                   //!< Current job id
    char currJobDir_[MAX_STR_LEN];    //!< Current job subdirectory
    std::atomic<int> currJobSFD_;     //!< Socket file descriptor for active client (-1 while no active job)

    // NOTE: Should probably use the listenMutex_ when touching these
    // All other variables touched in both threads
    Input* currInput_;                //!< Input object for current job
    Output* currOutput_;              //!< Output object for current job

    std::atomic<bool> acceptJob_;     //!< Flag to indicate Server can set currInput_
    std::atomic<bool> jobCompleted_;  //!< Flag to indicate we can send currOutput_ to activeClient_

    /**
     * \brief Provide response to waiting clients
     *
     * Developer note: This function is run on the select() thread. Be careful with
     * member variable use (either use atomics or lock the listenMutex_)
     *
     * @param sfd Socket file descriptor
     * @return Input object for newly accepted job
     **/
    bool HandleMessage(int sfd);

    /**
     * \brief Provide Status response to waiting clients
     *
     * @param client Socket for client
     * @param code Status code to send (0/1 for ready/busy, 2-4 matching terachem_server::JobStatusCase for active client)
     * @return True if successful send
     **/
    bool SendStatus(const Socket& client, int code);
}; // end class Server

} // end namespace TCPB

#endif
