#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <string>
#include <iostream>

#define BUFFER_SIZE 2048

int main(void) {
    int on = 1;
    struct addrinfo hints, *res;
    int sockfd;
    
    char buf[BUFFER_SIZE];
    
    // get host info, make socket and connect it
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int r = getaddrinfo("www.google.com", "80", &hints, &res);
    if (r < 0) {
        printf("error on getaddrinf(): %s\n", gai_strerror(r));
        exit(1);
    }

    /*************************************************************/
    /* Create an AF_INET6 stream socket to receive incoming      */
    /* connections on                                            */
    /*************************************************************/
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket() failed");
        exit(1);
    }

    connect(sockfd, res->ai_addr, res->ai_addrlen);
    
    std::string header = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
    send(sockfd, header.c_str(), header.size(), 0);

    ssize_t byte_count;
    ssize_t total_count = 0;
    std::string message;
    while (1) {
        *buf = 0;
        byte_count = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
        buf[byte_count] = 0;

        if (byte_count == -1) {
            std::cout << "ERROR\n";
            break;
        } else if (byte_count == 0) {
            std::cout << "finished\n";
            break;
        }

        // Append new chunk to final message
        message += buf;
        printf("%.*s\n", 2500, message.c_str());

        // Check if message ends with </html>
        int found = false;
        for (int i = 0; i < 50; ++i) {
            if (byte_count >= 7 && strncmp(buf + byte_count - 8 - i, "</html>", 7) == 0) {
                found = true;
                break;
            }
        }
        if (found) break;
    }

    std::cout << message << "\n";

    return 0;
}