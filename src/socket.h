 /** \file tcpbsocket.h
 *  \brief Definition of Socket class
 */

#ifndef TCPB_SOCKET_H_
#define TCPB_SOCKET_H_

#include <string>

namespace TCPB {

/**
 * \brief TeraChem Protocol Buffer (TCPB) Socket class
 *
 * Helper class to manage a socket for Socket
 **/
class Socket {
  public:
    //Constructor/Destructor
    /**
     * \brief Constructor for Socket class
     *
     * Sets up the logfile if the #SOCKETLOGS macro is defined.
     *
     * @param host C string of hostname with TCPB server
     * @param port Integer port of TCPB server
     **/
    Socket(std::string host,
           int port);

    /**
     * \brief Destructor for Socket
     *
     * The destructor also handles disconnect and logfile cleanup.
     **/
    ~Socket();

    // Rule of 5: Moveable but not copyable
    // Pattern taken from https://codereview.stackexchange.com/q/131137
    void swap(Socket& other)          noexcept;
    Socket(Socket&& move)             noexcept;
    Socket& operator=(Socket&& move)  noexcept;
    Socket(Socket const&)             = delete;
    Socket& operator=(Socket const&)  = delete;

    /**
     * \brief Connect function for client socket
     **/
     void Connect();

    /**
     * \brief Disconnect function for client socket
     **/
     void Disconnect();

    /**
     * \brief Check if connection is alive
     *
     * @return True if socket is active
     **/
     bool IsConnected() { return (server_ != -1); }

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

  private:
    std::string host_;
    int port_;
    int server_;
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

} // end namespace TCPB 

#endif
