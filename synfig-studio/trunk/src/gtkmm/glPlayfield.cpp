#include "glPlayfield.h"
#include "app.h"
#include <iostream>
#include <cmath>

glPlayfield::glPlayfield():
	_width(0),
	_height(0),
	_scrollX(0),
	_scrollY(0),
	_lastPrecision(0),
	_font("LiberationSans-Regular.ttf") {

	// If something went wrong, bail out.
	if(_font.Error()) {
		synfig::error("Cannot find font or font error");
		throw;
	}
}

glPlayfield::~glPlayfield() {
}

void glPlayfield::initializeGL() {
	//glShadeModel(GL_SMOOTH);
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

	glClearColor(0.5, 0.5, 0.5, 1);

	// Smooth lines
	//glEnable(GL_LINE_SMOOTH);
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Round points
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(8.0f);

	_font.FaceSize(14);
}

void glPlayfield::resizeGL(int width, int height) {
	/*if (width % 2)
		width = width + 1;
	if (height% 2)
		height = height + 1;*/

	// Store new values
	_width = width;
	_height = height;

	/*glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(-width/2, width/2, height/2, -height/2);
	glOrtho(0, width, height, 0, NEAR, FAR);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(2.0f / (float)width, -2.0f / (float)height, 1.0f);
	glTranslatef(-((float)width / 2.0f), -((float)height / 2.0f), 0.0f);
	glViewport(0, 0, width, height); /* viewport size in pixels */

	// Translate so we draw in the middle of the pixel
	//glTranslatef(/*_scrollX + */0.5, /*_scrollY + */0.5, 0);
}

void glPlayfield::clearGL() {
	glClear(GL_COLOR_BUFFER_BIT);
}

void glPlayfield::drawAnimateBorder() {
	// FIXME: Rectangle sometimes doesn't have the right size
	const float w2 = _width / 2.0f, h2 = _height / 2.0f;
	glBegin(GL_LINE_LOOP);
	glVertex2f(-w2 - _scrollX, -h2 - _scrollY);
	glVertex2f(w2 - 1 - _scrollX, -h2 - _scrollY);
	glVertex2f(w2 - 1 - _scrollX, h2 - 1 - _scrollY);
	glVertex2f(-w2 - _scrollX, h2 - 1 - _scrollY);
	glEnd();
}

void glPlayfield::drawLine(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2, int precision) {
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void glPlayfield::drawLines(std::vector<GLfloat> points, const unsigned int mode) {
	glEnableClientState(GL_VERTEX_ARRAY);
	// Yes, vector elements are ALWAYS contiguous
	glVertexPointer(3, GL_FLOAT, 0, &points[0]);
	glDrawArrays(mode, 0, points.size() / 3);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void glPlayfield::prepareBezier(std::vector<GLfloat> points, const GLint precision) {
	glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, points.size() / 3, &points[0]);
	glEnable(GL_MAP1_VERTEX_3);
	_lastPrecision = precision;
	glMapGrid1f(precision, 0.0, 1.0);
}

void glPlayfield::drawBezier(const unsigned int mode) {
	if (mode == GL_LINE)
		glEvalMesh1(mode, 0, _lastPrecision);
	else {
		glBegin(mode);
		for (GLfloat j = 0; j <= _lastPrecision; j += 1)
			glEvalCoord1f(j / _lastPrecision);
		glEnd();
	}
}

void glPlayfield::drawCircle(const GLfloat cx, const GLfloat cy, const GLfloat r, int precision) {
	// From http://slabode.exofire.net/circle_draw.shtml
	if (precision <= 0)
		precision = int(2.0f * M_PI / (acosf(1 - 0.25 / r)));
	float theta = 2 * M_PI / float(precision); 
	float tangetial_factor = tanf(theta);//calculate the tangential factor 

	float radial_factor = cosf(theta);//calculate the radial factor 

	float x = r;//we start at angle = 0 

	float y = 0; 

	// FIXME: USe a vertex array or buffer
	glBegin(GL_LINE_LOOP); 
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

void glPlayfield::drawArc(const GLfloat cx, const GLfloat cy, const GLfloat r, const GLfloat start_angle, const GLfloat arc_angle, int precision) { 
	// From http://slabode.exofire.net/circle_draw.shtml
	if (precision <= 0)
		precision = int(2.0f * M_PI / (acosf(1 - 0.25 / r)));
	float theta = arc_angle / float(precision - 1);//theta is now calculated from the arc angle instead, the - 1 bit comes from the fact that the arc is open

	float tangetial_factor = tanf(theta);

	float radial_factor = cosf(theta);


	float x = r * cosf(start_angle);//we now start at the start angle
	float y = r * sinf(start_angle); 

	glBegin(GL_LINE_STRIP);//since the arc is not a closed curve, this is a strip now
	for(int j = 0; j < precision; j++)
	{ 
		glVertex2f(x + cx, y + cy);

		float tx = -y; 
		float ty = x; 

		x += tx * tangetial_factor; 
		y += ty * tangetial_factor; 

		x *= radial_factor; 
		y *= radial_factor; 
	} 
	glEnd(); 
}

void glPlayfield::drawRectangle(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2) {
	glBegin(GL_LINE_LOOP);
	glVertex2f(x1, y1);
	glVertex2f(x2, y1);
	glVertex2f(x2, y2);
	glVertex2f(x1, y2);
	glEnd();
}

