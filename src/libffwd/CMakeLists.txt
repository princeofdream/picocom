cmake_minimum_required(VERSION 3.10)

project(ffwd VERSION 1.0.0)

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

set(FFWD_LIBS -pthread)
if (${CMAKE_SYSTEM_NAME} MATCHES "QNX")
    list(APPEND FFWD_LIBS socket)
endif()

# Create the static library
add_library(${PROJECT_NAME}-static STATIC ${ffwd_libs_srcs})
# set_target_properties(${PROJECT_NAME}-static PROPERTIES VERSION ${VERSION})
set_target_properties(${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Create the shared library
add_library(${PROJECT_NAME}-shared SHARED ${ffwd_libs_srcs})
# set_target_properties(${PROJECT_NAME}-shared PROPERTIES VERSION ${VERSION})
set_target_properties(${PROJECT_NAME}-shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}-shared ${FFWD_LIBS})

# Set version information for the shared library
set_target_properties(ffwd-shared PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

# Create the executables
add_executable(${PROJECT_NAME}_srv ${ffwd_srcs})
add_executable(${PROJECT_NAME}_cli ${ffwd_cli_srcs})

# Link libraries if needed (e.g., pthread for threading)
# target_link_libraries(libffwd pthread)

install(TARGETS ${PROJECT_NAME}-shared
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        MANIFEST DESTINATION ${CMAKE_INSTALL_PREFIX}/share
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
        INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS ${PROJECT_NAME}-static
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        MANIFEST DESTINATION ${CMAKE_INSTALL_PREFIX}/share
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
        INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)


