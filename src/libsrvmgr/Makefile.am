
AUTOMAKE_OPTIONS=foreign

AM_CFLAGS  = -I${top_srcdir}/src/libsrvmgr/include
AM_CFLAGS  += -lstdc++
AM_CFLAGS  += -DCOMPILE_FOR_HOST

AM_CFLAGS  += -g -O2

AM_LDFLAGS = -lpthread
AM_CXXFLAGS = ${AM_CFLAGS}


pkglib_LTLIBRARIES = libsrvmgr.la
libsrvmgr_la_LDFLAGS = -avoid-version -shared

# lib_LIBRARIES   = libsrvmgr.la
libsrvmgr_la_SOURCES  = local_service.cpp   \
					messages_manager.cpp \
					netlink_event.cpp    \
					network_manager.cpp  \
					process_manager.cpp  \
					service_manager.cpp  \
					misc_utils.cpp       \
					network_filter.cpp   \
					network_utils.cpp


libsrvmgr_la_headers = basic.h            \
					manager_client.h   \
					messages_manager.h \
					netlink_event.h    \
					network_manager.h  \
					process_manager.h  \
					local_service.h    \
					service_manager.h  \
					misc_utils.h       \
					network_filter.h   \
					network_utils.h


target:
	echo ${libsrvmgr_la_SOURCES}

.PHONY: libsrvmgr-headers clean-local distclean-local

libsrvmgr-headers: ${libsrvmgr_la_headers}

clean-local:

distclean-local:
	-rm -rf Makefile.in
	-rm -rf .deps

