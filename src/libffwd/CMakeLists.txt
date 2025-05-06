cmake_minimum_required(VERSION 3.10)

project(libffwd VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add argparse library
include_directories(${CMAKE_SOURCE_DIR}/argparse)

set(ffwd_libs_srcs
        src/file_descriptor.cpp
        src/pipe.cpp
        src/socket_server.cpp
        src/socket_client.cpp
        src/epoll.cpp
        src/fifodes.cpp
        src/msgpak.cpp
)

# Source files
set(ffwd_srcs
        ${ffwd_libs_srcs}
        src/ffwd.cpp
)
set(ffwd_cli_srcs
        ${ffwd_libs_srcs}
        src/ffwd_cli.cpp
)

# Create the static library
add_library(libffwd_static STATIC ${ffwd_libs_srcs})
set_target_properties(libffwd_static PROPERTIES OUTPUT_NAME "ffwd")

# Create the shared library
add_library(libffwd_shared SHARED ${ffwd_libs_srcs})
set_target_properties(libffwd_shared PROPERTIES OUTPUT_NAME "ffwd")

# Set version information for the shared library
set_target_properties(libffwd_shared PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

# Create the executables
add_executable(ffwd ${ffwd_srcs})
add_executable(ffwd_cli ${ffwd_cli_srcs})

# Link libraries if needed (e.g., pthread for threading)
# target_link_libraries(libffwd pthread)