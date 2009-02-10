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

	createShaders();

	init_tessellation();
}

Renderer_OpenGL::~Renderer_OpenGL()
{
	synfig::info("Renderer_OpenGL: Closing...");
	if (_buffer)
		delete [] _buffer;

	// Delete tessellation objects
	gluDeleteTess(_tess);
	_tess = NULL;

	// BLEND_END stores the last blend value (so, the total count)
	const int NBLENDS = Color::BLEND_END;

	for (int j = 0; j < NBLENDS; j++) {
		glDetachShader(_vertex_shader[0], _program[j]);
		glDeleteShader(_vertex_shader[0]);
	}
	delete [] _vertex_shader;

	// Detach fragment shaders and delete programs
	for (int j = 0; j < NBLENDS; j++) {
		glDetachShader(_frag_shader[j], _program[j]);
		glDeleteShader(_frag_shader[j]);
		glDeleteProgram(_program[j]);
	}
	delete [] _frag_shader;
	delete [] _program;

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
Renderer_OpenGL::checkShader(GLuint s)
{
	int len = 0, written = 0;
	char *log;

	glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);

	// Undefined behaviour?? (Returns 1 on no-error (for NULL character count)
	if (len > 1)
	{
		log = new char[len];
		glGetShaderInfoLog(s, len, &written, log);
		synfig::error("Renderer_OpenGL::checkShader: %s", log);
		delete [] log;
		throw;
	}

}

void
Renderer_OpenGL::checkProgram(GLuint p)
{
	int len = 0, written = 0;
	char *log;

	glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);

	// Undefined behaviour?? (Returns 1 on no-error (for NULL character count)
	if (len > 1)
	{
		log = new char[len];
		glGetProgramInfoLog(p, len, &written, log);
		synfig::error("Renderer_OpenGL::checkProgram: %s", log);
		delete [] log;
		throw;
	}

}

void
Renderer_OpenGL::createShaders()
{
#define CREATE_FRAGMENT_SHADER(source, id) \
	_frag_shader[id] = glCreateShader(GL_FRAGMENT_SHADER);	\
	glShaderSource(_frag_shader[id], 1, source, NULL);	\
	checkShader(_frag_shader[id]);	\
	_program[id] = glCreateProgram();	\
	glAttachShader(_program[id], _vertex_shader[0]);	\
	glAttachShader(_program[id], _frag_shader[id]);	\
	glLinkProgram(_program[id]);	\
	checkProgram(_program[id]);

	const int NBLENDS = Color::BLEND_END;

	// Save space for program, vertex and fragment identifiers
	_program       = new GLuint[NBLENDS];
	_vertex_shader = new GLuint[1];
	_frag_shader   = new GLuint[NBLENDS];

	// Create minimal vertex shader
	const char *vertex_program[] = { "void main() { gl_Position = ftransform(); }" };
	_vertex_shader[0] = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vertex_shader[0], 1, vertex_program, NULL);
	checkShader(_vertex_shader[0]);

	// Create fragment shaders
	const char *blend_composite[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_composite, Color::BLEND_COMPOSITE);
	const char *blend_straight[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_straight, Color::BLEND_STRAIGHT);
	const char *blend_onto[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_onto, Color::BLEND_ONTO);
	const char *blend_straight_onto[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_straight_onto, Color::BLEND_STRAIGHT_ONTO);
	const char *blend_behind[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_behind, Color::BLEND_BEHIND);
	const char *blend_screen[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_screen, Color::BLEND_SCREEN);
	const char *blend_overlay[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_overlay, Color::BLEND_OVERLAY);
	const char *blend_hard_light[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_hard_light, Color::BLEND_HARD_LIGHT);
	const char *blend_multiply[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_multiply, Color::BLEND_MULTIPLY);
	const char *blend_divide[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_divide, Color::BLEND_DIVIDE);
	const char *blend_add[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_add, Color::BLEND_ADD);
	const char *blend_subtract[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_subtract, Color::BLEND_SUBTRACT);
	const char *blend_difference[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_difference, Color::BLEND_DIFFERENCE);
	const char *blend_brighten[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_brighten, Color::BLEND_BRIGHTEN);
	const char *blend_darken[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_darken, Color::BLEND_DARKEN);
	const char *blend_color[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_color, Color::BLEND_COLOR);
	const char *blend_hue[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_hue, Color::BLEND_HUE);
	const char *blend_saturation[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_saturation, Color::BLEND_SATURATION);
	const char *blend_luminance[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_luminance, Color::BLEND_LUMINANCE);

	const char *blend_alpha_brighten[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_alpha_brighten, Color::BLEND_ALPHA_BRIGHTEN);
	const char *blend_alpha_darken[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_alpha_darken, Color::BLEND_ALPHA_DARKEN);
	const char *blend_alpha_over[] = { "void main() { gl_FragColor = vec4(1.0, 0.5, 0.0, 1.0); } " };
	CREATE_FRAGMENT_SHADER(blend_alpha_over, Color::BLEND_ALPHA_OVER);
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

		// Create a new buffer
		if (_buffer)
			delete [] _buffer;

		// The 4 is because we've to allocate memory for the FOUR channels!!
		_buffer = new unsigned char[_vw * _vh * sizeof(surface_type) * 4];
		if (!_buffer) {
			synfig::error("Renderer_OpenGL: Cannot allocate %d bytes of memory", _vw * _vh * sizeof(surface_type));
			throw;
		}

		// Initialize textures
		memset(_buffer, 0, _vw * _vh * sizeof(surface_type));

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

			transfer_data(_buffer, j);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + j,
				_tex_target,
				_tex[j],
				MIPMAP_LEVEL);
		}

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

