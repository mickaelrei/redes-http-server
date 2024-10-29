#pragma once

#include <netinet/in.h>
#include <ostream>

#define READ_BUFFER_SIZE 1024

/// @brief HTTP server
class HttpServer {
public:
    /// @brief Constructor
    /// @param port server port, defaults to 8080
    HttpServer(int port = 8080);

    /// @brief Start listening to requests
    void start();
    
private:
    /// @brief Server port
    int _port;

    /// @brief Server socket file descriptor
    int fdServer;

    /// @brief Server address info
    sockaddr_in serverAddress;

    /// @brief Buffer for connection data reading (buffer size + 1 for \0 at end)
    char readBuffer[READ_BUFFER_SIZE + 1];
};