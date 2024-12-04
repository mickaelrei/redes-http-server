#include <iostream>

#include "server.hpp"
#include "proxy_server.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Running HTTP server\n";
        HttpServer server;
        server.start();
        return 0;
    }

    std::cout << "Running proxy server\n";
    ProxyServer server;
    server.start();
}