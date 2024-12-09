cmake_minimum_required(VERSION 3.4)

set(prj_name intercomm-shared)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../soa_sdk/soa_stack/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../soa_sdk/soa_stack/include/APF
	${CMAKE_CURRENT_SOURCE_DIR}/../soa_sdk/soa_stack/include/kt_someip
    ${CMAKE_CURRENT_SOURCE_DIR}/../soa_sdk/soa_stack/include/RunTime
    ${CMAKE_CURRENT_SOURCE_DIR}/../someiplib/InterCommunication
    ${CMAKE_CURRENT_SOURCE_DIR}/../someiplib/
)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../soa_sdk/soa_lib
)

project(${prj_name})
add_library(intercomm SHARED
    Intercomm.cpp
	InterCommunication_serviceimpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../someiplib/InterCommunication/InterCommunication_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../someiplib/InterCommunication/InterCommunication_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../someiplib/InterCommunication/InterCommunication_skeleton.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "QNX")
	list(APPEND OS_LIBS socket)
endif()

target_link_libraries(intercomm -pthread ${TARGET_SOMEIP_LIBS} ${OS_LIBS})

# install(TARGETS intercomm
#     LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#     RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
# )

