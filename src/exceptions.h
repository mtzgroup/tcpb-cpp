/** \file tcpboutput.h
 *  \brief Definition of TCPBExceptions class
 */

#ifndef TCPBEXCEPTIONS_H_
#define TCPBEXCEPTIONS_H_

#include <stdexcept>

/**
 * \brief Exception class for errors communicating to the TeraChem Protocol Buffer (TCPB) server
 *
 * Primarily designed to be called in TCPBClient, which has socket/job information
 **/
class ServerError : public std::runtime_error {
  public:
    /**
     * \brief Constructor for ServerError
     *
     * Constructs a more helpful message based on current job
     *
     * @param msg Base message for exception
     * @param host Server hostname
     * @param port Server port number
     * @param jobDir Current job directory from server
     * @param jobId Current job id number from server
     **/
    ServerError(std::string msg,
                std::string host,
                int port,
                std::string jobDir,
                int jobId);

    const char* what() const throw() { return msg_.c_str(); }
  
  private:
    std::string msg_;
};

#endif
