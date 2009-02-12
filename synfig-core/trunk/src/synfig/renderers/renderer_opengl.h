/* === S Y N F I G ========================================================= */
/*!	\file renderer_opengl.h
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
#include <GL/glu.h>

#include "synfig/color.h"
#include "synfig/vector.h"

#include <vector>

/* === M A C R O S ========================================================= */

// From GLEW (This must't be a need!)
#ifndef CALLBACK
#define GLEW_CALLBACK_DEFINED
#  if defined(__MINGW32__)
#    define CALLBACK __attribute__ ((__stdcall__))
#  elif (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#    define CALLBACK __stdcall
#  else
#    define CALLBACK
#  endif
#endif

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

class Renderer_OpenGL_Config;

namespace synfig {

class Renderer_OpenGL
{
	// Variables
	private:
		//! Configuration object
		Renderer_OpenGL_Config *config;
		//! Number of FBOs in GPU (one multisampled, one normal)
		static const unsigned int N_BUFFERS = 2;
		//! Array indexes of FBOs
		static const unsigned int _fbo_multisampled = 0, _fbo_normal = 1;
		//! Number of textures / color attachments (one for the result, one for the buffer, one for the multisample result)
		static const unsigned int N_TEXTURES = 3;
		//! Array indexes of textures
		static const unsigned int _tex_write = 0, _tex_buffer = 1, _tex_result = 2;
		//! Number of renderbuffers (one for the multisampling)
		static const unsigned int N_RENDERBUFFERS = 1;
		//! Array indexes of renderbuffers
		static const unsigned int _rb_multisampled = 0;
		//! Mipmapping level
		static const unsigned int MIPMAP_LEVEL = 0;
		//! Current viewport width & height
		GLuint _vw, _vh;
		//! Current scene top-left and bottom-right points
		Point _tl, _br;
		//! Current scene ratios
		float _pw, _ph;
		//! Buffers IDs
		GLuint _fbuf[N_BUFFERS];
		//! Textures IDs
		GLuint _tex[N_TEXTURES];
		//! Renderbuffers IDs
		GLuint _rb[N_RENDERBUFFERS];
		//! Texture buffer
		unsigned char* _buffer;

		// Transformation
		//! Enumeration that holds the transformation types
		enum Transformation {
			ROTATE_TRANS = 1,
			TRANSLATE_TRANS,
			ZOOM_TRANS
		};
		//! Stores the orders of base transformations (first layer)
		std::vector<Transformation> _trans_list;
		//! Stores the accumulated rotations (to be able to do post-rotation)
		GLfloat _rotation;
		//! Stores the rotation origin
		Point _rotation_origin;
		//! Stores the accumulated translations (to be able to do post-translation)
		Point _translation;
		//! Stores the accumulated scalation (zoom) (to be able to do post-scale)
		GLfloat _scale;
		//! Stores the zoom origin
		Point _scale_origin;

		// Shaders
		//! Program ids
		GLuint *_program;
		//! Vertex shader ids
		GLuint *_vertex_shader;
		//! Fragment shader ids
		GLuint *_frag_shader;

		// Tessellation
		//! Tessellation object
		GLUtesselator *_tess;
		//! Stores the points passed using set_contour_data()
		std::vector<GLdouble> _points[2];
		//! Stores the current list being used (for invert behaviour to work)
		unsigned int _curr_contour;
	// Functions
	private:
		void checkShader(GLuint s);
		void checkProgram(GLuint p);
		void createShaders();
		void checkErrors();

		// Transformation
		void add_trans(Transformation trans);
		void apply_trans();

		// Tessellation
		void init_tessellation();
		static void CALLBACK tess_error_cb(const GLenum code);
		static void CALLBACK tess_combine_cb(GLdouble coords[3], void *vertex_data[4], GLfloat weight[4], void **outData);

		void transfer_data(unsigned char* buf, unsigned int tex_num);

	public:
		Renderer_OpenGL();
		~Renderer_OpenGL();

		void set_wh(const GLuint w, const GLuint h, const Point tl, const Point br);
		void reset();

		// Transformation
		void pre_rotate(const GLfloat angle, const Point origin);
		void post_rotate(const GLfloat angle, const Point origin);
		void pre_translate(const Point origin);
		void post_translate(const Point origin);
		void pre_zoom(const GLfloat zoom, const Point origin);
		void post_zoom(const GLfloat zoom, const Point origin);

		inline void set_color(const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat a) { glColor4f(r, g, b, a); }
		inline void set_color(synfig::Color color) { glColor4f(color.get_r(), color.get_g(), color.get_b(), color.get_a()); }

		void draw_circle(const GLfloat cx, const GLfloat cy, const GLfloat r, int precision = 0);
		void draw_rectangle(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2);
		void fill();

		// Tessellation
		inline void set_winding_style(bool even_odd) { gluTessProperty(_tess, GLU_TESS_WINDING_RULE, even_odd ? GLU_TESS_WINDING_ODD : GLU_TESS_WINDING_NONZERO); }
		inline void begin_polygon() { gluTessBeginPolygon(_tess, NULL); _points[0].clear(); _points[1].clear(); _curr_contour = 0; }
		inline void end_polygon() { gluTessEndPolygon(_tess); }
		inline void begin_contour() { gluTessBeginContour(_tess); }
		void end_contour();
		void add_contour_vertex(const GLdouble x, const GLdouble y, const GLdouble z = 0);

		void blend(synfig::Color::BlendMethod blend_method);

		const unsigned char* get_data(PixelFormat pf);
};	// END of class Renderer_OpenGL

};	// END of namespace synfig

/* === E N D =============================================================== */

#endif
