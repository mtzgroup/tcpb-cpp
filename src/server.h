/** \file server.h
 *  \brief Definition of Server class
 */

#ifndef TCPB_SERVER_H_
#define TCPB_SERVER_H_

#include <string>

#include "socket.h"
#include "input.h"
#include "output.h"

namespace TCPB {

/**
 * \brief TeraChem Protocol Buffer (TCPB) Server class
 *
 * Server handles responding to clients passed on by select() multiplexing thread
 * Takes one active client for computing and responds with busy to all others
 **/
class Server {
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
    const Input& GetJobInput() { return currInput_; }

    /**
     * \brief Blocking call for sending job output
     *
     * @param Output object for completed job
     **/
    void SendJobOutput(const Output& out);

  private:
    SelectServerSocket* server_;
    const Socket* activeClient_;
    int currJobId_;

    // TODO: How to handle client died?
    bool acceptJob_;  //!< Flag to indicate Server can set currInput_
    bool jobCompleted_; //!< Flag to indicate we can send currOutput_ to activeClient_
    bool clientNotified_; //!< Flag to indicate activeClient_ is ready for currOutput_

    Input currInput_;
    Output currOutput_;

    /**
     * \brief Provide response to waiting clients
     *
     * Usually accepts Status/Input and responds with Status busy
     * If acceptJob_ == true, accepts Input and sets currInput_ and activeClient_
     * If jobCompleted_, will accept Status and respond with Status + currOutput_ to activeClient_
     *
     * @return Input object for newly accepted job
     **/
    bool HandleMessage(const Socket& client);
}; // end class Server

} // end namespace TCPB

#endif
