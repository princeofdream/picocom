#include "socket_client.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

SocketClient::SocketClient() : sockfd(-1) {}

SocketClient::~SocketClient() {
    disconnect();
}

bool SocketClient::connect(const std::string& server_ip, int server_port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return false;
    }

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    if (::connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection to server failed" << std::endl;
        close(sockfd);
        sockfd = -1;
        return false;
    }

    return true;
}

void SocketClient::disconnect() {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

bool SocketClient::sendMessage(const std::string& message) {
    if (sockfd == -1) {
        std::cerr << "Not connected to any server" << std::endl;
        return false;
    }

    ssize_t bytes_sent = send(sockfd, message.c_str(), message.size(), 0);
    if (bytes_sent < 0) {
        std::cerr << "Failed to send message" << std::endl;
        return false;
    }

    return true;
}