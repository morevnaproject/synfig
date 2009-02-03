# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

INCLUDES = \
	-I$(top_builddir) -I$(top_srcdir)/src @INCLTDL@

TARGETHEADERS = \
	target_multi.h	 target_null.h	 target_null_tile.h   target_scanline.h	  target_tile.h
TARGETSOURCES = \
	target_multi.cpp target_null.cpp target_null_tile.cpp target_scanline.cpp target_tile.cpp

noinst_LTLIBRARIES = \
	libtarget.la
libtarget_la_SOURCES = \
	$(TARGETHEADERS) $(TARGETSOURCES)
libtarget_la_CXXFLAGS = @SYNFIG_CFLAGS@ -DLIBDIR="\"@libdir@\"" -DSYSCONFDIR="\"@sysconfdir@\""
libtarget_la_LDFLAGS = -no-undefined

include_synfigdir=@synfigincludedir@/synfig/targets
include_synfig_HEADERS = \
	$(TARGETHEADERS)
