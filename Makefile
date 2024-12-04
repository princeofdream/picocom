
VERSION = 3.2a

#CC ?= gcc
CPPFLAGS += -DVERSION_STR=\"$(VERSION)\"
CFLAGS += -Wall -g

LD = $(CC)
LDFLAGS ?= -g
LDLIBS ?=

all: ppcom
OBJS =

## This is the maximum size (in bytes) the output (e.g. copy-paste)
## queue is allowed to grow to. Zero means unlimitted.
TTY_Q_SZ = 0
CPPFLAGS += -DTTY_Q_SZ=$(TTY_Q_SZ)

## Comment this out to disable high-baudrate support
CPPFLAGS += -DHIGH_BAUD

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
OBJS += src/linenoise-1.0/linenoise.o
src/linenoise-1.0/linenoise.o : src/linenoise-1.0/linenoise.c src/linenoise-1.0/linenoise.h

## Comment this in to enable (force) custom baudrate support
## even on systems not enabled by default.
#CPPFLAGS += -DUSE_CUSTOM_BAUD

## Comment this in to disable custom baudrate support
## on ALL systems (even on these enabled by default).
#CPPFLAGS += -DNO_CUSTOM_BAUD

## Comment this IN to remove help strings (saves ~ 4-6 Kb).
#CPPFLAGS += -DNO_HELP


OBJS += src/ppcom.o
OBJS += src/term.o
OBJS += src/fdio.o
OBJS += src/split.o
OBJS += src/custbaud.o
OBJS += src/termios2.o
OBJS += src/custbaud_bsd.o

ppcom : $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

ppcom.o : src/ppcom.c src/term.h src/fdio.h src/split.h src/custbaud.h
term.o : src/term.c src/term.h src/termios2.h src/custbaud_bsd.h src/custbaud.h
split.o : src/split.c src/split.h
fdio.o : src/fdio.c src/fdio.h
termios2.o : src/termios2.c src/termios2.h src/termbits2.h src/custbaud.h
custbaud_bsd.o : src/custbaud_bsd.c src/custbaud_bsd.h src/custbaud.h
custbaud.o : src/custbaud.c src/custbaud.h

.c.o :
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<


doc : docs/ppcom.1.html docs/ppcom.1 docs/ppcom.1.pdf

ppcom.1 : docs/ppcom.1.md
	sed 's/\*\*\[/\*\*/g;s/\]\*\*/\*\*/g' $? \
	| pandoc -s -t man \
	    -Vfooter="Picocom $(VERSION)" -Vdate="`date -I`" \
	    -Vadjusting='l' \
	    -Vhyphenate='' \
	    -o $@

ppcom.1.html : docs/ppcom.1.md
	pandoc -s -t html \
	    --template ~/.pandoc/tmpl/manpage.html \
	    -c ~/.pandoc/css/normalize-noforms.css \
	    -c ~/.pandoc/css/manpage.css \
	    --self-contained \
	    -Vversion="v$(VERSION)" -Vdate="`date -I`" \
	    -o $@ $?

ppcom.1.pdf : docs/ppcom.1
	groff -man -Tpdf $? > $@


clean:
	rm -f src/*.o
	rm -f sec/linenoise-1.0/*.o
	rm -f *~
	rm -f \#*\#

distclean: clean
	rm -f ppcom

realclean: distclean
	rm -f docs/ppcom.1
	rm -f docs/ppcom.1.html
	rm -f docs/ppcom.1.pdf

