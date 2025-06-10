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

void Epoll::addSt(int fd, uint32_t events, std::function<void(int, epoll_st_t&)> callback)
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
    vec_epoll_st.push_back({fd, events, fd, -1, nullptr}); // Initialize epoll_st_
    cbs[fd] = callback;
    qLogD("Added callback for file descriptor %d", fd);
}

void Epoll::remove(int fd)
{
    try {
        qLogD("Removing file descriptor %d from epoll", fd);
        // Check if the file descriptor exists in the callbacks map
        if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
            qLogE("Failed to remove file descriptor %d from epoll", fd);
            throw std::runtime_error("Failed to remove file descriptor from epoll");
        }
    } catch (const std::exception& e) {
        qLogE("Exception while checking file descriptor %d: %s", fd, e.what());
    }
    // Remove the callback
    try {
        qLogD("Removing file descriptor %d from epoll", fd);
        callbacks.erase(fd);
        cbs.erase(fd);
    } catch (const std::out_of_range& e) {
        qLogE("Callback for file descriptor %d not found: %s", fd, e.what());
    }
    qLogD("Removed file descriptor %d from epoll and erased callbacks", fd);
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
            if (cbs.find(fd) != cbs.end())
            {
                int ret;
                // qLogI("EPOLLIN callback fd: %d", fd);
                epoll_st_t epollSt;
                ret = getEpollStByFd(vec_epoll_st, epollSt, fd);
                if (ret < 0) {
                    qLogE("Failed to find epoll_st for fd: %d", fd);
                    continue; // Skip if not found
                }
                cbs[fd](fd, epollSt);
            }
        }
        if (events[i].events & EPOLLOUT) {
            qLogI("EPOLLOUT");
        }
    }

    return active_fds;
}

int Epoll::getEpollStByFd(std::vector<epoll_st_t> vec_epollSt, epoll_st_t& epollSt, int fd)
{
    epollSt = { -1, 0, -1, -1, nullptr }; // Initialize epollSt
    for (size_t i = 0; i < vec_epollSt.size(); ++i) {
        if (vec_epollSt[i].infd == fd) {
            epollSt = vec_epollSt[i];
            return i; // Return the index of the found event
        }
    }

    return -1; // Event not found
}

int Epoll::getEpollStByIndex(std::vector<epoll_st_t> vec_epollSt, epoll_st_t& epollSt, int index)
{
    epollSt = { -1, 0, -1, -1, nullptr }; // Initialize epollSt

    for (size_t i = 0; i < vec_epollSt.size(); ++i) {
        if (vec_epollSt[i].index == index) {
            epollSt = vec_epollSt[i];
            return i; // Return the index of the found event
        }
    }

    return -1; // Event not found
}
