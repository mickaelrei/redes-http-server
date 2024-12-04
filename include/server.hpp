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
    /// @brief Returns the response string for a given request string
    /// @param request request string
    /// @return response string
    std::string requestResponse(const std::string &request) const;

    /// @brief Attempts to get a file
    /// @param filePath file path
    /// @param exists output boolean telling whether file exists or not
    /// @return string content of the file
    std::string getFile(const std::string &filePath, bool &exists) const;

    /// @brief Server port
    int _port;

    /// @brief Server socket file descriptor
    int fdServer;

    /// @brief Server address info
    sockaddr_in serverAddress;

    /// @brief Buffer for connection data reading (buffer size + 1 for \0 at end)
    char readBuffer[READ_BUFFER_SIZE + 1];
};