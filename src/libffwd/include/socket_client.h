#ifndef LIBFFWD_SOCKET_CLIENT_H
#define LIBFFWD_SOCKET_CLIENT_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class SocketClient {
public:
    SocketClient();
    ~SocketClient();

    bool connect(const std::string& server_ip, int server_port);
    void disconnect();
    bool sendMessage(const std::string& message);

private:
    int sockfd;
    struct sockaddr_in server_addr;
    bool is_connected;
};

#endif // LIBFFWD_SOCKET_CLIENT_H