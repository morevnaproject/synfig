/* === S Y N F I G ========================================================= */
/*!	\file context.h
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

#ifndef __SYNFIG_CONTEXT_H
#define __SYNFIG_CONTEXT_H

/* === H E A D E R S ======================================================= */

#include "canvasbase.h"
#include "rect.h"
// For RenderMethod enum (maybe we can add it to RendDesc)
#include "target.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Vector;
typedef Vector Point;
class Color;
class Surface;
class RendDesc;
class ProgressCallback;
class Layer;
class Time;
class Rect;

/*!	\class Context
**	\todo writeme
**	\see Layer, Canvas */
class Context : public CanvasBase::const_iterator
{
private:
	/*!	Calls accelerated rendering (software) on the child layers */
	//bool accelerated_render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const;

	/*!	Calls OpenGL rendering on the child layers */
	//bool opengl_render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb) const;
	
public:
	Context() { }

	Context(const CanvasBase::const_iterator &x):CanvasBase::const_iterator(x) { }

	Context operator=(const CanvasBase::const_iterator &x)
	{ return CanvasBase::const_iterator::operator=(x); }

	/*!	\todo write me */
	Color get_color(const Point &pos)const;

	/*!	Iterates over layers and calls the appropiate renderer */
	bool render(Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb, RenderMethod method) const;

	/*!	\todo write me */
   	void set_time(Time time)const;

	/*!	\writeme */
   	void set_time(Time time,const Vector &pos)const;

	Rect get_full_bounding_rect()const;

	/*! \writeme */
	etl::handle<Layer> hit_check(const Point &point)const;

}; // END of class Context

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
