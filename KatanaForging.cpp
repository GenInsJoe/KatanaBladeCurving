// Joe.cpp - multiple patches
// Copyright (c) Joe Bjork, 2014
// All rights reserved

#include "glew.h"
#include "freeglut.h"
#include "Draw.h"
#include "Patch.h"
#include "Widget.h"
#include "mat.h"

// Application Data

// display
mat4		modelview, persp, fullview;
bool    	viewControlMesh = true, viewShadedPatch = true, viewLinedPatch = false, viewCurve = false;
float		blk[] = {0, 0, 0}, wht[] = {1, 1, 1};

// widgets
Button		viewControlMeshBut(30, 20, 18, wht);
Button		viewShadedPatchBut(30, 45, 18, wht);
Button		viewLinedPatchBut(30, 70, 18, wht);
Button		viewCurveBut(30, 95, 18, wht);
Slider		patchRes(200, 20, 62, 2, 40, 10, Slider::Horizontal, wht);
Slider		curveyness(500, 20, 62, .01f, .3, .05f, Slider::Vertical, wht, true);
Mover		ptMover;

// patches
int				res = 20;							// res*res vertices
const int		npatches = 12;
const int		nTpatches = 6;						// subset of npatches for the ones that are triangular
float			s = 2.f;							// scale from base points
Patch			patches[npatches];

// interaction
int			xMouseDown, yMouseDown; // for each mouse down, need start point
vec2		rotOld, rotNew;			// previous, current rotations
mat4		rotM;				    // MouseDrag sets, Display uses
bool		cameraDown = false;

// Curvature Correction
void CC(){
	float movex = curveyness.GetValue()* s, movey = 2*curveyness.GetValue()*s;
	for (int i = 0; i < npatches; i++){
		for (int j = 0; j < 16; j++){
			vec3 temp = patches[i].pts[j / 4][j % 4].origPoint;
			float xResist = patches[i].pts[j / 4][j % 4].xresis;
			float yResist = patches[i].pts[j / 4][j % 4].yresis;
			temp = vec3(temp.x - (movex*xResist), temp.y + (movey*yResist), temp.z);
			patches[i].pts[j / 4][j % 4].point = temp;
		}
		patches[i].SetVertices();
	}
}

// resets the control points to thier original positions
void reset(){
	for (int i = 0; i < npatches; i++){
		for (int j = 0; j < 16; j++){
			patches[i].pts[j / 4][j % 4].point = patches[i].pts[j / 4][j % 4].origPoint;
		}
		patches[i].SetVertices();
	}
}
// Display

