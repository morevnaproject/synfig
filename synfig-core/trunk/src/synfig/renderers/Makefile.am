# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

INCLUDES = \
	-I$(top_builddir) -I$(top_srcdir)/src @INCLTDL@

RENDERERHEADERS = \
	renderer_opengl.h renderer_opengl_config.h
RENDERERSOURCES = \
	renderer_opengl.cpp renderer_opengl_config.cpp

noinst_LTLIBRARIES = \
	librenderer.la
librenderer_la_SOURCES = \
	$(RENDERERHEADERS) $(RENDERERSOURCES)
librenderer_la_CXXFLAGS = @SYNFIG_CFLAGS@ -DLIBDIR="\"@libdir@\"" -DSYSCONFDIR="\"@sysconfdir@\""
librenderer_la_LDFLAGS = -no-undefined

include_synfigdir=@synfigincludedir@/synfig/renderers
include_synfig_HEADERS = \
	$(RENDERERHEADERS)
