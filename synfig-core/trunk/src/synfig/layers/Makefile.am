# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

INCLUDES = \
	-I$(top_builddir) -I$(top_srcdir)/src @INCLTDL@

LAYERHEADERS = \
	layer_bitmap.h	 layer_composite.h   layer_duplicate.h	 layer_mime.h	layer_motionblur.h			\
	layer_pastecanvas.h   layer_polygon.h	layer_shape.h	layer_solidcolor.h
LAYERSOURCES = \
	layer_bitmap.cpp layer_composite.cpp layer_duplicate.cpp layer_mime.cpp layer_motionblur.cpp			\
	layer_pastecanvas.cpp layer_polygon.cpp layer_shape.cpp layer_solidcolor.cpp

noinst_LTLIBRARIES = \
	liblayer.la
liblayer_la_SOURCES = \
	$(LAYERHEADERS) $(LAYERSOURCES)
liblayer_la_CXXFLAGS = @SYNFIG_CFLAGS@ -DLIBDIR="\"@libdir@\"" -DSYSCONFDIR="\"@sysconfdir@\""
liblayer_la_LDFLAGS = -no-undefined

include_synfigdir=@synfigincludedir@/synfig/layers
include_synfig_HEADERS = \
	$(LAYERHEADERS)
