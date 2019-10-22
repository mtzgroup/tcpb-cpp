 /** \file tcpbsocket.h
 *  \brief Definition of Socket class
 *
 */

#ifndef TCPB_SOCKET_H_
#define TCPB_SOCKET_H_

#include <sys/time.h> // for fd_set

#include <atomic>
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
     * @param sfd Socket file descriptor (default: -1, which creates new)
     * @param logName Logfile name (default: socket_<fd>.log)
     * @param cleanOnDestroy Whether to close the socket in destructor (default: true)
     **/
    Socket(int sfd = -1,
           const std::string& logName = "socket.log",
           bool cleanOnDestroy = true);

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
     * \brief Comparison between sockets
     *
     * @return True if underlying socket file descriptor is the same
     **/
    bool HasSameFD(const Socket& other) const { return (socket_ == other.socket_); }

    /**
     * \brief Check if connection is alive
     *
     * @return True if socket is active
     **/
     bool IsConnected() const { return (socket_ != -1); }

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
                    const char* log) const;

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
                    const char* log) const;

  protected:
    int socket_;          //!< Socket file descriptor
    FILE* logFile_;       //!< Logfile pointer
    bool cleanOnDestroy_; //!< Bool for closing socket in destructor

    /**
     * \brief A low-level socket recv wrapper to ensure full packet recv
     *
     * @param buf Buffer for incoming packet
     * @param len Byte size of incoming packet
     * @return nsent Number of bytes recv'd
     **/
    int RecvN(char* buf,
              int len) const;

    /**
     * \brief A low-level socket send wrapper to ensure full packet send
     *
     * @param buf Buffer for outgoing packet
     * @param len Byte size of outgoing packet
     * @return nsent Number of bytes sent
     **/
    int SendN(const char* buf,
              int len) const;

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
    void SocketLog(const char* format, ...) const;
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
 * This is an abstract class: You must inherit this class and implement the HandleClientMessage()
 * function, which is called in the select() loop, in order to use this.
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
    SelectServerSocket(int port);

    /**
     * \brief Destructor for SelectServerSocket class
     **/
    ~SelectServerSocket();

    // Rule of 5: Not moveable or copyable due to threading
    void swap(SelectServerSocket& other)                      = delete; // Helper swap function
    SelectServerSocket(SelectServerSocket&& move)             = delete; // Move constructor
    SelectServerSocket& operator=(SelectServerSocket&& move)  = delete; // Move operator

  protected:
    std::thread listenThread_;      //!< Thread for select() loop
    std::mutex listenMutex_;        //!< Mutex for select() loop thread
    fd_set activefds_;              //!< Set of active sockets in select() loop
    int maxfd_;                     //!< Largest file descriptor in activefds_
    std::atomic<bool> exitFlag_;    //!< Flag for exiting select() loop

    /**
     * \brief Run the select() loop to multiplex the listening socket
     **/
    void RunSelectLoop();


    /**
     * \brief Handle processing and replying to clients in the select() loop
     *
     * This pure virtual function must be implemented in a derived class
     *
     * @param client Socket object for incoming message
     **/
    virtual bool HandleClientMessage(const Socket& client) = 0;
}; // end class SelectServerSocket

} // end namespace TCPB 

#endif
