#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "http_util.hpp"
#include "proxy_server.hpp"

/// @brief For socket SO_REUSEADDR option
static const int reuseAddr = 1;

ProxyServer::ProxyServer(int port) : _port{port} {
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
}

void ProxyServer::start() {
    struct sockaddr addr;
    socklen_t len = 0;
    while (true) {
        // Accept client connection
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
        auto bytesRead = recv(fdClient, readBuffer, PROXY_READ_BUFFER_SIZE, 0);
        if (bytesRead == -1) {
            std::cout << "Error on recv(): " << std::strerror(errno) << "\n";
            continue;
        }

        // Get response
        std::string requestStr = readBuffer;
        auto response = requestResponse(requestStr);

        // Send response
        ssize_t n = send(fdClient, response.c_str(), response.size(), 0);
        if (n == -1) {
            std::cout << "Error on send(): " << std::strerror(errno) << "\n";
        }

        // Close client socket
        if (0 != close(fdClient)) {
            std::cout << "Error on close(): " << std::strerror(errno) << "\n";
        }
    }
}

std::string ProxyServer::requestResponse(const std::string &request) const {
    // Find first newline
    size_t idx = request.find('\n', 0);
    if (idx == -1UL) {
        return responseStatusLine(ResponseCode::BAD_REQUEST);
    }

    // Check if method and target are ok
    bool checkedMethod = false;
    size_t targetStartIdx = 0;
    size_t targetEndIdx = 0;
    for (size_t i = 0; i < idx; ++i) {
        // Search space characters
        if (request[i] != ' ') continue;

        if (!checkedMethod) {
            // Target starts on next char
            targetStartIdx = i + 1;

            // Checking valid methods
            checkedMethod = true;
            if (strncmp(request.c_str(), "GET", 3) == 0) continue;

            return responseStatusLine(ResponseCode::BAD_REQUEST);
        } else {
            targetEndIdx = i;
            break;
        }
    }

    // Check for errors
    if (!checkedMethod || targetStartIdx == 0 || targetEndIdx == 0) {
        return responseStatusLine(ResponseCode::BAD_REQUEST);
    }

    // Check for valid target
    auto targetPath = request.substr(targetStartIdx, targetEndIdx - targetStartIdx);

    // Remove possible leading slash
    if (targetPath[0] == '/') {
        targetPath = targetPath.substr(1);
    }

    // Remove leading http:// or https://
    if (targetPath.substr(0, 8) == "/http://") {
        targetPath = targetPath.substr(8);
    } else if (targetPath.substr(0, 9) == "/https://") {
        targetPath = targetPath.substr(9);
    }

    // If target is now empty, return not found
    if (targetPath.empty()) {
        return responseStatusLine(ResponseCode::NOT_FOUND);
    }

    // Connect to target address
    return connectToAddress(targetPath);
}

std::string ProxyServer::connectToAddress(const std::string &targetAddress) const {
    // Getting address info
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Check if address fails
    int code = getaddrinfo(targetAddress.c_str(), "80", &hints, &res);
    if (code < 0) {
        return responseStatusLine(ResponseCode::NOT_FOUND);
    }

    // Create socket for address connection
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Connect socket to address
    connect(sock, res->ai_addr, res->ai_addrlen);
    
    // Send HTTP request
    std::string header = "GET / HTTP/1.1\r\nHost: ";
    header += targetAddress;
    header += "\r\n\r\n";
    send(sock, header.c_str(), header.size(), 0);

    // Keep reading until finished
    char buffer[PROXY_READ_BUFFER_SIZE];
    ssize_t byteCount;
    std::string message;
    while (1) {
        byteCount = recv(sock, buffer, PROXY_READ_BUFFER_SIZE - 1, 0);
        buffer[byteCount] = 0;

        if (byteCount == -1) {
            return responseStatusLine(ResponseCode::INTERNAL_SERVER_ERROR);
        } else if (byteCount == 0) {
            break;
        }

        // Append new chunk to final message
        message += buffer;

        // Check if message ends with </html>
        int found = false;
        for (int i = 0; i < 50; ++i) {
            if (0 == strncmp(message.end().base() - 8 - i, "</html>", 7)) {
                found = true;
                break;
            }
        }
        if (found) break;
    }

    return message;
}
