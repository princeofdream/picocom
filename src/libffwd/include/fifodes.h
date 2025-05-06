#ifndef LIBFFWD_FIFO_H
#define LIBFFWD_FIFO_H

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdexcept>

class FifoDes {
public:
    FifoDes(const std::string& fifoName);
    ~FifoDes();

    int init();
    int create_fifo();
    int open_fifo(int mode);
    ssize_t read_fifo(std::string& buffer, size_t size);
    size_t write_fifo(const std::string &message);

private:
    std::string fifoPath;
    int fifoFd;
    void close_fifo();
};

#endif // LIBFFWD_FIFO_H