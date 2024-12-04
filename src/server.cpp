#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#include "http_util.hpp"
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
}

void HttpServer::start() {
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
        auto bytesRead = recv(fdClient, readBuffer, READ_BUFFER_SIZE, 0);
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

std::string HttpServer::requestResponse(const std::string &request) const {
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

    // Get file content
    auto targetPath = request.substr(targetStartIdx, targetEndIdx - targetStartIdx);
    if (targetPath.empty() || targetPath == "/") {
        targetPath = "/index.html";
    }

    bool exists;
    auto fileContent = getFile(targetPath, exists);

    if (!exists) {
        return responseStatusLine(ResponseCode::NOT_FOUND);
    }

    // Build response text
    std::stringstream responseStream{""};
    responseStream << responseStatusLine(ResponseCode::OK);
    responseStream << "Content-Length: " << fileContent.size();
    responseStream << "\n\n" << fileContent;

    return responseStream.str();
}

std::string HttpServer::getFile(const std::string &filePath, bool &exists) const {
    const std::string contentFolder = "./content/";
    // Attempt to open file
    std::ifstream file{contentFolder + filePath};
    if (!file.is_open()) {
        exists = false;
        return "";
    }

    exists = true;
    // Keep reading content
    std::string content;
    std::string line;
    while (getline(file, line)) {
        content += line;
        content += "\n";
    }

    // Close
    file.close();

    return content;
}
