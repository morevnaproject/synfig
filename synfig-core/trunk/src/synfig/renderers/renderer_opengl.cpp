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
#include <math.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
//using namespace etl;
using namespace synfig;
//using namespace GLX;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Renderer_OpenGL::Renderer_OpenGL(): _vw(0), _vh(0), _pw(0), _ph(0), _buffer(NULL), _write_tex(0), _read_tex(1)
{
	// Get a context (platform-dependant code)
#ifdef linux
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		synfig::error("Renderer_OpenGL: Cannot open X display");
		throw;
	}
	Window root = DefaultRootWindow(dpy);
	// Check if multisampling it's supported
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24,
	                GLX_SAMPLE_BUFFERS_ARB, 1, GLX_SAMPLES_ARB, 4,		// Multisampling
	                None };
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	_multisampling = vi != NULL;
	if(vi == NULL) {
		// Multisampling not supported, fall back to normal window
		att[3] = None;
		if (!(vi = glXChooseVisual(dpy, 0, att))) {
			synfig::error("Renderer_OpenGL: Cannot choose X visual");
			throw;
		}
	}
	synfig::info("Renderer_OpenGL: Multisampling it's %s!", _multisampling ? "supported" : "unsupported");
	XSetWindowAttributes swa;
	swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	win = XCreateWindow(dpy, root, 0, 0, 640, 480, 0, vi->depth, InputOutput, vi->visual, CWColormap, &swa);
	// XMapWindow(dpy, win);
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	XFree(vi);
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
		delete [] _buffer;

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
Renderer_OpenGL::checkErrors()
{
	GLint err = glGetError();
	if (err != GL_NO_ERROR) {
		synfig::error("Renderer_OpenGL: OpenGL Error (%s)", gluErrorString(err));
		throw;
	}
}

void
Renderer_OpenGL::transfer_data(unsigned char* buf, unsigned int tex_num)
{
	glBindTexture(_tex_target, _tex[tex_num]);
	glTexSubImage2D(_tex_target, MIPMAP_LEVEL, 0, 0, _vw, _vh, GL_RGBA, GL_FLOAT, buf);
}


void
Renderer_OpenGL::set_wh(const GLuint vw, const GLuint vh, const Point tl, const Point br)
{
	bool changed = false;
	if ((tl != _tl) || (br != _br)) {
		changed = true;

		_tl = tl;
		_br = br;

		// Initialize matrices
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// We use the coords inverted (bottom and top) because OpenGL uses lower-left coordinates
		gluOrtho2D(_tl[0], _br[0], _tl[1], _br[1]);
		glMatrixMode(GL_MODELVIEW);
	}
	if ((vw) && (vh) && ((vw != _vw) || (_vh != vh))) {
		changed = true;

		_vw = vw;
		_vh = vh;

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glViewport(0, 0, _vw, _vh);

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
			_vw, _vh, 0, GL_RGBA, GL_FLOAT, NULL);

			checkErrors();

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + j,
				_tex_target,
				_tex[j],
				MIPMAP_LEVEL);
		}

		// Create a new buffer
		delete [] _buffer;
		_buffer = new unsigned char[_vw * _vh * sizeof(surface_type)];
		if (!_buffer) {
			synfig::error("Renderer_OpenGL: Cannot allocate %d bytes of memory", _vw * _vh * sizeof(surface_type));
			throw;
		}

		// Initialize textures
		memset(_buffer, 0, _vw * _vh * sizeof(surface_type));

		transfer_data(_buffer, _write_tex);
		transfer_data(_buffer, _read_tex);

		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _write_tex);

		glPolygonMode(GL_FRONT, GL_FILL);

		checkErrors();

		CHECK_FRAMEBUFFER_STATUS();
	}
	if (changed) {
		// Re-calculate ratios
		_pw = (br[0] - tl[0]) / _vw;
		_ph = (br[1] - tl[1]) / _vh;
	}
}

// FIXME: This is the same code as in glPlayfield!
void
Renderer_OpenGL::draw_circle(const GLfloat cx, const GLfloat cy, const GLfloat r, int precision)
{
	// From http://slabode.exofire.net/circle_draw.shtml
	// TODO: Fix precision function to depend on viewport size relation (better than now)
	if (precision <= 0)
		precision = int(2.0f * M_PI / (acosf(1 - 0.25 / ((r*r)/_pw))));	// We're using _pw, but _ph works as well
	float theta = 2 * M_PI / float(precision);
	float tangetial_factor = tanf(theta);//calculate the tangential factor

	float radial_factor = cosf(theta);//calculate the radial factor

	float x = r;//we start at angle = 0

	float y = 0;

	// FIXME: USe a vertex array or buffer
	glBegin(GL_POLYGON);
	for(int j = 0; j < precision; j++)
	{
		glVertex2f(x + cx, y + cy);//output vertex

		//calculate the tangential vector
		//remember, the radial vector is (x, y)
		//to get the tangential vector we flip those coordinates and negate one of them

		float tx = -y;
		float ty = x;

		//add the tangential vector

		x += tx * tangetial_factor;
		y += ty * tangetial_factor;

		//correct using the radial factor

		x *= radial_factor;
		y *= radial_factor;
	}
	glEnd();
}

void
Renderer_OpenGL::draw_rectangle(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2)
{
	glBegin(GL_QUADS);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();
}

void
Renderer_OpenGL::fill()
{
	glBegin(GL_QUADS);
	glVertex2f(_tl[0], _tl[1]);
	glVertex2f(_br[0], _tl[1]);
	glVertex2f(_br[0], _br[1]);
	glVertex2f(_tl[0], _br[1]);
	glEnd();
}

const unsigned char*
Renderer_OpenGL::get_data(PixelFormat pf)
{
	GLenum format, type;

	// Get selected format
	if (pf & PF_GRAY)
		if (pf & PF_A)
			format = GL_LUMINANCE_ALPHA;
		else
			format = GL_LUMINANCE;
	else if (pf & PF_A) {
		if (pf & PF_BGR)
			format = GL_BGRA;
		else
			format = GL_RGBA;
	}
	else {
		if (pf & PF_BGR)
			format = GL_BGR;
		else
			format = GL_RGB;
	}

	// Get selected type
	// TODO: Add (check and support for) half and index types
	if (pf & PF_8BITS)
		type = GL_UNSIGNED_BYTE;
	else if (pf & PF_16BITS)
		type = GL_UNSIGNED_SHORT;
	else if (pf & PF_32BITS)
		type = GL_UNSIGNED_INT;
	else if (pf & PF_FLOAT)
		type = GL_FLOAT;
	else {
		synfig::error("Undefined pixel format size");
		throw;
	}

	swap();
	glEnable(GL_MULTISAMPLE_ARB);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + _read_tex);
	glReadPixels(0, 0, _vw, _vh, format, type, _buffer);
	glDisable(GL_MULTISAMPLE_ARB);

	checkErrors();

	return _buffer;
}
