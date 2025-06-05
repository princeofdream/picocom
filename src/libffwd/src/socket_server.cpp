#include "socket_server.h"
#include "dbglog.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

SocketServer::SocketServer() : server_port(-1), server_fd(-1), MaxClients(10)
{
}

SocketServer::SocketServer(int port) : server_port(port), server_fd(-1), MaxClients(10)
{
}

SocketServer::~SocketServer() {
    stop();
}

void SocketServer::setPort(int port) {
    server_port = port;
}

bool SocketServer::start() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return false;
    }

    if (listen(server_fd, MaxClients) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return false;
    }

    qLogI("Server started on port: %d", server_port);
    return true;
}

void SocketServer::stop() {
    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
        std::cout << "Server stopped" << std::endl;
    }
}

int SocketServer::acceptClient()
{
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return -1;
    }
    client_fds.push_back(client_fd);
    qLogI("Accepted client with fd %d", client_fd);
    return client_fd;
}

void SocketServer::sendMessage(int client_fd, const std::string& message) {
    send(client_fd, message.c_str(), message.size(), 0);
}

void SocketServer::handleClient(int client_fd) {
    char buffer[1024];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        qLogI("Received message: %s", buffer);
        // Process the message and send a response if needed
    }
    close(client_fd);
}

int SocketServer::removeClient(int client_fd)
{
    for (auto it = client_fds.begin(); it != client_fds.end(); ++it) {
        if (*it == client_fd) {
            close(client_fd);
            client_fds.erase(it);
            qLogI("Removed client with fd %d", client_fd);
            return 0;
        }
    }
}