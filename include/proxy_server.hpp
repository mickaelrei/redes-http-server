#pragma once

#include <netinet/in.h>
#include <ostream>

#define PROXY_READ_BUFFER_SIZE 2048
#define PROTOCOL_VERSION "HTTP/1.1"

/// @brief HTTP proxy server
class ProxyServer {
public:
    /// @brief Constructor
    /// @param port server port, defaults to 8080
    ProxyServer(int port = 8080);

    /// @brief Start listening to requests
    void start();
    
private:
    /// @brief Returns the response string for a given request string
    /// @param request request string
    /// @return response string
    std::string requestResponse(const std::string &request) const;

    /// @brief Attempts to connect to a given address
    /// @param targetAddress target address
    /// @return response string
    std::string connectToAddress(const std::string &targetAddress) const;

    /// @brief Server port
    int _port;

    /// @brief Server socket file descriptor
    int fdServer;

    /// @brief Server address info
    sockaddr_in serverAddress;

    /// @brief Buffer for connection data reading (buffer size + 1 for \0 at end)
    char readBuffer[PROXY_READ_BUFFER_SIZE + 1];
};