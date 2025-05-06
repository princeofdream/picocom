#include "file_descriptor.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

FileDescriptor::FileDescriptor() : fd(-1) {}

FileDescriptor::~FileDescriptor() {
    if (fd != -1) {
        close(fd);
    }
}

void FileDescriptor::open(const std::string& path, int flags) {
    fd = ::open(path.c_str(), flags);
    if (fd == -1) {
        throw std::runtime_error("Failed to open file descriptor");
    }
}

void FileDescriptor::close(int fd) {
    if (fd != -1) {
        if (::close(fd) == -1) {
            throw std::runtime_error("Failed to close file descriptor");
        }
        fd = -1;
    }
}

ssize_t FileDescriptor::read(void* buffer, size_t size) {
    if (fd == -1) {
        throw std::runtime_error("File descriptor is not open");
    }
    return ::read(fd, buffer, size);
}