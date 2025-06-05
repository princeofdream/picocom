#include <iostream>
#include <string>
#include <unistd.h>
#include "ffwd.h"

ffwd::ffwd(): srvPort(9800), msgfd(-1), cliRunning(false)
{
    sockServMsg = SocketServer(srvPort);
    sockServCtl = SocketServer(srvPort+1);
}

ffwd::~ffwd()
{}

void ffwd::ppcom_callbackMsg (int fd) {
    char buffer[1024];
    int loopCnt;
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        // qLogI("Received message(%d): %s", bytes_read, buffer);
    } else {
        qLogE("Failed to read fd[%d] message", fd);
        sockServCtl.removeClient(fd);
        close(fd);
    }

    if (sockServCtl.getSocketClientFD().size() > 0) {
        for (int i = 0; i < sockServCtl.getSocketClientFD().size(); i++) {
            int client_fd = sockServCtl.getSocketClientFD()[i];
            if (client_fd != -1) {
                write(client_fd, buffer, strlen(buffer));
                // printf("\r\nSent message to client %d: %s\r\n", client_fd, buffer);
            }
        }
    // } else {
    //     qLogE("No client connected to control socket");
    }
}

void ffwd::ppcom_callbackCtl (int fd) {
    char buffer[1024];
    int loopCnt;
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        qLogI("Received message(%d): %s", bytes_read, buffer);
    } else {
        qLogE("Failed to read fd[%d] message", fd);
        sockServCtl.removeClient(fd);
        close(fd);
    }

    if (sockServMsg.getSocketClientFD().size() > 0) {
        for (int i = 0; i < sockServMsg.getSocketClientFD().size(); i++) {
            int client_fd = sockServMsg.getSocketClientFD()[i];
            if (client_fd != -1) {
                write(client_fd, buffer, strlen(buffer));
            }
        }
    // } else {
    //     qLogE("No client connected to control socket");
    }

}

int ffwd::startSrv() {
    std::string connection_type = "socket";

    // Start the server
    sockServMsg.start();
    sockServCtl.start();

    // Example of adding server socket to epoll
    mepoll.add(sockServMsg.getSocketServerFD(), EPOLLIN, [&](int fd)
        {
            int client_msg_fd = sockServMsg.acceptClient();
            if (client_msg_fd != -1) {
                mepoll.add(client_msg_fd, EPOLLIN, std::bind(&ffwd::ppcom_callbackMsg, this, client_msg_fd));
            }
        }
    );
    mepoll.add(sockServCtl.getSocketServerFD(), EPOLLIN, [&](int fd)
        {
            int client_ctl_fd = sockServCtl.acceptClient();
            if (client_ctl_fd != -1) {
                mepoll.add(client_ctl_fd, EPOLLIN, std::bind(&ffwd::ppcom_callbackCtl, this, client_ctl_fd));
            }
        }
    );

    srvThrd = std::thread([&]() {
        // Main loop for handling events
        while (true) {
            std::vector<int> active_fds = mepoll.wait(3000); // Wait for events
            if (active_fds.size() == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }
        }
    });
    srvThrd.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return 0;
}

int ffwd::stopSrv() {
    // Cleanup
    sockServCtl.stop();
    sockServMsg.stop();
    return 0;
}

int ffwd::init() {
    // for server
    startSrv();

    // for cli
    msgfd = setup_socket("127.0.0.1", srvPort);
    if (msgfd < 0) {
        return msgfd;
    }
    // std::thread receiver_thread(std::bind(&ffwd::receive_messages, this, msgfd),msgfd);
    return 0;
}

int ffwd::exit() {
    // for server
    stopSrv();

    // for client
    cliRunning = false;
    // if (receiver_thread.joinable()) {
    //     receiver_thread.join();
    // }

    close(msgfd);
    return 0;
}

int ffwd::setup_socket(const std::string& host, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        qLogE("Failed to create socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        qLogE("Invalid address/ Address not supported");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        qLogE("Connection failed");
        return 1;
    }

    cliRunning = true;
    return sockfd;
}

void ffwd::receive_messages(int sockfd) {
    char buffer[1024];
    while (cliRunning) {
        memset(buffer, 0x0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            qLogE("Failed to receive response");
            break;
        } else if (bytes_received == 0) {
            qLogI("Server closed the connection");
            break;
        }
        qLogI("Message received from server: %s", buffer);
    }
    cliRunning = false; // 如果接收线程退出，设置运行状态为 false
}

int ffwd::send_message(int sockfd, const std::string& message) {
    char buffer[1024];
    int bytes_sent;

    if (message.empty()) {
        qLogE("Message is empty");
        return 1;
    }

    bytes_sent = send(sockfd, message.c_str(), message.size(), 0);
    if (bytes_sent < 0) {
        qLogE("Failed to send message");
        return bytes_sent;
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int ffwd::getMsgFD() {
    return msgfd;
}


