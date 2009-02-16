/* === S Y N F I G ========================================================= */
/*!	\file renderer_opengl.cpp
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
#include "renderer_opengl_config.h"
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

Renderer_OpenGL::Renderer_OpenGL(): _vw(0), _vh(0), _pw(0), _ph(0), _buffer(NULL),
	_rotation(0), _scale(0)
{
	// Initialize GL and check capabilities
#ifdef _DEBUG
	config = new Renderer_OpenGL_Config(true);
#else
	config = new Renderer_OpenGL_Config(false);
#endif
	config->initGL();

	if (config->fbo_max_attachs() < N_TEXTURES) {
		synfig::error("Renderer_OpenGL: Only %d supported textures, can't render in OpenGL", config->fbo_max_attachs());
		throw;
	}

	// Init our FBOs
	glGenFramebuffersEXT(N_BUFFERS, _fbuf);

	// Clear IDs
	memset(_tex, 0, sizeof(GLuint) * N_TEXTURES);
	memset(_rb, 0, sizeof(GLuint) * N_RENDERBUFFERS);

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
	}
	glDeleteShader(_vertex_shader[0]);
	delete [] _vertex_shader;

	// Detach fragment shaders and delete programs
	for (int j = 0; j < NBLENDS; j++) {
		glDetachShader(_frag_shader[j], _program[j]);
		glDeleteShader(_frag_shader[j]);
		glDeleteProgram(_program[j]);
	}
	delete [] _frag_shader;
	delete [] _program;

	// Delete our textures
	if (_tex[0] != 0)
		glDeleteTextures(N_TEXTURES, _tex);
	// Delete our renderbuffers
	if (_rb[0] != 0)
		glDeleteRenderbuffersEXT(N_RENDERBUFFERS, _rb);

	// Delete our FBOs
	glDeleteFramebuffersEXT(N_BUFFERS, _fbuf);

	config->closeGL();
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
	const char *vertex_program[] = { "varying vec2 TexCoord; void main() { gl_Position = ftransform(); TexCoord = gl_MultiTexCoord0.st; }" };
	_vertex_shader[0] = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vertex_shader[0], 1, vertex_program, NULL);
	checkShader(_vertex_shader[0]);

	// Create fragment shaders
	// TODO: This one is for checking, remove it (this can be perfectly done with normal OpenGL alpha blending)
	const char *blend_composite[] = { "varying vec2 TexCoord; uniform sampler2D source; uniform sampler2D dest;"
		"void main() {"
			"vec4 src = texture2D(source, TexCoord);"
			"vec4 dst = texture2D(dest, TexCoord);"
			"if (src.a == 0.0) gl_FragColor = dst;"
			"else if ((dst.a == 0.0) || (src.a == 1.0)) gl_FragColor = src;"
			"else {"
				"float a_src = src.a, a_dst = dst.a;"
				"src *= a_src; dst *= a_dst;"
				"dst = src + dst*(1.0 - a_src);"
				"a_dst = a_src + a_dst * (1.0 - a_src);"
				"dst /= a_dst; dst.a = a_dst;"
				"gl_FragColor = dst;"
			"}"
		"}"
	};
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
Renderer_OpenGL::add_trans(Transformation trans)
{
	if (_trans_list.size() == 0) {
		_trans_list.push_back(trans);
		return;
	}

	if (_trans_list.back() != trans) {
		std::vector<Transformation>::iterator it;
		for (it = _trans_list.begin(); it != _trans_list.end(); ++it) {
			if (*it == trans) {
				_trans_list.erase(it);
			}
		}
		_trans_list.push_back(trans);
	}
}

void
Renderer_OpenGL::apply_trans()
{
	glLoadIdentity();

	std::vector<Transformation>::iterator it;
	for (it = _trans_list.begin(); it != _trans_list.end(); it++) {
		if (*it == ROTATE_TRANS)
			glRotatef(_rotation, 0.0, 0.0, 1.0);
		else if (*it == TRANSLATE_TRANS)
			glTranslatef(_translation[0], _translation[1], 0.0);
		else	// ZOOM_TRANS
			glScalef(_scale, _scale, 0.0);
	}
	_rotation = _scale = 0;
	_rotation_origin = _scale_origin = _translation = Point(0, 0);

	_trans_list.clear();
}

void
Renderer_OpenGL::transfer_data(unsigned char* buf, unsigned int tex_num)
{
	glBindTexture(config->tex_target(), _tex[tex_num]);
	glTexSubImage2D(config->tex_target(), MIPMAP_LEVEL, 0, 0, _vw, _vh, GL_RGBA, GL_FLOAT, buf);
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

		// If we already have any generated textures, delete them
		if (_tex[0] != 0)
			glDeleteTextures(N_TEXTURES, _tex);
		// Create textures and bind them to our FBO
		glGenTextures(N_TEXTURES, _tex);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbuf[_fbo_normal]);
		for (unsigned int j = 0; j < N_TEXTURES; j++) {
			glBindTexture(config->tex_target(), _tex[j]);

			glTexParameteri(config->tex_target(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(config->tex_target(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(config->tex_target(), GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(config->tex_target(), GL_TEXTURE_WRAP_T, GL_CLAMP);

			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

			glTexImage2D(config->tex_target(), MIPMAP_LEVEL, config->tex_internal_format(),
			_vw, _vh, 0, GL_RGBA, GL_FLOAT, NULL);

			checkErrors();

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + j,
				config->tex_target(),
				_tex[j],
				MIPMAP_LEVEL);
		}

		checkErrors();

		CHECK_FRAMEBUFFER_STATUS();

		// FIXME: If no multisampling, don't create renderbuffers
		// If we already have any generated renderbuffers, delete them
		if (_rb[0] != 0)
			glDeleteRenderbuffersEXT(N_RENDERBUFFERS, _rb);

		// Generate our renderbuffers
		glGenRenderbuffersEXT(N_RENDERBUFFERS, _rb);

		// Just one renderbuffer for the moment, multisampled if supported
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _rb[_rb_multisampled]);
		if (config->supported_fbo_coverage_ms())	// Coverage multisampling
			glRenderbufferStorageMultisampleCoverageNV(
				GL_RENDERBUFFER_EXT,
				config->ms_coverage().color_samples,
				config->ms_coverage().coverage_samples,
				config->tex_internal_format(), _vw, _vh);
		else if (config->supported_fbo_ms())	// Multisampling
			glRenderbufferStorageMultisampleEXT(
				GL_RENDERBUFFER_EXT,
				config->ms_samples(),
				config->tex_internal_format(), _vw, _vh);
		else		// No multisampling
			glRenderbufferStorageEXT(
				GL_RENDERBUFFER_EXT,
				config->tex_internal_format(), _vw, _vh);

		// Attach it to the multisampled FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbuf[_fbo_multisampled]);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, _rb[_rb_multisampled]);

		// Default drawing location: FBO with multisampled renderbuffer
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _rb_multisampled);

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

void
Renderer_OpenGL::reset()
{
	// Clear buffers
	memset(_buffer, 0, _vw * _vh * sizeof(surface_type));

	// Clear textures
	// TODO: I think it's enough to clear the result texture, because the others will be overwritten
	for (unsigned int j = 0; j < N_TEXTURES; j++)
		transfer_data(_buffer, j);

	// Use the accumulated rotation
	apply_trans();
}

void
Renderer_OpenGL::pre_rotate(const GLfloat angle, const Point origin)
{
	add_trans(ROTATE_TRANS);

	GLfloat ang = angle;
	if (ang < 0)
		ang += 360;
	_rotation = fmod(_rotation + ang, 360);

	// FIXME: Maybe we've to accumulate that too
	_rotation_origin = origin;
}

void
Renderer_OpenGL::post_rotate(const GLfloat angle, const Point origin)
{
	glRotatef(-angle, 0.0, 0.0, 1.0);
}

void
Renderer_OpenGL::pre_translate(const Point origin)
{
	add_trans(TRANSLATE_TRANS);

	_translation += origin;
}

void
Renderer_OpenGL::post_translate(const Point origin)
{
	glTranslatef(-origin[0], -origin[1], 0.0);
}

void
Renderer_OpenGL::pre_zoom(const GLfloat zoom, const Point origin)
{
	add_trans(ZOOM_TRANS);

	_scale += zoom;

	// FIXME: Maybe we've to accumulate that too
	_scale_origin = origin;
}

void
Renderer_OpenGL::post_zoom(const GLfloat zoom, const Point origin)
{
	glScalef(zoom, zoom, 1.0);
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
	// FIXME: If no multisampling, don't blit renderbuffers, but draw directly
	// Retrieve multisampled renderbuffer
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _fbuf[_fbo_multisampled]);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _fbuf[_fbo_normal]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _tex_write);
	// Filter doesn't matter, because we're using the same size for both framebuffers
	glBlitFramebufferEXT(0, 0, _vw, _vh, 0, 0, _vw, _vh, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Copy current result to intermediate buffer
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _fbuf[_fbo_normal]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _tex_buffer);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + _tex_result);
	glBlitFramebufferEXT(0, 0, _vw, _vh, 0, 0, _vw, _vh, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Draw result into the result texture
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _tex_result);

	// Select the appropiate program
	glUseProgram(_program[blend_method]);
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _tex[_tex_write]);
	// Get location of the texture sampler (TODO: Store location on an array)
	GLuint source = glGetUniformLocation(_program[blend_method], "source");
	glUniform1i(source, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _tex[_tex_buffer]);
	// Get location of the texture sampler (TODO: Store location on an array)
	GLuint dest = glGetUniformLocation(_program[blend_method], "dest");
	glUniform1i(dest, 1);

	// Render to do the copy
	// Reset the modelview matrix to get rid of transformations
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0, 0); glVertex2f(_tl[0], _tl[1]);
		glTexCoord2f(1, 0); glVertex2f(_br[0], _tl[1]);
		glTexCoord2f(1, 1); glVertex2f(_br[0], _br[1]);
		glTexCoord2f(0, 1); glVertex2f(_tl[0], _br[1]);
	}
	glEnd();
	glPopMatrix();

	// Disable program
	glUseProgram(0);
	//glDisable(GL_TEXTURE_2D);

	// Restore writting texture
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _fbuf[_fbo_multisampled]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _rb_multisampled);
	// Clear previous data
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

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
	vector<GLdouble> &vec = _points.back();
	for (unsigned int j = 0; j < vec.size(); j += 3)
		gluTessVertex(_tess, reinterpret_cast<GLdouble*>(&vec[j]), reinterpret_cast<void*>(&vec[j]));

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
	vector<GLdouble> &vec = _points.back();
	vec.push_back(x);
	vec.push_back(y);
	vec.push_back(z);
}

void
Renderer_OpenGL::add_particle_vertex(const Point origin)
{
	glVertex3f(origin[0], origin[1], 0);
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

	// Change FBO to get result
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbuf[_fbo_normal]);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + _tex_result);
	glReadPixels(0, 0, _vw, _vh, format, type, _buffer);
	// Return to the multisampled FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbuf[_fbo_multisampled]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + _rb_multisampled);

	checkErrors();

	return _buffer;
}
