/* === S Y N F I G ========================================================= */
/*!	\file renderer_opengl_config.h
**	\brief Renderer_OpenGL_Config
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

#ifndef __RENDERER_OPENGL_CONFIG_H
#define __RENDERER_OPENGL_CONFIG_H

/* === H E A D E R S ======================================================= */

#include <GL/glew.h>
#include <GL/gl.h>
#ifdef linux
#include <GL/glx.h>

#include <vector>

#endif

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

struct coverage_unit {
	GLint color_samples, coverage_samples;

	inline coverage_unit(): color_samples(0), coverage_samples(0) {}
};

/* === C L A S S E S & S T R U C T S ======================================= */

class Renderer_OpenGL_Config
{
	// Variables
	private:
#ifdef linux
		Display *_dpy;
		Window _win;
		GLXContext _glc;
#endif
		//! Stores if debugging is enabled
		bool _debug;
		//! Stores if window manager multisampling it's currently supported
		bool _multisampling;

		// General
		//! Stores if textures non-POT are supported
		bool _tex_non_pot;
		//! Stores if rectangle textures are supported
		bool _tex_rect;
		//! Texture target
		GLuint _tex_target;
		//! Texture internal format
		GLuint _tex_internal_format;

		// FBOs
		//! Stores if FBOs are supported
		bool _fbo;
		//! Maximum attachment points
		GLint _fbo_max_attachs;
		//! Stores if FBO blitting is supported
		bool _fbo_blitting;

		// FBO Multisampling
		//! Stores if framebuffer multisampling it's currently supported
		bool _fbo_multisampling;
		//! Stores the number of maximum multisampling samples
		GLint _ms_max_samples;
		//! Stores the number of selected multisampling samples
		unsigned int _ms_samples;
		//! Stores if framebuffer coverage multisampling it's currently supported
		bool _fbo_coverage_multisampling;
		//! Stores the maximum number of coverage combinations
		GLint _ms_max_coverage;
		//! Stores the combinations of coverage multisampling
		std::vector<coverage_unit> _ms_coverage_comb;
		//! Stores the selected combination of coverage multisampling
		coverage_unit _ms_coverage;

	// Functions
	private:
		void check_caps();

	public:
		Renderer_OpenGL_Config(bool debug);
		~Renderer_OpenGL_Config();

		//! Initializes OpenGL, get's a context and get capabilities of the hardware
		bool initGL();
		//! Destroys the OpenGL context and closes
		bool closeGL();

		//! \return True if FBOs are supported, False otherwise
		inline bool supported_fbo() { return _fbo; }
		//! \return True if FBO Multisampling is supported, False otherwise
		inline bool supported_fbo_ms() { return _fbo_multisampling; }
		//! \return True if FBO Coverage Multisampling is supported, False otherwise
		inline bool supported_fbo_coverage_ms() { return _fbo_coverage_multisampling; }

		// General
		//! \return Selected texture target
		inline GLuint tex_target() { return _tex_target; }
		//! \return Selected texture internal format
		inline GLuint tex_internal_format() { return _tex_internal_format; }

		// FBOs
		//! \return Maximum possible number of attachments to a FBO
		inline unsigned int fbo_max_attachs() { return _fbo_max_attachs; }

		// FBO Multisampling
		//! \return Maximum possible number of multisampling samples
		inline unsigned int ms_max_samples() { return _ms_max_samples; }
		//! \return Number of multisampling samples currently selected
		inline unsigned int ms_samples() { return _ms_samples; }
		//! \return Possible combination of multisampling coverage samples
		inline std::vector<coverage_unit> ms_coverage_comb() { return _ms_coverage_comb; }
		//! \return Combination of multisampling coverage samples currently selected
		inline coverage_unit ms_coverage() { return _ms_coverage; }
};

/* === E N D =============================================================== */

#endif
