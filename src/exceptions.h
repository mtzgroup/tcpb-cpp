/** \file exceptions.h
 *  \brief Definition of exceptions classes
 */

#ifndef TCPB_EXCEPTIONS_H_
#define TCPB_EXCEPTIONS_H_

#include <stdexcept>

namespace TCPB {

/**
 * \brief Exception class for errors communicating to the TeraChem Protocol Buffer (TCPB) server
 *
 * Primarily designed to be called in TCPB::Client, which has socket/job information
 **/
class ServerCommError : public std::runtime_error {
  public:
    /**
     * \brief Constructor for ServerCommError
     *
     * Constructs a more helpful message based on current job
     *
     * @param msg Base message for exception
     * @param host Server hostname
     * @param port Server port number
     * @param jobDir Current job directory from server
     * @param jobId Current job id number from server
     **/
    ServerCommError(std::string msg,
                    std::string host,
                    int port,
                    std::string jobDir,
                    int jobId);

    const char* what() const throw() { return msg_.c_str(); }
  
  private:
    std::string msg_;
}; // end class ServerCommError 

} // end namespace TCPB
#endif
