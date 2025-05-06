#include "pipe.h"
#include <unistd.h>
#include <stdexcept>

Pipe::Pipe() {
    if (pipe(pipe_fd) == -1) {
        throw std::runtime_error("Failed to create pipe");
    }
}

Pipe::~Pipe() {
    close(pipe_fd[0]);
    close(pipe_fd[1]);
}

ssize_t Pipe::read() {
    char buffer[1024];
    ssize_t bytesRead = ::read(pipe_fd[0], buffer, sizeof(buffer));
    if (bytesRead < 0) {
        throw std::runtime_error("Failed to read from pipe");
    }
    return bytesRead;
}

void Pipe::write(const char* data, size_t length) {
    ssize_t bytesWritten = ::write(pipe_fd[1], data, length);
    if (bytesWritten < 0) {
        throw std::runtime_error("Failed to write to pipe");
    }
}