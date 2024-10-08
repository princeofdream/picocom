
#CC ?= gcc
CPPFLAGS += -DVERSION_STR=\"$(VERSION)\"
CFLAGS += -Wall -g

LD = $(CXX)
LDFLAGS ?=

LDFLAGS += \
		   -lpthread \
		   -L../libprocmgr \
		   -Wl,-rpath=../libprocmgr \
		   -L./libprocmgr \
		   -Wl,-rpath=./libprocmgr \
		   -lprocmgr

all: ppcom ppcom_cli
OBJS =

## This is the maximum size (in bytes) the output (e.g. copy-paste)
## queue is allowed to grow to. Zero means unlimitted.
TTY_Q_SZ = 0
CPPFLAGS += -DTTY_Q_SZ=$(TTY_Q_SZ)

## Comment this out to disable high-baudrate support
CPPFLAGS += -DHIGH_BAUD -I./include -I../libprocmgr/include

## Normally you should NOT enable both: UUCP-style and flock(2)
## locking.

## Comment this out to disable locking with flock
CPPFLAGS += -DUSE_FLOCK

## Comment these out to disable UUCP-style lockdirs
#UUCP_LOCK_DIR=/var/lock
#CPPFLAGS += -DUUCP_LOCK_DIR=\"$(UUCP_LOCK_DIR)\"

## Comment these out to disable "linenoise"-library support
HISTFILE = .ppcom_history
CPPFLAGS += -DHISTFILE=\"$(HISTFILE)\" \
	    -DLINENOISE
OBJS += src/linenoise.o
linenoise.o : src/linenoise.cpp src/linenoise.h

## Comment this in to enable (force) custom baudrate support
## even on systems not enabled by default.
#CPPFLAGS += -DUSE_CUSTOM_BAUD

## Comment this in to disable custom baudrate support
## on ALL systems (even on these enabled by default).
#CPPFLAGS += -DNO_CUSTOM_BAUD

## Comment this IN to remove help strings (saves ~ 4-6 Kb).
#CPPFLAGS += -DNO_HELP

OBJS += \
		src/term.o \
		src/fdio.o \
		src/split.o \
		src/custbaud.o \
		src/termios2.o  \
		src/custbaud_bsd.o \
		src/ctrl_term.o

PPCOM_OBJS = $(OBJS) src/ppcom.o
PPCOM_CLI_OBJS = $(OBJS) src/ppcom_cli.o

ppcom : $(PPCOM_OBJS)
	set -e;for i in $(OBJS); do $(CXX) $(CFLAGS) -c $(LDFLAGS) $$i;done
	$(LD) -o $@ $(PPCOM_OBJS) $(LDFLAGS)
ppcom_cli : $(PPCOM_CLI_OBJS)
	set -e;for i in $(OBJS); do $(CXX) $(CFLAGS) -c $(LDFLAGS) $$i;done
	$(LD) $(LDFLAGS) -o $@ $(PPCOM_CLI_OBJS) $(LDLIBS) -L../libprocmgr -Wl,-rpath=../libprocmgr -lprocmgr

HEADERS = custbaud_bsd.h  custbaud.h  debug.h  fdio.h  ctrl_term.h  split.h  termbits2.h  term.h  termios2.h

# ppcom.o : ppcom.cpp ../include/term.h ../include/fdio.h ../include/split.h ../include/custbaud.h
# term.o : term.cpp ../include/term.h ../include/termios2.h ../include/custbaud_bsd.h ../include/custbaud.h
# split.o : split.cpp ../include/split.h
# fdio.o : fdio.cpp ../include/fdio.h
# termios2.o : termios2.cpp ../include/termios2.h ../include/termbits2.h ../include/custbaud.h
# custbaud_bsd.o : custbaud_bsd.cpp ../include/custbaud_bsd.h ../include/custbaud.h
# custbaud.o : custbaud.cpp ../include/custbaud.h
# ctrl_term.o : ctrl_term.cpp

.cpp.o :
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<


doc : ppcom.1.html ppcom.1 ppcom.1.pdf

ppcom.1 : ppcom.1.md
	sed 's/\*\*\[/\*\*/g;s/\]\*\*/\*\*/g' $? \
	| pandoc -s -t man \
	    -Vfooter="Picocom $(VERSION)" -Vdate="`date -I`" \
	    -Vadjusting='l' \
	    -Vhyphenate='' \
	    -o $@

ppcom.1.html : ppcom.1.md
	pandoc -s -t html \
	    --template ~/.pandoc/tmpl/manpage.html \
	    -c ~/.pandoc/css/normalize-noforms.css \
	    -c ~/.pandoc/css/manpage.css \
	    --self-contained \
	    -Vversion="v$(VERSION)" -Vdate="`date -I`" \
	    -o $@ $?

ppcom.1.pdf : ppcom.1
	groff -man -Tpdf $? > $@


clean:
	rm -f src/*.o
	rm -f *~
	rm -f \#*\#
	rm -f ppcom ppcom_cli

distclean: clean
	rm -f ppcom

realclean: distclean
	rm -f ppcom.1
	rm -f ppcom.1.html
	rm -f ppcom.1.pdf
	rm -f CHANGES
