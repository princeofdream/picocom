#include <iostream>
#include <string>
#include <unistd.h>
#include "ffwd.h"

ffwd::ffwd(): srvPort(9800), msgfd(-1), cliRunning(false)
{
    // Initialize the socket servers
    sockServMsg = SocketServer(srvPort);
    sockServCtl = SocketServer(srvPort+1);
}

ffwd::~ffwd()
{}

void ffwd::ppcom_EpollCallbackMsg (int fd, epoll_st_t &epollSt) {
    char buffer[1024];
    int loopCnt;
    int bytes_read = read(fd, buffer, sizeof(buffer));

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        // qLogI("Received message(%d): %s", bytes_read, buffer);
    } else {
        qLogE("Failed to read fd[%d] message", fd);
        sockServMsg.removeClient(fd);
        close(fd);
        static_cast<ffwd*>(epollSt.data)->getEpoll().remove(fd);
    }

    // Send the message to all connected clients
    if (sockServMsg.getSocketClientFD().size() > 0) {
        for (int i = 0; i < sockServMsg.getSocketClientFD().size(); i++) {
            int client_fd = sockServMsg.getSocketClientFD()[i];
            // qLogI("[%d] Current fd [%d], Skipping message fd [%d, %d]", fd, client_fd, msgfd, ctlfd);
            if (client_fd == msgfd || client_fd == fd) {
                // qLogI("Current fd [%d], Skipping message fd [%d, %d]", client_fd, msgfd, ctlfd);
                continue; // Skip invalid client file descriptors
            }
            if (client_fd > 0) {
                write(client_fd, buffer, strlen(buffer));
                qLogI("[%d] Sent message to client fd %d: %s", fd, client_fd, buffer);
            }
        }
    }
}

void ffwd::ppcom_EpollCallbackCtl (int fd, epoll_st_t &epollSt)
{
    char buffer[1024];
    int loopCnt;
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        qLogI("[%d] Received message(%d): %s", fd, bytes_read, buffer);
    } else {
        qLogE("Failed to read fd[%d] message", fd);
        sockServCtl.removeClient(fd);
        close(fd);
        static_cast<ffwd*>(epollSt.data)->getEpoll().remove(fd);
    }

    // Send the message to all server
    if (sockServCtl.getSocketClientFD().size() > 0) {
        for (int i = 0; i < sockServCtl.getSocketClientFD().size(); i++) {
            int client_fd = sockServCtl.getSocketClientFD()[i];
            if (client_fd == ctlfd || client_fd == fd) {
                // qLogI("[%d] Current fd [%d], Skipping message fd [%d, %d]", fd, client_fd, msgfd, ctlfd);
                continue; // Skip invalid client file descriptors
            }
            if (client_fd > 0) {
                write(client_fd, buffer, strlen(buffer));
                qLogI("[%d] Sent message to client fd %d: %s", fd, client_fd, buffer);
            }
        }
    }

    return;
}

int ffwd::startSrv() {
    std::string connection_type = "socket";

    // Start the server
    sockServMsg.start();
    sockServCtl.start();

    mepoll.addSt(sockServMsg.getSocketServerFD(), EPOLLIN, [&](int fd, epoll_st_t &epollSt)
        {
            int client_msg_fd = sockServMsg.acceptClient();
            epollSt.data = this; // Store 'this' pointer for callback
            qLogI("[%d] Accepted client connection on fd: %d", fd, client_msg_fd);
            if (client_msg_fd != -1) {
                // qLogI("Adding callback for client_msg_fd: %d", client_msg_fd);
                mepoll.addSt(client_msg_fd, EPOLLIN, std::bind(&ffwd::ppcom_EpollCallbackMsg, this, client_msg_fd, epollSt));
            }
        }
    );
    mepoll.addSt(sockServCtl.getSocketServerFD(), EPOLLIN, [&](int fd, epoll_st_t &epollSt)
        {
            int client_ctl_fd = sockServCtl.acceptClient();
            epollSt.outfd = fd;
            epollSt.data = this; // Store 'this' pointer for callback
            qLogI("[%d] Accepted control connection on fd: %d", fd, client_ctl_fd);
            if (client_ctl_fd != -1) {
                // qLogI("Adding callback for client_ctl_fd: %d", client_ctl_fd);
                mepoll.addSt(client_ctl_fd, EPOLLIN, std::bind(&ffwd::ppcom_EpollCallbackCtl, this, client_ctl_fd, epollSt));
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
    qLogI("get message from tty msgfd: %d", msgfd);
    if (msgfd < 0) {
        return msgfd;
    }
    ctlfd = setup_socket("127.0.0.1", srvPort+1);
    qLogI("set control to tty with ctlfd: %d", ctlfd);
    if (ctlfd < 0) {
        return ctlfd;
    }
    // std::thread receiver_thread(std::bind(&ffwd::receive_messages, this, msgfd),msgfd);
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
    close(ctlfd);
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



