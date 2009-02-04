/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "renderer_opengl.h"
#include "synfig/general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_OpenGL::Renderer_OpenGL()
{
	// Get a context (platform-dependant code)
#ifdef linux
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		synfig::error("Cannot open X display");
		throw;
	}
	Window root = DefaultRootWindow(dpy);
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		synfig::error("Cannot choose X visual");
		throw;
	}
	XSetWindowAttributes swa;
	swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	win = XCreateWindow(dpy, root, 0, 0, 640, 480, 0, vi->depth, InputOutput, vi->visual, CWColormap, &swa);
	// XMapWindow(dpy, win);
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
#endif

	glewInit();
}

Renderer_OpenGL::~Renderer_OpenGL()
{
	// Release context (platform dependant code)
#ifdef linux
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
#endif
}
