cmake_minimum_required(VERSION 3.4)

set(prj_name InterCommunication_Client)

project(${prj_name})

add_executable(${prj_name} main.cpp)

if (${CMAKE_SYSTEM_NAME} MATCHES "QNX")
	list(APPEND OS_LIBS socket)
endif()

target_link_libraries(${prj_name} -pthread InterCommunication ${TARGET_SOMEIP_LIBS} ${OS_LIBS})

install(TARGETS ${prj_name}
	LIBRARY DESTINATION ${INSTALL_RELEASE_DIR}/soa_lib
	ARCHIVE DESTINATION ${INSTALL_RELEASE_DIR}/soa_lib
	RUNTIME DESTINATION ${INSTALL_RELEASE_DIR}/bin
)

