#include "epoll.h"
#include "dbglog.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <vector>
#include <iostream>

Epoll::Epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor");
    }
}

Epoll::~Epoll() {
    close(epoll_fd);
}

void Epoll::add(int fd, uint32_t events, std::function<void(int)> callback)
{
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;

    qLogD("Adding file descriptor %d to epoll", fd);
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        throw std::runtime_error("Failed to add file descriptor to epoll");
    }

    qLogD("Adding callback for file descriptor %d", fd);
    // Store the callback for later use
    callbacks[fd] = callback;
    qLogD("Added callback for file descriptor %d", fd);
}

void Epoll::remove(int fd)
{
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1)
    {
        throw std::runtime_error("Failed to remove file descriptor from epoll");
    }
    // Remove the callback
    callbacks.erase(fd);
}

std::vector<int> Epoll::wait(int timeout)
{
    std::vector<int> active_fds;
    const int max_events = 1024;
    struct epoll_event events[max_events];

    int num_events = epoll_wait(epoll_fd, events, max_events, timeout);
    if (num_events == -1)
    {
        throw std::runtime_error("Failed to wait for epoll events");
        active_fds.erase(active_fds.begin(), active_fds.end());
        return active_fds;
    }

    for (int i = 0; i < num_events; ++i)
    {
        if (events[i].events & EPOLLIN) {
            int fd = events[i].data.fd;
            active_fds.push_back(fd);
            // Call the stored callback
            if (callbacks.find(fd) != callbacks.end())
            {
                // qLogI("EPOLLIN callback fd: %d", fd);
                callbacks[fd](fd);
            }
        }
        if (events[i].events & EPOLLOUT) {
            qLogI("EPOLLOUT");
        }
    }

    return active_fds;
}
