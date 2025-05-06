#ifndef LIBFFWD_SOCKET_SERVER_H
#define LIBFFWD_SOCKET_SERVER_H

#include <string>
#include <vector>
#include <functional>
#include <netinet/in.h>

class SocketServer {
public:
    SocketServer(int port);
    ~SocketServer();

    bool start();
    void stop();
    void sendMessage(int clientId, const std::string& message);
    int getSocketServerFD() const {
        return server_fd;
    }
    int acceptClient();
    void handleClient(int client_fd);
    using MessageCallback = std::function<void(int clientId, const std::string &message)>;
    void registerMessageCallback(MessageCallback callback);

private:
    int serverSocket;
    int server_port;
    int server_fd;
    bool running;
    size_t MaxClients;
    MessageCallback messageCallback;
    std::vector<int> client_fds;

    void acceptClients();
    void handleClientMessage(int clientSocket);
};

#endif // LIBFFWD_SOCKET_SERVER_HSERVER_H