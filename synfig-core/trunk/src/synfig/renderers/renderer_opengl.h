/* === S Y N F I G ========================================================= */
/*!	\file template.h
**	\brief OpenGL renderer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2009 Uiomae
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

#ifndef __SYNFIG_RENDERER_OPENGL_H
#define __SYNFIG_RENDERER_OPENGL_H

/* === H E A D E R S ======================================================= */

// glew.h HAS to be included BEFORE gl.h & glx.h
#include <GL/glew.h>
#include <GL/gl.h>
#ifdef linux
#include <GL/glx.h>
#endif

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Renderer_OpenGL
{
	private:
#ifdef linux
		Display *dpy;
		Window win;
		GLXContext glc;
#endif
	public:
		Renderer_OpenGL();
		~Renderer_OpenGL();
};	// END of class Renderer_OpenGL

};	// END of namespace synfig

/* === E N D =============================================================== */

#endif
