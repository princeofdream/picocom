
AUTOMAKE_OPTIONS=foreign

AM_CFLAGS = -I${top_srcdir}/src/include
AM_CFLAGS += -I${top_srcdir}/src/libsrvmgr/include
AM_CFLAGS += -DTTY_Q_SZ=0
AM_CFLAGS += -DHIGH_BAUD
AM_CFLAGS += -DUSE_FLOCK
AM_CFLAGS += -DHISTFILE=\".ppcom_history\"
AM_CFLAGS += -DLINENOISE

AM_CFLAGS += -DCONFIG_LIB_SRVMGR

AM_CFLAGS += -g -O2

AM_LDFLAGS = -lpthread
AM_LDFLAGS += -lsrvmgr
AM_LDFLAGS += -L${top_srcdir}/src/libsrvmgr/

AM_CXXFLAGS = ${AM_CFLAGS}

bin_PROGRAMS   = ppcom
bin_PROGRAMS  += ppcom_cli
ppcom_SOURCES  = linenoise.cpp
ppcom_SOURCES += custbaud_bsd.cpp \
				 custbaud.cpp \
				 fdio.cpp \
				 ppcom.cpp \
				 split.cpp \
				 term.cpp \
				 termios2.cpp

ppcom_headers = custbaud_bsd.h \
				custbaud.h \
				fdio.h \
				split.h \
				termbits2.h \
				term.h \
				termios2.h

ppcom_cli_SOURCES  = ppcom_cli.cpp

LDADD = ${top_srcdir}/src/libsrvmgr/libsrvmgr.la

target:
	echo ${ppcom_SOURCES}

.PHONY: ppcom-headers clean-local distclean-local

ppcom-headers: ${ppcom_headers}

clean-local:

distclean-local:
	-rm -rf Makefile.in


