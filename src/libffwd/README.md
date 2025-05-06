# libffwd Project

libffwd is a C++ library designed to facilitate the handling of various input sources such as FIFO, file descriptors, pipes, and sockets (both server and client). The library utilizes epoll for efficient event handling and provides a structured way to manage input and output operations.

## Features

- **FIFO Support**: Create, read from, and write to FIFO (named pipes).
- **File Descriptor Management**: Open, close, and read from file descriptors.
- **Pipe Functionality**: Create and read from pipes for inter-process communication.
- **Socket Server and Client**: Implement socket communication for client-server architecture.
- **Epoll Integration**: Efficiently manage multiple input sources using epoll for event notification.
- **Message Packaging**: Define and handle message structures for communication.

## Directory Structure

```
libffwd
├── src
│   ├── fifo.cpp
│   ├── fifo.h
│   ├── file_descriptor.cpp
│   ├── file_descriptor.h
│   ├── pipe.cpp
│   ├── pipe.h
│   ├── socket_server.cpp
│   ├── socket_server.h
│   ├── socket_client.cpp
│   ├── socket_client.h
│   ├── epoll.cpp
│   ├── epoll.h
│   ├── msgpak.cpp
│   ├── msgpak.h
│   └── main.cpp
├── include
│   ├── fifo.h
│   ├── file_descriptor.h
│   ├── pipe.h
│   ├── socket_server.h
│   ├── socket_client.h
│   ├── epoll.h
│   └── msgpak.h
├── CMakeLists.txt
└── README.md
```

## Installation

To build the project, you need to have CMake installed. Follow these steps:

1. Clone the repository:
   ```
   git clone <repository-url>
   cd libffwd
   ```

2. Create a build directory:
   ```
   mkdir build
   cd build
   ```

3. Run CMake to configure the project:
   ```
   cmake ..
   ```

4. Build the project:
   ```
   make
   ```

## Usage

After building the project, you can use the library in your applications. Include the necessary headers and link against the compiled library.

### Example

Here is a simple example of how to use the library:

```cpp
#include "socket_server.h"
#include "msgpak.h"

// Your code to initialize and use the library
```

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any enhancements or bug fixes.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.