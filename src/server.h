/** \file server.h
 *  \brief Definition of Server class
 */

#ifndef TCPB_SERVER_H_
#define TCPB_SERVER_H_

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

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
class Server {
public:
  //Constructor/Destructor
  /**
   * \brief Constructor for Server class
   *
   * @param port Integer port of TCPB server
   * @param selectSleep Microseconds to sleep between select() checks
   **/
  Server(int port, float selectSleep = 100000);

  /**
   * \brief Destructor for Server
   **/
  ~Server();

  // Rule of 5: Not moveable or copyable
  Server(Server &&move)             = delete; // Move constructor
  Server &operator=(Server &&move)  = delete; // Move operator
  Server(const Server &)            = delete; // Copy constructor
  Server &operator=(const Server &) = delete; // Copy operator

  /**
   * \brief Blocking call for new job input
   *
   * @return Input object for newly accepted job
   **/
  const Input RecvJobInput();

  /**
   * \brief Blocking call for sending job output
   *
   * @param out Output object for completed job
   **/
  void SendJobOutput(const Output &out);

private:
  int socket_;                    //!< Server socket file descriptor
  std::thread listenThread_;      //!< Thread for select() loop
  std::mutex listenMutex_;        //!< Mutex for select() loop thread
  fd_set activefds_;              //!< Set of active sockets in select() loop
  int maxfd_;                     //!< Largest file descriptor in activefds_
  std::atomic<bool> exitFlag_;    //!< Flag for exiting select() loop
  const int selectSleep_;         //!< Microseconds for select() sleep

  char serverDir_[MAX_STR_LEN];     //!< Server directory
  int stdoutFD_;                    //!< File descriptor for stdout

  int currJobId_;                   //!< Current job id
  char currJobDir_[MAX_STR_LEN];    //!< Current job subdirectory
  std::atomic<int> currJobSFD_;     //!< Active client file descriptor

  Input *currInput_;                //!< Input object for current job
  Output *currOutput_;              //!< Output object for current job

  std::atomic<bool> acceptJob_;     //!< Inter-thread flag to populate currInput_
  std::atomic<bool> jobCompleted_;  //!< Inter-thread flag to send currOutput_

  /**
   * \brief Run the select() loop to multiplex the listening socket
   **/
  void RunSelectLoop();

  /**
   * \brief Provide response to waiting clients
   *
   * Developer note: This function is run on the select() thread. Be careful with
   * member variable use (either use atomics or lock the listenMutex_)
   *
   * @param sfd Socket file descriptor
   * @return Input object for newly accepted job
   **/
  bool HandleClientMessage(int sfd);

  /**
   * \brief Provide Status response to waiting clients
   *
   * @param client Socket for client
   * @param code Status code to send (0/1 for ready/busy, 2-4 matching terachem_server::JobStatusCase for active client)
   * @return True if successful send
   **/
  bool SendStatus(const Socket &client, int code);

  /**
   * \brief Reset stdout and flags for active client
   **/
  void ResetActiveClient();

  /**
  * \brief Shutdown a client socket
  *
  * @param sfd Socket file descriptor
  **/
  void ShutdownClient(int sfd);
}; // end class Server

} // end namespace TCPB

#endif