void Display() {
    // background, blending, zbuffer
    glClearColor(.2f, .2f, .2f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
	// update matrices
	modelview = Translate(0, 0, -5)*rotM;
	float fov = 15, nearPlane = -.001f, farPlane = -500;
	float aspect = (float) glutGet(GLUT_WINDOW_WIDTH) / (float) glutGet(GLUT_WINDOW_HEIGHT);
	persp = Perspective(fov, aspect, nearPlane, farPlane);
	fullview = persp*modelview;
	// draw patch
	for (int i = 0; i < npatches; i++) {
		Patch &p = patches[i];
		p.UseShader(modelview, persp);
		if (viewShadedPatch)
			p.Shade(modelview, persp, vec3(1, .7f, 0), vec3(.75f, .75f, .75f)); 
		if (viewLinedPatch)
			p.Draw(modelview, persp, vec3(0, 1, 1));
		if (viewControlMesh)
			p.DrawControlMesh(persp*modelview, vec3(0, .5f, 0), vec3(1, 0, 0));
	}
	// draw butttons in 2D screen space
	UseDrawShader(ScreenMode());
	viewControlMeshBut.Draw("control mesh", viewControlMesh? blk : NULL);
	viewShadedPatchBut.Draw("shaded", viewShadedPatch? blk : NULL);
	viewLinedPatchBut.Draw("lines", viewLinedPatch? blk : NULL);
	viewCurveBut.Draw("enable curve", viewCurve? blk : NULL);
	curveyness.Draw("Curve Strength", blk);
	glFlush();
}

// Mouse

vec3 *PickPoint(int x, int y, bool rightButton) {
	vec3 *ret = NULL;
	float dsqmin = 100;
	for (int i = 0; i < npatches; i++) {
		for (int k = 0; k < 16; k++) {
			vec3 *pt = &patches[i].pts[k/4][k%4].point;
			float dsq = ScreenDistSq(x, y, *pt, fullview, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
			if (dsq < dsqmin) {
				dsqmin = dsq;
				ret = pt;
			}
		}
	}
	return ret;
}

void MouseButton(int butn, int state, int x, int y) {
    y = glutGet(GLUT_WINDOW_HEIGHT)-y; // invert y for upward-increasing screen space
    if (state == GLUT_UP) {
		if (cameraDown)
			rotOld = rotNew;
		else if (ptMover.IsPicked())
			ptMover.UnPick();
		else if (viewControlMeshBut.Hit(x, y))
			viewControlMesh = !viewControlMesh;
		else if (viewLinedPatchBut.Hit(x, y))
			viewLinedPatch = !viewLinedPatch;
		else if (viewShadedPatchBut.Hit(x, y))
			viewShadedPatch = !viewShadedPatch;
		else if (viewCurveBut.Hit(x, y)){
			viewCurve = !viewCurve;
			if (viewCurve)
				CC();
			else
				reset();
		}
		else if (curveyness.Hit(x, y))
			curveyness.Mouse(x, y);
	}
	cameraDown = false;
	if (state == GLUT_DOWN) {
		if (!viewControlMeshBut.Hit(x, y) &&
			!viewLinedPatchBut.Hit(x, y) &&
			!viewShadedPatchBut.Hit(x, y) &&
			!viewCurveBut.Hit(x, y) &&
			!curveyness.Hit(x, y)) {
				vec3 *pp = viewControlMesh? PickPoint(x, y, butn == GLUT_RIGHT_BUTTON) : NULL;
				bool curvePt = false;
				int temp = npatches - nTpatches;
				if (pp) {
					// pick or deselect control point
					if (butn == GLUT_LEFT_BUTTON)
						ptMover.Pick(pp, x, y, modelview, persp);
				}
				else {
					cameraDown = true;
					xMouseDown = x;
					yMouseDown = y;
				}
			}
	}
    glutPostRedisplay();
}

void MouseDrag(int x, int y) {
	y = glutGet(GLUT_WINDOW_HEIGHT) - y;
	if (ptMover.IsPicked()) {
		ptMover.Drag(x, y, modelview, persp);
		for (int i = 0; i < npatches; i++)
			patches[i].SetVertices();
	}else if(curveyness.Hit(x, y)){
		curveyness.Mouse(x, y);
		if (viewCurve)
			CC();
	}else if (cameraDown) {
		rotNew = rotOld+.3f*(vec2((float)(x-xMouseDown), (float)(y-yMouseDown)));
		rotM = RotateY(rotNew.x)*RotateX(rotNew.y);
	}
    glutPostRedisplay();
}

// Patches
vec3			cp[npatches - nTpatches][4];			// Corner points array (not including tip point)
vec3			tipPoint = vec3(.56f*s, .02f*s, 0.f);	// tip of sword

void Points(){ // hardcoded points
	float tness = .01f, tness2 = tness - (tness / 10), x0 = -.5f*s;
	float tipBladeC = tipPoint.x - (.06f*s), tipBladeMid = tipPoint.x - (.08f*s);
	float tipBladeTop = tipBladeC + (.005f*s);

	// base points
	cp[0][0] = vec3(x0, -.03f*s, 0.f);				// blade edge left end point
	cp[0][1] = vec3(tipBladeC, -.024f*s, 0.f);			// blade edge right end point
	cp[0][2] = vec3(x0, 0.f, tness*s);				// front second edge left end point
	cp[0][3] = vec3(tipBladeMid, 0.f, tness*s);			// front second edge right end point
	cp[1][2] = vec3(x0, .02f*s, tness2*s);			// front third edge left end point
	cp[1][3] = vec3(tipBladeC, .02f*s, tness2*s);		// front thrid edge right end point
	cp[2][2] = vec3(x0, .025f*s, 0.f);				// top edge left end point
	cp[2][3] = vec3(tipBladeTop, .025f*s, 0.f);			// top edge right end point
	cp[3][2] = vec3(x0, .02f*s, -1.f*tness2*s);		// back third edge left end point
	cp[3][3] = vec3(tipBladeC, .02f*s, -1.f*tness2*s);	// back third edge right end point
	cp[4][2] = vec3(x0, .0f*s, -1.f*tness*s);		// back second edge left end point
	cp[4][3] = vec3(tipBladeMid, .0f*s, -1.f*tness*s);	// back second edge right end point
	cp[5][2] = cp[0][0];
	cp[5][3] = cp[0][1];
	for (int i = 1; i < npatches - nTpatches; i++){
		cp[i][0] = cp[i - 1][2];
		cp[i][1] = cp[i - 1][3];
	}
}

void InitPatches(){
	int index = (npatches - nTpatches); 
	// initialize the rectangular patches
	for (int i = 0; i < index; i++){
		patches[i].Init(res, cp[i][0], cp[i][1], cp[i][2], cp[i][3]);
	}
	int j = index;
	for (int i = 0; i < nTpatches; i ++){
			patches[j++].Init(res, cp[i][1], tipPoint, cp[i][3], tipPoint);
	}

	// initilize original points
	for (int i = 0; i < npatches; i++){
		for (int j = 0; j < 16; j++){
			patches[i].pts[j / 4][j % 4].origPoint = patches[i].pts[j / 4][j % 4].point;
		}
	}
}

void setResis(){
	int temp = npatches - nTpatches;
	for (int i = 0; i < temp; i++){
		for (int j = 0; j < 16; j++){
			int jMod4 = j % 4;
			float curv = .05f*(jMod4*jMod4) - .02f*jMod4;
			patches[i].pts[j / 4][jMod4].xresis = curv;
			patches[i].pts[j / 4][jMod4].yresis = curv;
		}
	}
		

	for (int i = temp; i < npatches; i++){
		for (int j = 0; j < 4; j++){
			for (int k = 0; k < 4; k++){
				float curv = .01f*(k*k) - .005f*k + (.05f*9.f - .02f * 3);
				patches[i].pts[j][k].xresis = curv;
				patches[i].pts[j][k].yresis = curv;
			}
		}
	}
}

// Application
int main(int ac, char **av) {
    // init app window
    glutInit(&ac, av);
    glutInitWindowSize(1500, 1000);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Katana Blade");
	GLenum err = glewInit();
	if (err != GLEW_OK)
        printf("Error initializaing GLEW: %s\n", glewGetErrorString(err));
	// init patch
	Points();
	InitPatches();
	setResis();
	if (viewCurve)
		CC();
    // callbacks
    glutDisplayFunc(Display);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseDrag);
    glutMainLoop();
}
