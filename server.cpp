#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#include "server.hpp"

/// @brief For socket SO_REUSEADDR option
static const int reuseAddr = 1;

HttpServer::HttpServer(int port) : _port{port} {
    // Create server socket
    fdServer = socket(AF_INET, SOCK_STREAM, 0);
    if (fdServer == -1) {
        std::cout << "Error on socket(): " << std::strerror(errno) << "\n";
        exit(1);
    }

    // Set sock options
    if (0 != setsockopt(fdServer, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(int))) {
        std::cout << "Error on setsockopt(): " << std::strerror(errno) << "\n";
        exit(1);
    }

    // Define server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket
    if (0 != bind(fdServer, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) {
        std::cout << "Error on bind(): " << std::strerror(errno) << "\n";
        exit(1);
    }

    // Listen to requests
    if (0 != listen(fdServer, 5)) {
        std::cout << "Error on listen(): " << std::strerror(errno) << "\n";
        exit(1);
    }
    std::cout << "Server running\n";
}

void HttpServer::start() {
    while (true) {
        // Accept client connection
        struct sockaddr addr;
        socklen_t len;
        int fdClient = accept(fdServer, &addr, &len);

        if (fdClient == -1) {
            std::cout << "Error on accept(): " << std::strerror(errno) << "\n";
            continue;
        }

        // Get client IP address
        auto addr_in = (struct sockaddr_in*)&addr;
        char *ipClient = inet_ntoa(addr_in->sin_addr);
        std::cout << "IP address: " << ipClient << "\n";

        // Receive data from client
        *readBuffer = '\0';
        auto bytesRead = recv(fdClient, readBuffer, READ_BUFFER_SIZE, 0);
        if (bytesRead == -1) {
            std::cout << "Error on recv(): " << std::strerror(errno) << "\n";
            continue;
        }

        // Show data
        std::cout << "Data from client: " << readBuffer << "\n";

        // TODO: Check if message contains correctly formed request line
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages#http_requests
        //
        // Template:
        // [METHOD] [URL] HTTP/[VERSION]
        //
        // Examples:
        // POST / HTTP/1.1
        // GET /image.png HTTP/1.1
        // POST /person/2 HTTP/1.0



        // TODO: Formalize response status line
        //
        // Template:
        // HTTP/[VERSION] [STATUS_CODE] [STATUS_MESSAGE]
        //
        // Examples:
        // HTTP/1.1 200 OK
        // HTTP/1.1 404 Not Found
        // HTTP/1.0 201 Created


        // Send response (hardcoded for now)
        std::string response = "HTTP/1.1 200 OK\n\nHello from server!\n";

        printf("response: \"%s\"\n", response.c_str());

        ssize_t n = send(fdClient, response.c_str(), response.size(), 0);
        if (n == -1) {
            std::cout << "Error on send(): " << std::strerror(errno) << "\n";
        }

        std::cout << "Sent\n";

        // Close client socket
        if (0 != close(fdClient)) {
            std::cout << "Error on close(): " << std::strerror(errno) << "\n";
        }

        std::cout << "Closed\n";
    }
}