void
Renderer_OpenGL::blend(synfig::Color::BlendMethod blend_method)
{
	// Change read / write textures
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + _write_tex);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _read_tex);

	// Select the appropiate program
	glUseProgram(_program[blend_method]);

	// Render to do the copy
	fill();

	// Restore writting texture
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + _read_tex);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _write_tex);

void
Renderer_OpenGL::init_tessellation()
{
	// Initialize tessellation object
	_tess = gluNewTess();

	// Callbacks
	gluTessCallback(_tess, GLU_TESS_VERTEX, (GLvoid (*) ( )) &glVertex3dv);
	gluTessCallback(_tess, GLU_TESS_BEGIN, (GLvoid (*) ( )) &glBegin);
	gluTessCallback(_tess, GLU_TESS_END, (GLvoid (*) ( )) &glEnd);
	gluTessCallback(_tess, GLU_TESS_COMBINE, (GLvoid (*) ( )) &Renderer_OpenGL::tess_combine_cb);
	gluTessCallback(_tess, GLU_TESS_ERROR, (GLvoid (*) ( ))&Renderer_OpenGL::tess_error_cb);
}

void CALLBACK
Renderer_OpenGL::tess_error_cb(const GLenum code)
{
	const GLubyte *str;

	str = gluErrorString(code);
	synfig::error("Renderer_OpenGL: Tesellation error:\n%s)", str);
}

void CALLBACK
Renderer_OpenGL::tess_combine_cb(GLdouble coords[3], void *vertex_data[4], GLfloat weight[4], void **outData)
{
	GLdouble *vertex = (GLdouble *) malloc(3 * sizeof(GLdouble));
	int i;


	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
	/*for (i = 3; i < 7; i++)
		vertex[i] = weight[0] * vertex_data[0][i]
				+ weight[1] * vertex_data[1][i]
				+ weight[2] * vertex_data[2][i]
				+ weight[3] * vertex_data[3][i];*/
	*outData = vertex;
}

void
Renderer_OpenGL::end_contour()
{
	// Use the data
	for (unsigned int j = 0; j < _points.size(); j += 3)
		gluTessVertex(_tess, reinterpret_cast<GLdouble*>(&_points[j]), reinterpret_cast<void*>(&_points[j]));

	// End the contour
	gluTessEndContour(_tess);
}

void
Renderer_OpenGL::add_contour_vertex(const GLdouble x, const GLdouble y, const GLdouble z)
{
	// We've to store the data because it get used
	// when doing gluTessEndPolygon
	// and it does have to exist until then

	// We don't use a direct pointer because Point is using float,
	// and GLU tessellation object needs GLdouble
	_points.push_back(x);
	_points.push_back(y);
	_points.push_back(z);
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
