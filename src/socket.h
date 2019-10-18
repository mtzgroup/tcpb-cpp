 /** \file tcpbsocket.h
 *  \brief Definition of Socket class
 *
 */

#ifndef TCPB_SOCKET_H_
#define TCPB_SOCKET_H_

#include <sys/time.h> // for fd_set

#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace TCPB {

/**
 * \brief TeraChem Protocol Buffer (TCPB) Socket class
 *
 * Helper class to manage a socket for Socket
 * Influenced by https://codereview.stackexchange.com/q/131137
 **/
class Socket {
  public:
    //Constructor/Destructor
    /**
     * \brief Constructor for Socket class
     *
     * @param logName Logfile name (defaults to socket.log)
     **/
    Socket(const std::string& logName = "socket.log");

    /**
     * \brief Destructor for Socket
     *
     * The destructor also handles disconnect and logfile cleanup.
     **/
    ~Socket();

    // Rule of 5: Moveable but not copyable
    void swap(Socket& other)          noexcept; // Helper swap function
    Socket(Socket&& move)             noexcept; // Move constructor
    Socket& operator=(Socket&& move)  noexcept; // Move operator
    Socket(const Socket&)             = delete; // Copy constructor
    Socket& operator=(const Socket&)  = delete; // Copy operator

    /**
     * \brief Check if connection is alive
     *
     * @return True if socket is active
     **/
     bool IsConnected() { return (socket_ != -1); }

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

  protected:
    int socket_;
    FILE* logFile_;

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
}; // end class Socket

/**
 * \brief ClientSocket class
 *
 * Uses the connect() function to bind a socket, designed for client usage
 **/
class ClientSocket : public Socket {
  public:
    /**
     * \brief Constructor for ClientSocket class
     *
     * @param host Server hostname
     * @param port Server port number
     **/
    ClientSocket(const std::string& host, int port);
}; // end class ClientSocket

/**
 * \brief SelectServerSocket class
 *
 * Uses the bind() and listen() functions to bind a socket, designed for server usage
 * Uses select() to multiplex sockets with a background thread
 * Basing this off of https://www.gnu.org/software/libc/manual/html_node/Server-Example.html
 * and http://www.binarytides.com/multiple-socket-connections-fdset-select-linux/
 *
 * A function callback is used so that this class can respond with "busy" messages to nonactive clients
 **/
class SelectServerSocket : public Socket {
  public:
    /**
     * \brief Constructor for SelectServerSocket class
     *
     * Launches select() loop in the background to handle multiple clients
     *
     * @param port Port to listen on
     * @param respCB Function callback to handle unactive client responses in select() loop
     **/
    SelectServerSocket(int port,
                 std::function<void(int)> replyCB);

    /**
     * \brief Destructor for SelectServerSocket class
     **/
    ~SelectServerSocket();

    // Rule of 5: Not moveable or copyable due to threading
    void swap(SelectServerSocket& other)                      = delete; // Helper swap function
    SelectServerSocket(SelectServerSocket&& move)             = delete; // Move constructor
    SelectServerSocket& operator=(SelectServerSocket&& move)  = delete; // Move operator

    /**
     * \brief Blocking call to wait for client connection
     *
     * select() is always running in background, this function
     * just exposes one of the client sockets as "active"
     *
     * @return Base socket for read/write access to client
     **/
    Socket AcceptClient();

  protected:
    std::thread listenThread_;  //!< Thread for select() loop
    std::mutex listenMutex_;    //!< Mutex for select() loop thread
    fd_set activefds_;          //!< Set of active sockets in select() loop
    int maxfd_;                 //!< Largest file descriptor in activefds_
    bool exitFlag_;             //!< Flag for exiting select() loop

    // Inter-thread variables
    bool accept_;               //!< Flag for select() loop populating acceptedSocket_;
    int activeClient_;          //!< Selected socket to return from AcceptClient() (-1 is inactive)

    std::function<void(int)> NonactiveReplyCB_; //!< Function callback for responding to non-active clients

    /**
     * \brief Run the select() loop to multiplex the listening socket
     **/
    void RunSelectLoop();
}; // end class SelectServerSocket

} // end namespace TCPB 

#endif
