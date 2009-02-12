/* === S Y N F I G ========================================================= */
/*!	\file renderer_opengl_config.cpp
**	\brief Renderer_OpenGL_Config
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "renderer_opengl_config.h"
#include "synfig/general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_OpenGL_Config::Renderer_OpenGL_Config(bool debug):
	_debug(debug), _multisampling(false),
	// General
	_tex_non_pot(false), _tex_rect(false),
	_tex_target(GL_TEXTURE_2D), _tex_internal_format(GL_RGBA8),
	// FBOs
	_fbo(false), _fbo_max_attachs(0), _fbo_blitting(false),
	// FBO Multisampling
	_fbo_multisampling(false), _ms_max_samples(0), _ms_samples(0),
	_fbo_coverage_multisampling(false), _ms_max_coverage(0)
{

}

Renderer_OpenGL_Config::~Renderer_OpenGL_Config()
{
	_ms_coverage_comb.clear();
}

void
Renderer_OpenGL_Config::check_caps()
{
#define SUPPORTED(var) var ? "supported" : "not supported"
	// General
	_tex_non_pot = GLEW_ARB_texture_non_power_of_two;
	_tex_rect = GLEW_ARB_texture_rectangle;
	// Use ARB_texture_rectangle only if ARB_texture_non_power_of_two isn't supported
	// and ARB_texture_rectangle is supported
	if ((!_tex_non_pot) && (_tex_rect))
		_tex_target = GL_TEXTURE_RECTANGLE_ARB;
	if (_debug) synfig::info(__FILE__": Texture target is %s, non_power_of_two %s, texture rectangle %s",
			_tex_target == GL_TEXTURE_2D ? "GL_TEXTURE_2D" : "GL_TEXTURE_RECTANGLE_ARB",
			SUPPORTED(_tex_non_pot),
			SUPPORTED(_tex_rect));
	// FBOs
	// Check FBOs support
	_fbo = GLEW_EXT_framebuffer_object;
	if (_fbo) {
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &_fbo_max_attachs);
		if (_debug) synfig::info(__FILE__": FBOs supported, max. attachment points %d", _fbo_max_attachs);
		_fbo_blitting = GLEW_EXT_framebuffer_blit;
		if (_debug) synfig::info(__FILE__": FBO blitting %s", SUPPORTED(_fbo_blitting));
	}


	// FBO Multisampling
	// Check framebuffer multisample support
	_fbo_multisampling = GLEW_EXT_framebuffer_multisample;
	if (_fbo_multisampling) {
		glGetIntegerv(GL_MAX_SAMPLES_EXT, &_ms_max_samples);
		_ms_samples = _ms_max_samples;
		if (_debug) synfig::info(__FILE__": Multisample supported, max. samples %d", _ms_max_samples);
	}

	// Check framebuffer multisample coverage support
	_fbo_coverage_multisampling = GLEW_NV_framebuffer_multisample_coverage;
	if (_fbo_coverage_multisampling) {
		glGetIntegerv(GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV, &_ms_max_coverage);
		if (_debug) synfig::info(__FILE__": Multisample coverage supported, max. combinations %d", _ms_max_coverage);

		_ms_coverage_comb.resize(_ms_max_coverage);
		glGetIntegerv(GL_MULTISAMPLE_COVERAGE_MODES_NV, (GLint*)&_ms_coverage_comb[0]);
		_ms_coverage = _ms_coverage_comb[_ms_max_coverage - 1];
		if (_debug) {
			std::vector<coverage_unit>::iterator it;
			for (it = _ms_coverage_comb.begin(); it != _ms_coverage_comb.end(); ++it) {
				synfig::info(__FILE__":\tCoverage combination: color samples %d, coverage samples %d", it->color_samples, it->coverage_samples);
			}
		}
	}
}

bool
Renderer_OpenGL_Config::initGL()
{
	// Get a context (platform-dependant code)
#ifdef linux
	_dpy = XOpenDisplay(NULL);
	if(_dpy == NULL) {
		synfig::error(__FILE__": Cannot open X display");
		throw;
	}
	Window root = DefaultRootWindow(_dpy);
	// Check if multisampling it's supported
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24,
	                GLX_SAMPLE_BUFFERS_ARB, 1, GLX_SAMPLES_ARB, 4,		// Multisampling
	                None };
	XVisualInfo *vi = glXChooseVisual(_dpy, 0, att);
	_multisampling = vi != NULL;
	if(vi == NULL) {
		// Multisampling not supported, fall back to normal window
		att[3] = None;
		if (!(vi = glXChooseVisual(_dpy, 0, att))) {
			synfig::error(__FILE__": Cannot choose X visual");
			throw;
		}
	}
	if (_debug) synfig::info(__FILE__": Multisampling it's %s!", _multisampling ? "supported" : "unsupported");
	XSetWindowAttributes swa;
	swa.colormap = XCreateColormap(_dpy, root, vi->visual, AllocNone);
	_win = XCreateWindow(_dpy, root, 0, 0, 640, 480, 0, vi->depth, InputOutput, vi->visual, CWColormap, &swa);
	// XMapWindow(_dpy, _win);
	_glc = glXCreateContext(_dpy, vi, NULL, GL_TRUE);
	XFree(vi);
	glXMakeCurrent(_dpy, _win, _glc);
#endif

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		synfig::error(__FILE__": Failed to initialize GLEW");
		throw;
	}

	// Pack and unpack pixels without padding
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	check_caps();

	return true;
}

bool
Renderer_OpenGL_Config::closeGL()
{
	// Release context (platform dependant code)
#ifdef linux
	glXMakeCurrent(_dpy, None, NULL);
	glXDestroyContext(_dpy, _glc);
	XDestroyWindow(_dpy, _win);
	XCloseDisplay(_dpy);
#endif
	return true;
}
