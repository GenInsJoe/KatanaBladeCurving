/* ==================================
    Draw.h
    Collection of draw support routines
    Copyright (c) Jules Bloomenthal, 2014
    All rights reserved
    ==================================

    Contents
		OpenGL Errors
		Screen Operations
		Drawing Functions
			dash
			3D line drawing
			2D line drawing
			disks
			arrow
			text
			miscellany
*/

#ifndef DRAW_HDR
#define DRAW_HDR

#include "glew.h"
#include "freeglut.h"
#include "mat.h"


//***** OpenGL Errors

int Errors(char *buf);
	// set buf to list of errors, return # errors

void CheckGL_Errors();
	// print list of errors


//****** Screen Operations

mat4 ScreenMode(int width, int height);
	// create transformation matrix that maps from pixel space, (0,0)-(width,height)
	// to NDC (clip) space, (-1,-1)-(1,1)

mat4 ScreenMode();
	// as above (GLUT provides width, height)

void ScreenPoint(vec3 p, mat4 m, float &xscreen, float &yscreen, float *zscreen = NULL);

vec2 ScreenPoint(vec3 p, mat4 m, float *zscreen = NULL);

void ScreenLine(float xscreen, float yscreen, mat4 &modelview, mat4 &persp, float p1[], float p2[]);
    // compute 3D world space line, given by p1 and p2, that transforms
    // to a line perpendicular to the screen at (xscreen, yscreen)

float ScreenDistSq(int x, int y, vec3 p, mat4 m, int width, int height, float *zscreen = NULL);
	// return distance squared, in pixels, between screen point (x, y) and
	// point p transformed by view matrix m


//****** Drawing Functions

void UseDrawShader();

void UseDrawShader(mat4 viewMatrix);
	// invoke a shader specifically designed for the 3D draw routines

// dash
void DashOn(int factor = 1, int offset = 0);
void DashOff();

// 3D line drawing
void Line(vec3 &p1, vec3 &p2, vec3 &col, float opacity = 1, float width = 1);
void Line(vec3 &p1, vec3 &p2, vec3 &col1, vec3 &col2, float opacity = 1);
void Line(float *p1, float *p2, float *col1, float *col2, float opacity = 1);

// 2D line drawing
void Line(vec2 &p1, vec2 &p2, vec3 &col1, vec3 &col2, float opacity = 1);
void Line(float x1, float y1, float x2, float y2, float *col1, float *col2, float opacity = 1);
void Line(int x1, int y1, int x2, int y2, float *col1, float *col2, float opacity = 1);

// disks
void Disk(int x, int y, float radius, float *color);
void Disk(vec2 &p, float radius, vec3 &color);
void Disk(vec3 &p, float radius, vec3 &color);

// arrow
void Arrow(vec2 &base, vec2 &head, char *label, double headSize, vec3 &color);
void ArrowV(vec3 &base, vec3 &vec, mat4 &m, char *label, double headSize, vec3 &color);

// text
void Text(int x, int y, const char *text);
	// position null-terminated text at pixel (x, y)
void Text(vec3 &p, mat4 &m, char *text, vec3 &col);
void Text(vec3 &p, mat4 &m, vec3 &col, char *format, ...);

// miscellany
void Rect(int x, int y, int w, int h, float *col, bool solid = true, float opacity = 1);
void Circle(vec2 &p, float dia, vec3 &color);
void Cross(float p[], float s, float col[]);
void Asterisk(float p[], float s, float col[]);
	// draw an asterisk at 3D point p, with scale s and given color
void Crosshairs(vec2 &s, float radius, vec3 &color);
void Sun(vec2 &p, vec3 *flashColor = NULL);
void Stipple(int factor=1,int a=-1,int b=-1,int c=-1,int d=-1,int e=-1,
                          int f=-1,int g=-1,int h=-1,int i=-1,int j=-1,
                          int k=-1,int l=-1,int m=-1,int n=-1,int o=-1,int p=-1);
    // args a-p are bit positions; eg, glmStipple(1, 4) means turn on bit 4, others off

#endif
