/* === S Y N F I G ========================================================= */
/*!	\file synfig/render.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDER_H
#define __SYNFIG_RENDER_H

/* === H E A D E R S ======================================================= */

#include "targets/target_scanline.h"
#include "vector.h"
#include "color.h"
#include "renddesc.h"
#include "general.h"
#include "layer.h"
#include "canvas.h"
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//! Renders starting at \a context to \a target
/*! \warning \a Target::set_rend_desc() must have
**		already been called on \a target before
**		you call this function!
*/
extern bool render(Context context, Target_Scanline::Handle target, const RendDesc &desc,ProgressCallback *);

extern bool parametric_render(Context context, Surface &surface, const RendDesc &desc,ProgressCallback *);

extern bool render_threaded(	Context context,
	Target_Scanline::Handle target,
	const RendDesc &desc,
	ProgressCallback *callback,
	int threads);

}; /* end namespace synfig */

/* -- E N D ----------------------------------------------------------------- */

#endif
