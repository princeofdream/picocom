#ifndef LIBFFWD_FILE_DESCRIPTOR_H
#define LIBFFWD_LIBFFWD_FILE_DESCRIPTOR_H

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>

class FileDescriptor {
public:
    FileDescriptor();
    virtual ~FileDescriptor();

    void open(const std::string& path, int flags);

    void close(int fd);

    ssize_t read(void* buffer, size_t count);

    int getFd() const {
        return fd;
    }

private:
    int fd;
};

#endif // LIBFFWD_FILE_DESCRIPTOR_H