/** \file tcpbsocket.h
 *  \brief Definition of TCPBSocket class
 */

#ifndef TCPBSOCKET_H_
#define TCPBSOCKET_H_

#include <string>

/**
 * \brief TeraChem Protocol Buffer (TCPB) Socket class
 *
 * Helper class to manage a socket for TCPBSocket
 **/
class TCPBSocket {
  public:
    //Constructor/Destructor
    /**
     * \brief Constructor for TCPBSocket class
     *
     * Sets up the logfile if the #SOCKETLOGS macro is defined.
     *
     * @param host C string of hostname with TCPB server
     * @param port Integer port of TCPB server
     **/
    TCPBSocket(std::string host,
               int port);

    /**
     * \brief Destructor for TCPBSocket
     *
     * The destructor also handles disconnect and logfile cleanup.
     **/
    ~TCPBSocket();

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
};

#endif
