#include <iostream>
#include <string>
#include <unistd.h>
#include "fifodes.h"
#include "file_descriptor.h"
#include "pipe.h"
#include "socket_server.h"
#include "socket_client.h"
#include "epoll.h"
#include "msgpak.h"
#include "dbglog.h"

SocketServer sockServMsg;
SocketServer sockServCtl;

void print_usage() {
    std::cerr << "Usage: libffwd -t <connection_type> [-p <path>] [-P <port>]\n"
              << "  -t <connection_type>  Type of connection: pipe, fifo, socket\n"
              << "  -f <path>             Path for fifo or pipe (default: /tmp/myfifo)\n"
              << "  -p <port>             Port for socket server (default: 9800)\n";
}

void callbackMsg (int fd) {
    char buffer[1024];
    int loopCnt;
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        qLogI("Received message: %s", buffer);
    } else {
        qLogE("Failed to read message");
        close(fd);
    }

    if (sockServCtl.getSocketClientFD().size() > 0) {
        for (int i = 0; i < sockServCtl.getSocketClientFD().size(); i++) {
            int client_fd = sockServCtl.getSocketClientFD()[i];
            if (client_fd != -1) {
                write(client_fd, buffer, strlen(buffer));
            }
        }
    // } else {
    //     qLogE("No client connected to control socket");
    }
}

void callbackCtl (int fd) {
    char buffer[1024];
    int loopCnt;
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        qLogI("Received message: %s", buffer);
    } else {
        qLogE("Failed to read message");
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

int main(int argc, char* argv[]) {
    std::string connection_type = "socket";
    std::string fifo_path = "/tmp/ffwd_fifo";
    int port = 9800;

    int opt;
    while ((opt = getopt(argc, argv, "t:f:p:")) != -1) {
        switch (opt) {
            case 't':
                connection_type = optarg;
                break;
            case 'f':
                fifo_path = optarg;
                break;
            case 'p':
                port = std::stoi(optarg);
                break;
            default:
                print_usage();
                return 1;
        }
    }

    if (connection_type != "pipe" && connection_type != "fifo" && connection_type != "socket") {
        print_usage();
        return 1;
    }

    // Initialize components
    sockServMsg = SocketServer(port);
    sockServCtl = SocketServer(port+1);
    SocketClient client;
    Epoll mepoll;

    // Create and configure FIFODes, Pipe, and File Descriptors
    FifoDes mfifo(fifo_path);
    FileDescriptor mfd;
    Pipe mpipe;

    // Start the server
    sockServMsg.start();
    sockServCtl.start();

    // Example of adding server socket to epoll
    mepoll.add(sockServMsg.getSocketServerFD(), EPOLLIN, [&](int fd)
        {
            int client_msg_fd = sockServMsg.acceptClient();
            if (client_msg_fd != -1) {
                mepoll.add(client_msg_fd, EPOLLIN, callbackMsg);
            }
        }
    );
    mepoll.add(sockServCtl.getSocketServerFD(), EPOLLIN, [&](int fd)
        {
            int client_ctl_fd = sockServCtl.acceptClient();
            if (client_ctl_fd != -1) {
                mepoll.add(client_ctl_fd, EPOLLIN, callbackCtl);
            }
        }
    );


    // Main loop for handling events
    while (true) {
        std::vector<int> active_fds = mepoll.wait(3000); // Wait for events
        if (active_fds.size() == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
    }


    // Cleanup
    sockServMsg.stop();
    return 0;
}

