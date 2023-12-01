
project(procmgr)

link_directories(
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

aux_source_directory(./src SOURCES)

set(PPCOM_PROC_LIBS -pthread)
if (${CMAKE_SYSTEM_NAME} MATCHES "QNX")
    list(APPEND PPCOM_PROC_LIBS socket)
endif()


add_library(${PROJECT_NAME}-shared SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME}-shared PROPERTIES VERSION ${VERSION})
set_target_properties(${PROJECT_NAME}-shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
# set_target_properties(${PROJECT_NAME}-shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(${PROJECT_NAME}-shared ${OS_LIBS})

add_library(${PROJECT_NAME}-static STATIC ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}-static ${OS_LIBS})
set_target_properties(${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}-shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION bin
        INCLUDES DESTINATION include
        MANIFEST DESTINATION share
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS ${PROJECT_NAME}-static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION bin
        INCLUDES DESTINATION include
        MANIFEST DESTINATION share
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)



