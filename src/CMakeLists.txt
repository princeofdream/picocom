
project(ppcom)

add_definitions("-std=c++14")
link_directories(
    ${CMAKE_BINARY_DIR}/libprocmgr
    ./
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/../libprocmgr/include
)

# aux_source_directory(./src SOURCES)
set(SOURCES fdio.c)
list(APPEND SOURCES term.c)
list(APPEND SOURCES termios2.c)
list(APPEND SOURCES linenoise.c)
# list(APPEND SOURCES ctrl_term.c)
list(APPEND SOURCES split.c)
list(APPEND SOURCES custbaud.c)
list(APPEND SOURCES custbaud_bsd.c)

list(APPEND SOURCES_SRV ppcom.c)
# list(APPEND SOURCES_CLI ppcom_cli.c)

add_executable(${PROJECT_NAME} ${SOURCES} ${SOURCES_SRV})


set(PPCOM_LIBS -pthread)
# list(APPEND PPCOM_LIBS procmgr)
if (${CMAKE_SYSTEM_NAME} MATCHES "QNX")
    list(APPEND PPCOM_LIBS socket)
endif()

target_link_libraries(${PROJECT_NAME} ${PPCOM_LIBS} -Wl,-rpath=./)
add_dependencies(${PROJECT_NAME} procmgr-shared)

# add_executable(${PROJECT_NAME}-cli ${SOURCES} ${SOURCES_CLI})
# target_link_libraries(${PROJECT_NAME}-cli ${PPCOM_LIBS} -Wl,-rpath=./)
# add_dependencies(${PROJECT_NAME}-cli procmgr-shared)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
        MANIFEST DESTINATION share
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# install(TARGETS ${PROJECT_NAME}-cli
#         RUNTIME DESTINATION bin
#         LIBRARY DESTINATION lib
#         ARCHIVE DESTINATION lib
#         INCLUDES DESTINATION include
#         MANIFEST DESTINATION share
#         RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#         LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
#         ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
#         INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

