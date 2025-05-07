/*
 * =====================================================================================
 *
 *       Filename:  ppcom_cli.cpp
 *
 *    Description: ppcom cli
 *
 *        Version:  1.0
 *        Created:  2021年09月17日 14时31分32秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  James Lee (JamesL), princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <iostream>
#include <string>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "fifodes.h"
#include "file_descriptor.h"
#include "pipe.h"
#include "dbglog.h"

std::atomic<bool> running(true); // 用于控制接收线程的运行状态

void print_usage() {
    qLogE("Usage: ffwd_cli -h <host> -p <port> -m <message>\n"
        "  -h <host>     Hostname or IP address of the server (default: 127.0.0.1)\n"
        "  -p <port>     Port of the server (default: 9800)\n"
        "  -m <message>  Message to send to the server");
}

int setup_socket(const std::string& host, int port) {
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
    return sockfd;
}

void receive_messages(int sockfd) {
    char buffer[1024];
    while (running) {
        memset(buffer, 0x0, sizeof(buffer));
        int bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            qLogE("Failed to receive response");
            break;
        } else if (bytes_received == 0) {
            qLogI("Server closed the connection");
            break;
        }
        // qLogI("Message received from server: %s", buffer);
        printf("%s", buffer);
    }
    running = false; // 如果接收线程退出，设置运行状态为 false
}

int send_message(int sockfd, const std::string& message) {
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

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 9800;
    std::string message;
    std::string fifo_path = "/tmp/ffwd_fifo";
    std::string comm_type = "sock";
    int msgfd = -1;

    int opt;
    while ((opt = getopt(argc, argv, "h:p:m:t:f:")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = std::stoi(optarg);
                break;
            case 'm':
                message = optarg;
                break;
            case 't':
                comm_type = optarg;
                break;
            case 'f':
                fifo_path = optarg;
                break;
            default:
                print_usage();
                return 1;
        }
    }

    // if (message.empty()) {
    //     print_usage();
    //     return 1;
    // }

    if (comm_type != "sock" && comm_type != "pipe" && comm_type != "fifo") {
        print_usage();
        return 1;
    }

    FifoDes mfifo(fifo_path);

    if (comm_type == "sock") {
        qLogI("Using socket communication");
        msgfd = setup_socket(host, port);
        if (msgfd < 0) {
            return msgfd;
        }
    } else if (comm_type == "pipe") {
        qLogI("Using pipe communication");
    } else if (comm_type == "fifo") {
        qLogI("Using fifo communication");
    }

     // 启动接收线程
    std::thread receiver_thread(receive_messages, msgfd);

    while (true) {
        if (!running) {
            break;
        }
        std::cin >> message;
        if (message == "exit") {
            break;
        }
        // qLogI("Sending message: %s", message.c_str());
        printf("%s", message.c_str());
        send_message(msgfd, message);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // 停止接收线程
    running = false;
    if (receiver_thread.joinable()) {
        receiver_thread.join();
    }

    close(msgfd);
    return 0;
}
