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

#include "synfig/color.h"
#include "synfig/vector.h"

/* === M A C R O S ========================================================= */

#define CHECK_FRAMEBUFFER_STATUS()                                    \
{                                                                     \
	GLenum status;                                                  \
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);       \
	switch(status) {                                                \
		case GL_FRAMEBUFFER_COMPLETE_EXT:                         \
			break;                                              \
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:                      \
			synfig::error("FBO: Unsupported configuration");    \
			throw;                                              \
			break;                                              \
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:            \
			synfig::error("FBO: Incomplete attachment");        \
			throw;                                              \
			break;                                              \
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:    \
			synfig::error("FBO: Incomplete missing attachment"); \
			throw;                                              \
			break;                                              \
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:            \
			synfig::error("FBO: Incomplete dimensions");        \
			throw;                                              \
			break;                                              \
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:               \
			synfig::error("FBO: Incomplete formats");           \
			throw;                                              \
			break;                                              \
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:           \
			synfig::error("FBO: Incomplete draw buffer");       \
			throw;                                              \
			break;                                              \
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:           \
			synfig::error("FBO: Incomplete read buffer");       \
			throw;                                              \
			break;                                              \
		default:                                                  \
			synfig::error("FBO: Hardware error");               \
			throw;                                              \
	}                                                               \
}


/* === T Y P E D E F S ===================================================== */

//! Type of surface
typedef float surface_type;

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class Renderer_OpenGL
{
	// Variables
	private:
#ifdef linux
		Display *dpy;
		Window win;
		GLXContext glc;
#endif
		//! Number of FBOs in GPU
		static const int N_BUFFERS = 1;
		//! Number of textures / color attachments per FBO
		static const int N_TEXTURES = 2;
		//! Mipmapping level
		static const int MIPMAP_LEVEL = 0;
		//! Current viewport width & height
		GLuint _vw, _vh;
		//! Current scene top-left and bottom-right points
		Point _tl, _br;
		//! Buffers IDs
		GLuint _fbuf[N_BUFFERS];
		//! Textures ids
		GLuint _tex[N_TEXTURES];
		//! Texture buffer
		unsigned char* _buffer;
		//! Texture target
		GLuint _tex_target;
		//! Maximum attachment points
		GLint _max_attachs;
		//! Indicates the next read and write texture
		unsigned int _write_tex, _read_tex;

	// Functions
	private:
		void checkErrors();
		void transfer_data(unsigned char* buf, unsigned int tex_num);

		inline void swap() { _read_tex = !_read_tex; _write_tex = !_write_tex; }

	public:
		Renderer_OpenGL();
		~Renderer_OpenGL();

		void set_wh(const GLuint w, const GLuint h, const Point tl, const Point br);

		inline void set_color(const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat a) { glColor4f(r, g, b, a); }

		void draw_circle(const GLfloat cx, const GLfloat cy, const GLfloat r, int precision = 0);
		void draw_rectangle(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2);

		const unsigned char* get_data(PixelFormat pf);
};	// END of class Renderer_OpenGL

};	// END of namespace synfig

/* === E N D =============================================================== */

#endif
