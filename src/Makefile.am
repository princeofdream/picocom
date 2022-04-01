SUBDIRS = libsrvmgr
SUBDIRS += ppcom

AM_CFLAGS = -I${top_srcdir}/src/include
AM_CFLAGS += -I${top_srcdir}/src/libsrvmgr/include
AM_CFLAGS += -g

AM_LDFLAGS = -L../ppcom -Wl,-rpath=../ppcom
AM_LDFLAGS += -L../../lib/ppcom -Wl,-rpath=../../lib/ppcom

AM_CXXFLAGS = ${AM_CFLAGS}

distclean-local:
	-rm -rf compile missing config.guess
	-rm -rf INSTALL configure depcomp
	-rm -rf config.sub config.h.in Makefile.in install-sh
	-rm -rf autom4te.cache aclocal.m4
	-rm -rf config*~
	-rm -rf ltmain.sh

