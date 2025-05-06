#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdexcept>
#include "fifodes.h"
#include "dbglog.h"

FifoDes::FifoDes(const std::string& fifoPath) : fifoPath("/tmp/ffwd_fifo"), fifoFd(-1) {
}

FifoDes::~FifoDes() {
    close_fifo();
}

int FifoDes::init() {
    create_fifo();

    fifoFd = open_fifo(O_RDWR | O_NONBLOCK);
    if (fifoFd < 0) {
        qLogE("Failed to open FIFO for reading and writing");
        throw std::runtime_error("Failed to open FifoDes");
        return -1;
    }
    return 0;
}

int FifoDes::create_fifo() {
    unlink(fifoPath.c_str());
    if (mkfifo(fifoPath.c_str(), 0666) == -1) {
        qLogE("Failed to create FIFO [%s]", fifoPath.c_str());
        throw std::runtime_error("Failed to create FifoDes");
        return -1;
    }
    return 0;
}

int FifoDes::open_fifo(int mode) {
    fifoFd = open(fifoPath.c_str(), mode);
    if (fifoFd < 0) {
        perror("Failed to open FIFO");
        return false;
    }
    return true;
}

size_t FifoDes::write_fifo(const std::string& message) {
    int fd;
    int wrSize;

    if (fifoFd < 0) {
        throw std::runtime_error("Failed to open FifoDes for writing");
    }
    wrSize = write(fd, message.c_str(), message.size());
    return wrSize;
}

ssize_t FifoDes::read_fifo(std::string& buffer, size_t size) {
    if (fifoFd < 0) {
        qLogE("FIFO not opened for reading");
        return -1;
    }
    char temp[size];
    ssize_t bytes_read = read(fifoFd, temp, size);
    if (bytes_read > 0) {
        buffer.assign(temp, bytes_read);
    }

    return bytes_read;
}

void FifoDes::close_fifo() {
    if (fifoFd > 0) {
        close(fifoFd);
    }
}