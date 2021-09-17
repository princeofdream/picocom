SUBDIRS = libsrvmgr
SUBDIRS += ppcom

AM_CFLAGS = -I${top_srcdir}/src/include
AM_CFLAGS += -I${top_srcdir}/src/libsrvmgr/include

distclean-local:
	-rm -rf compile missing config.guess
	-rm -rf INSTALL configure depcomp
	-rm -rf config.sub config.h.in Makefile.in install-sh
	-rm -rf autom4te.cache aclocal.m4
	-rm -rf config*~
	-rm -rf ltmain.sh

