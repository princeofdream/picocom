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

void print_usage() {
    std::cerr << "Usage: libffwd -t <connection_type> [-p <path>] [-P <port>]\n"
              << "  -t <connection_type>  Type of connection: pipe, fifo, socket\n"
              << "  -f <path>             Path for fifo or pipe (default: /tmp/myfifo)\n"
              << "  -p <port>             Port for socket server (default: 9800)\n";
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
    SocketServer sock_server(port);
    SocketClient client;
    Epoll mepoll;

    // Create and configure FIFODes, Pipe, and File Descriptors
    FifoDes mfifo(fifo_path);
    FileDescriptor mfd;
    Pipe mpipe;

    // Start the server
    sock_server.start();

    std::function<void(int)> callback = [&](int fd) {
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

        loopCnt =  0;
        while (loopCnt < 10) {
            memset(buffer, 0x0, sizeof(buffer));
            sprintf(buffer, "%s-%02d", "Ack", loopCnt);
            write(fd, buffer, bytes_read);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            loopCnt++;
        }
    };

    // Example of adding server socket to epoll
    mepoll.add(sock_server.getSocketServerFD(), EPOLLIN, [&](int fd)
        {
            int client_fd = sock_server.acceptClient();
            if (client_fd != -1) {
                mepoll.add(client_fd, EPOLLIN, callback);
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
    sock_server.stop();
    return 0;
}