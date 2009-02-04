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

#include <string.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_OpenGL::Renderer_OpenGL(): _buffer(NULL), _write_tex(0), _read_tex(1)
{
	// Get a context (platform-dependant code)
#ifdef linux
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		synfig::error("Renderer_OpenGL: Cannot open X display");
		throw;
	}
	Window root = DefaultRootWindow(dpy);
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		synfig::error("Renderer_OpenGL: Cannot choose X visual");
		throw;
	}
	XSetWindowAttributes swa;
	swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	win = XCreateWindow(dpy, root, 0, 0, 640, 480, 0, vi->depth, InputOutput, vi->visual, CWColormap, &swa);
	// XMapWindow(dpy, win);
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
#endif

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		synfig::error("Renderer_OpenGL: Failed to initialize GLEW");
		throw;
	}

	// Pack and unpack pixels without padding
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &_max_attachs);
	if (_max_attachs < N_TEXTURES) {
		synfig::error("Renderer_OpenGL: Only %d supported textures, can't render in OpenGL", _max_attachs);
		throw;
	}

	// Init our FBOs
	glGenFramebuffersEXT(N_BUFFERS, _fbuf);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbuf[0]);

	// Choose a texture format
	_tex_target = GL_TEXTURE_2D;
	// Use ARB_texture_rectangle only if ARB_texture_non_power_of_two isn't supported
	// and ARB_texture_rectangle is supported
	if ((!GLEW_ARB_texture_non_power_of_two) && (GLEW_ARB_texture_rectangle))
		_tex_target = GL_TEXTURE_RECTANGLE_ARB;

	synfig::info("Renderer_OpenGL: FBOs up! (Texture target is %s, non_power_of_two %s)",
			_tex_target == GL_TEXTURE_2D ? "GL_TEXTURE_2D" : "GL_TEXTURE_RECTANGLE_ARB",
			GLEW_ARB_texture_non_power_of_two ? "supported" : "unsupported");
}

Renderer_OpenGL::~Renderer_OpenGL()
{
	synfig::info("Renderer_OpenGL: Closing...");
	if (_buffer)
		delete _buffer;

	// Delete our FBOs
	glDeleteFramebuffersEXT(N_BUFFERS, _fbuf);
	// Delete our textures
	glDeleteTextures(N_TEXTURES, _tex);

	// Release context (platform dependant code)
#ifdef linux
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
#endif
}

void
Renderer_OpenGL::transfer_data(surface_type *buf, unsigned int tex_num)
{
	glBindTexture(_tex_target, _tex[tex_num]);
	glTexSubImage2D(_tex_target, MIPMAP_LEVEL, 0, 0, _w, _h, GL_RGBA, GL_FLOAT, buf);
}


void
Renderer_OpenGL::set_wh(const GLuint w, const GLuint h)
{
	if ((w != _w) || (_h != h)) {
		_w = w;
		_h = h;

		// Initialize matrices
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, _w, 0, _h);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glViewport(0, 0, _w, _h);

		// Create textures and bind them to our FBO
		glGenTextures(N_TEXTURES, _tex);

		for (int j = 0; j < N_TEXTURES; j++) {
			glBindTexture(_tex_target, _tex[j]);

			glTexParameteri(_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(_tex_target, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(_tex_target, GL_TEXTURE_WRAP_T, GL_CLAMP);

			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			glTexImage2D(_tex_target, MIPMAP_LEVEL, GL_RGBA32F_ARB,
			_w, _h, 0, GL_RGBA, GL_FLOAT, NULL);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + j,
				_tex_target,
				_tex[j],
				MIPMAP_LEVEL);
		}

		// Create a new buffer
		if (_buffer)
			delete _buffer;

		_buffer = new surface_type[_w * _h];
		if (!_buffer) {
			synfig::error("Renderer_OpenGL: Cannot allocate %d bytes of memory", _w * _h);
			throw;
		}

		// Initialize textures
		memset(_buffer, 0, _w * _h);

		transfer_data(_buffer, _write_tex);
		transfer_data(_buffer, _read_tex);

		CHECK_FRAMEBUFFER_STATUS();
	}
}
