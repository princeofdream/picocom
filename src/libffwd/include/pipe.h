#ifndef LIBFFWD_PIPE_H
#define LIBFFWD_PIPE_H

#include <unistd.h>
#include <stdexcept>

class Pipe {
public:
    Pipe();
    virtual ~Pipe();

    ssize_t read();
    void write(const char* data, size_t size);

private:
    int pipe_fd[2];
};

#endif // LIBFFWD_PIPE_H