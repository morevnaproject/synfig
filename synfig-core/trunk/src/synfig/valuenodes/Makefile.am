# $Id$

MAINTAINERCLEANFILES = \
	Makefile.in

INCLUDES = \
	-I$(top_builddir) -I$(top_srcdir)/src @INCLTDL@

VALUENODEHEADERS = \
	valuenode_add.h   valuenode_anglestring.h   valuenode_animated.h   valuenode_atan2.h   valuenode_bline.h	\
	valuenode_blinecalctangent.h   valuenode_blinecalcvertex.h   valuenode_blinecalcwidth.h				\
	valuenode_blinereversetangent.h   valuenode_composite.h   valuenode_const.h   valuenode_cos.h			\
	valuenode_dotproduct.h   valuenode_duplicate.h   valuenode_dynamiclist.h   valuenode_exp.h			\
	valuenode_gradientcolor.h   valuenode_gradientrotate.h   valuenode_greyed.h   valuenode_integer.h		\
	valuenode_intstring.h   valuenode_join.h   valuenode_linear.h   valuenode_log.h   valuenode_radialcomposite.h	\
	valuenode_range.h   valuenode_realstring.h   valuenode_reciprocal.h   valuenode_reference.h			\
	valuenode_repeat_gradient.h   valuenode_scale.h   valuenode_segcalctangent.h   valuenode_segcalcvertex.h	\
	valuenode_sine.h   valuenode_step.h   valuenode_stripes.h   valuenode_subtract.h   valuenode_switch.h		\
	valuenode_timedswap.h   valuenode_timeloop.h   valuenode_timestring.h   valuenode_twotone.h			\
	valuenode_vectorangle.h   valuenode_vectorlength.h   valuenode_vectorx.h   valuenode_vectory.h
VALUENODESOURCES = \
	valuenode_add.cpp valuenode_anglestring.cpp valuenode_animated.cpp valuenode_atan2.cpp valuenode_bline.cpp	\
	valuenode_blinecalctangent.cpp valuenode_blinecalcvertex.cpp valuenode_blinecalcwidth.cpp			\
	valuenode_blinereversetangent.cpp valuenode_composite.cpp valuenode_const.cpp valuenode_cos.cpp			\
	valuenode_dotproduct.cpp valuenode_duplicate.cpp valuenode_dynamiclist.cpp valuenode_exp.cpp			\
	valuenode_gradientcolor.cpp valuenode_gradientrotate.cpp valuenode_greyed.cpp valuenode_integer.cpp		\
	valuenode_intstring.cpp valuenode_join.cpp valuenode_linear.cpp valuenode_log.cpp valuenode_radialcomposite.cpp	\
	valuenode_range.cpp valuenode_realstring.cpp valuenode_reciprocal.cpp valuenode_reference.cpp			\
	valuenode_repeat_gradient.cpp valuenode_scale.cpp valuenode_segcalctangent.cpp valuenode_segcalcvertex.cpp	\
	valuenode_sine.cpp valuenode_step.cpp valuenode_stripes.cpp valuenode_subtract.cpp valuenode_switch.cpp		\
	valuenode_timedswap.cpp valuenode_timeloop.cpp valuenode_timestring.cpp valuenode_twotone.cpp			\
	valuenode_vectorangle.cpp valuenode_vectorlength.cpp valuenode_vectorx.cpp valuenode_vectory.cpp

noinst_LTLIBRARIES = \
	libvaluenode.la
libvaluenode_la_SOURCES = \
	$(VALUENODEHEADERS) $(VALUENODESOURCES)
libvaluenode_la_CXXFLAGS = @SYNFIG_CFLAGS@ -DLIBDIR="\"@libdir@\"" -DSYSCONFDIR="\"@sysconfdir@\""
libvaluenode_la_LDFLAGS = -no-undefined

include_synfigdir=@synfigincludedir@/synfig/valuenodes
include_synfig_HEADERS = \
	$(VALUENODEHEADERS)
