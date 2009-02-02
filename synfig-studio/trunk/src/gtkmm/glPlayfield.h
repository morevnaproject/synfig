#ifndef __GLPLAYFIELD_H_
#define __GLPLAYFIELD_H_
#pragma once

#include <GL/gl.h>
#include <GL/glu.h>
#include <FTGL/ftgl.h>
#include <vector>
#include <iostream>

class glPlayfield
{
	private:
		static const int NEAR = 1;
		static const int FAR = 100;
		int _width, _height;
		int _scrollX, _scrollY;
		GLint _lastPrecision;
		FTTextureFont _font;
	public:
		glPlayfield();
		~glPlayfield();
	public:
		void initializeGL();
		void resizeGL(int width, int height);
		void clearGL();
		inline void setColorGL(const GLfloat r, const GLfloat g, const GLfloat b) { glColor3f(r, g, b); }
		inline void setColorGL(const GLdouble r, const GLdouble g, const GLdouble b) { glColor3d(r, g, b); }
		inline void setColorGL(const GLubyte r, const GLubyte g, const GLubyte b) { glColor3ub(r, g, b); }
		inline void setColorGL(const GLushort r, const GLushort g, const GLushort b) { glColor3us(r, g, b); }
		inline void setColorGL(const GLuint rgb) { glColor3ub((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF); }
		inline void setFunctionGL(const unsigned int op) { if (op == GL_COPY) glDisable(GL_COLOR_LOGIC_OP); else { glEnable(GL_COLOR_LOGIC_OP); glLogicOp(op); } }
		inline void setLineWidthGL(const GLfloat width) { glLineWidth(width); }
		inline void setPointSizeGL(const GLfloat size) { glPointSize(size); }
		inline void setFontSize(const unsigned int size) { _font.FaceSize(size); }

		inline void enableStippling(int size = 1) { glEnable(GL_LINE_STIPPLE); glLineStipple(size, 0x0F0F); }
		inline void disableStippling() { glDisable(GL_LINE_STIPPLE); }
		inline void startPrimitive(const int mode) { glBegin(mode); }
		inline void endPrimitive() { glEnd(); }	

		void drawPlayfield(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2);

		void drawAnimateBorder();
		void drawPoints(std::vector<GLfloat> points);
		inline void addPoint(const GLfloat x, const GLfloat y) { glVertex2f(x, y); }
		inline void drawPoint(const GLfloat x, const GLfloat y) { glBegin(GL_POINTS); glVertex2f(x, y); glEnd(); }
		void drawLine(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2, int precision = 1);
		void drawLines(std::vector<GLfloat> points, const unsigned int mode = GL_LINE_STRIP);
		void prepareBezier(std::vector<GLfloat> points, const GLint precision);
		void drawBezier(const unsigned int mode = GL_LINE);
		void drawCircle(const GLfloat cx, const GLfloat cy, const GLfloat r, int precision = 0);
		void drawArc(const GLfloat cx, const GLfloat cy, const GLfloat r, const GLfloat start_angle, const GLfloat arc_angle, int precision = 0); 
		void drawRectangle(const GLfloat x1, const GLfloat y1, const GLfloat x2, const GLfloat y2);
		inline void drawText(const char *str, const FTGL_DOUBLE x, const FTGL_DOUBLE y) { glPushMatrix(); glScalef(1, -1, 1); _font.Render(str, -1, FTPoint(x, -y)); glPopMatrix(); }

		inline void scroll(int x, int y) { _scrollX += x; _scrollY += y; }
};

#endif // __GLPLAYFIELD_H_
