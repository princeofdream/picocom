#ifndef LIBFFWD_EPOLL_H
#define LIBFFWD_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include <functional>

class Epoll {
public:
    Epoll();
    virtual ~Epoll();

    void add(int fd, uint32_t events, std::function<void(int)> callback);
    void remove(int fd);
    std::vector<int> wait(int timeout);
    std::unordered_map<int, std::function<void(int)>> getCallbacks() const {
        return callbacks;
    }

    private : int epoll_fd;
    struct epoll_event event;
    // std::vector<std::function<void(int)>> callbacks;
    std::unordered_map<int, std::function<void(int)>> callbacks;
};

#endif // LIBFFWD_EPOLL_H