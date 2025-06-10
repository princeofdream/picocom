#ifndef LIBFFWD_EPOLL_H
#define LIBFFWD_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include <functional>

typedef struct epoll_st
{
    int index;           // Index for tracking the event
    uint32_t events;      // Epoll events
    int infd;             // File descriptor associated with the event
    int outfd;            // Output file descriptor (if applicable)
    void *data;           // User data associated with the event
} epoll_st_t;

class Epoll {
public:
    Epoll();
    virtual ~Epoll();

    void add(int fd, uint32_t events, std::function<void(int)> callback);
    void addSt(int fd, uint32_t events, std::function<void(int, epoll_st_t&)> callback);
    void remove(int fd);
    std::vector<int> wait(int timeout);
    std::unordered_map<int, std::function<void(int)>> getCallbacks() const {
        return callbacks;
    }
    std::unordered_map<int, std::function<void(int, epoll_st_t&)>> getCbs() const {
        return cbs;
    }
    int getEpollStVecSize() const {
        return vec_epoll_st.size();
    };
    std::vector<epoll_st_t> getEpollStVec() const {
        return vec_epoll_st;
    };
    int getEpollStByFd(std::vector<epoll_st_t> vec_epollSt, epoll_st_t& epollSt, int fd);
    int getEpollStByIndex(std::vector<epoll_st_t> vec_epollSt, epoll_st_t& epollSt, int index);

private :
    int epoll_fd;
    struct epoll_event event;
    std::unordered_map<int, std::function<void(int)>> callbacks;
    std::unordered_map<int, std::function<void(int, epoll_st_t&)>> cbs;
    std::vector<epoll_st_t> vec_epoll_st;
};

#endif // LIBFFWD_EPOLL_H
