
AUTOMAKE_OPTIONS=foreign

AM_CFLAGS  = -I${top_srcdir}/include
AM_CFLAGS += -DTTY_Q_SZ=0
AM_CFLAGS += -DHIGH_BAUD
AM_CFLAGS += -DUSE_FLOCK
AM_CFLAGS += -DHISTFILE=\".picocom_history\"
AM_CFLAGS += -DLINENOISE

AM_LDFLAGS = -lpthread

AM_CXXFLAGS = ${AM_CFLAGS}

bin_PROGRAMS   = ppcom
ppcom_SOURCES  = linenoise.c
ppcom_SOURCES += custbaud_bsd.c \
				 custbaud.c \
				 fdio.c \
				 picocom.c \
				 split.c \
				 term.c \
				 termios2.c

ppcom_headers = custbaud_bsd.h \
				custbaud.h \
				fdio.h \
				split.h \
				termbits2.h \
				term.h \
				termios2.h

target:
	echo ${ppcom_SOURCES}

.PHONY: ppcom-headers

ppcom-headers: ${ppcom_headers}

