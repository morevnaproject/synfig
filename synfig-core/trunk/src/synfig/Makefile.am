# $Id$

SUBDIRS = \
	layers targets valuenodes renderers

MAINTAINERCLEANFILES = \
	Makefile.in

INCLUDES = \
	-I$(top_builddir) -I$(top_srcdir)/src @INCLTDL@ -Ilayers -Itargets -Ivaluenodes -Irenderers

EXTRA_DIST = \
	surfacenew.cpp pch.h synfig.nsh

IMPORTERHEADERS = \
	listimporter.h
IMPORTERSOURCES = \
	listimporter.cpp

VALUEHEADERS = \
	blinepoint.h   gradient.h   value.h
VALUESOURCES = \
	blinepoint.cpp gradient.cpp value.cpp

SYNFIGHEADERS = \
	angle.h canvasbase.h general.h guidset.h interpolation.h nodebase.h protocol.h quick_rng.h real.h releases.h	\
	segment.h smartfile.h synfig_string.h string_decl.h surfacenew.h synfig.h types.h vector.h version.h			\
	activepoint.h	blur.h	 canvas.h   color.h   context.h	  curve_helper.h   curveset.h	distance.h		\
	exception.h   gamma.h	guid.h	 importer.h   keyframe.h   layer.h   loadcanvas.h   main.h   module.h		\
	mutex.h	  node.h   palette.h   paramdesc.h   polynomial_root.h	 rect.h	  renddesc.h   render.h			\
	savecanvas.h   surface.h   target.h   synfig_time.h   timepointcollect.h   transform.h	  uniqueid.h   valuenode.h	\
	waypoint.h
SYNFIGSOURCES = \
	activepoint.cpp blur.cpp canvas.cpp color.cpp context.cpp curve_helper.cpp curveset.cpp distance.cpp		\
	exception.cpp gamma.cpp guid.cpp importer.cpp keyframe.cpp layer.cpp loadcanvas.cpp main.cpp module.cpp		\
	mutex.cpp node.cpp palette.cpp paramdesc.cpp polynomial_root.cpp rect.cpp renddesc.cpp render.cpp		\
	savecanvas.cpp surface.cpp target.cpp synfig_time.cpp timepointcollect.cpp transform.cpp uniqueid.cpp valuenode.cpp	\
	waypoint.cpp

lib_LTLIBRARIES = libsynfig.la
libsynfig_la_SOURCES = \
	$(VALUEHEADERS) $(VALUESOURCES) \
	$(SYNFIGHEADERS) $(SYNFIGSOURCES) $(IMPORTERHEADERS) $(IMPORTERSOURCES)
libsynfig_la_LIBADD = @LIBLTDL@ @SYNFIG_LIBS@ @LIBADD_DL@ layers/liblayer.la targets/libtarget.la valuenodes/libvaluenode.la renderers/librenderer.la -lGLEW -lGL
libsynfig_la_CXXFLAGS = @SYNFIG_CFLAGS@ -DLIBDIR="\"@libdir@\"" -DSYSCONFDIR="\"@sysconfdir@\""
libsynfig_la_LDFLAGS = -export-dynamic -no-undefined -version-info 0:0:0

include_synfigdir=@synfigincludedir@/synfig
include_synfig_HEADERS = \
	$(SYNFIGHEADERS) $(IMPORTERHEADERS) $(VALUEHEADERS)
