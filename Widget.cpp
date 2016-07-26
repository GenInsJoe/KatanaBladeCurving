//  Widgets.cpp - Buttons, Sliders, Movers, Typescripts
//  Copyright (c) Jules Bloomenthal, 2012
//  All rights reserved

#include "Draw.h"
#include "Widget.h"
#include "freeglut.h"

static float offWhtV = 240.f/255.f, ltGryV = 227.f/255.f, mdGryV = 160.f/255.f, dkGryV = 105.f/255.f;
static float blk[] = {0, 0, 0}, wht[] = {1, 1, 1}, offWht[] = {offWhtV, offWhtV, offWhtV},
	         ltGry[] = {ltGryV, ltGryV, ltGryV},  mdGry[] = {mdGryV, mdGryV, mdGryV},
	         dkGry[] = {dkGryV, dkGryV, dkGryV};

//****** Support

void DrawDepressedBox(int x, int y, int w, int h, float *color = NULL);

void DrawUnpressedBox(int x, int y, int w, int h, float *color = NULL);

//***** Titles

Header::Header(int x1, int y1, int x2, int y2, char *title, float *textColor)
{
	Init(x1, y1, x2, y2, title, textColor);
};

bool Header::Within(int x, int y)
{
	return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

void Header::Draw()
{
	int strwid = 9*(1+strlen(title));
	int w = x2-x1+1, h = y2-y1+1;
	if (title && *title) {
		int margin = (w-strwid)/2;
		glColor3fv(textColor);
		Text(x1+margin+4, y2-4, title);
		Rect(x1, y2, margin, 1, dkGry);         // left-top
		Rect(x2-margin, y2, margin, 1, dkGry);  // right-top
		Rect(x1+1, y2-1, margin-1, 1, wht);      // left-top
		Rect(x2-margin, y2-1, margin-1, 1, wht); // right-top
	}
	else {
		Rect(x1, y2, w, 1, dkGry);
		Rect(x1+1, y2-1, w-2, 1, wht);
	}
	Rect(x1, y1, 1, h, dkGry);                  // left
	Rect(x2-1, y1+1, 1, h-1, dkGry);            // right
	Rect(x1, y1+1, w, 1, dkGry);                // bottom
	Rect(x1+1, y1+2, 1, h-3, wht);               // left
	Rect(x2, y1, 1, h, wht);                     // right
	Rect(x1, y1, w, 1, wht);                     // bottom
}

void Header::Init(int _x1, int _y1, int _x2, int _y2, char *_title, float *_textColor) {
	x1 = _x1;
	y1 = _y1;
	x2 = _x2;
	y2 = _y2;
	if (_title)
		strcpy(title, _title);
	if (_textColor)
		memcpy(textColor, _textColor, 3*sizeof(float));
}

//****** Buttons

Button::Button(int x, int y, int w, int h, float *bgrndCol, float *txtCol) {
	InitRectangle(x, y, w, h, bgrndCol, txtCol);
}

Button::Button(int x, int y, float radius, float *bgrndCol, float *txtCol) {
	InitDot(x, y, radius, bgrndCol, txtCol);
}

Button::Button(int x, int y, int size, float *txtCol) {
	InitCheckbox(x, y, size, txtCol);
}

void Button::InitButton(Button *b, float *bgrndCol, float *txtCol) {
	for (int i = 0; i < 3; i++) {
		b->backgroundColor[i] = bgrndCol? bgrndCol[i] : 1;
		b->textColor[i] = txtCol? txtCol[i] : 1;
	}
	b->textWidth = -1;
	b->font = GLUT_BITMAP_9_BY_15;
}

void Button::InitCheckbox(int x, int y, int size, float *txtCol) {
	this->x = x;
	this->y = y;
	w = h = size;
	winW = -1;
	type = B_Checkbox;
	InitButton(this, NULL, txtCol);
}

void Button::InitDot(int ax, int ay, float aradius, float *bgrndCol, float *txtCol) {
	textWidth = -1;
	font = GLUT_BITMAP_9_BY_15;
	x = ax;
	y = ay;
	radius = aradius;
	type = B_Dot;
	winW = -1;
	InitButton(this, bgrndCol, txtCol);
}

void Button::InitRectangle(int ax, int ay, int aw, int ah, float *bgrndCol, float *txtCol) {
	textWidth = -1;
	font = GLUT_BITMAP_9_BY_15;
	x = ax;
	y = ay;
	w = aw;
	h = ah;
	type = B_Rectangle;
	winW = -1;
	InitButton(this, bgrndCol, txtCol);
}

void Button::InitTube(int ax, int ay, int aw, float aradius, float *bgrndCol, float *txtCol) {
	textWidth = -1;
	font = GLUT_BITMAP_9_BY_15;
	x = ax;
	y = ay;
	w = aw;
	radius = aradius;
	type = B_Tube;
	winW = -1;
	InitButton(this, bgrndCol, txtCol);
}

void CenterText(int x, int y, char *text, int buttonWidth) {
	int xoff = (buttonWidth-9*strlen(text))/2;
	Text(x+xoff, y, text);
}

void Button::ShowName(char *name, float *color) {
	float yDotOffset = font == GLUT_BITMAP_TIMES_ROMAN_24? 6.5f : 4.f;
	float yRectOffset = font == GLUT_BITMAP_TIMES_ROMAN_24? 4.f : 6.f;
	int xpos = type == B_Tube? x+13 : type == B_Rectangle? x+5 : type == B_Checkbox? x+w : x+(int)radius+4;
	int ypos = type == B_Tube || type == B_Dot || type == B_Checkbox? y-(int)yDotOffset : y+(int)yRectOffset;
	int nchars = strlen(name);
	bool singleLine = 9*nchars <= w; // assume 9by15 font
	if (singleLine && h > 30)
		ypos += (h-20)/2;
	glColor3fv(color);
	if (type == B_Checkbox || type == B_Dot) {
		char *cr = strchr(name, '\n'), buf[100];
		int len = cr? cr-name : 0;
		if (len) {
			strncpy(buf, name, len);
			buf[len] = 0;
			Text(xpos, ypos+9, buf);
			Text(xpos, ypos-5, cr+1);
			if (textWidth < 0) {
				int line1 = len, line2 = nchars-len-1;
				textWidth = 9*(line1 > line2? line1 : line2);
			}
		}
		else
			Text(xpos, ypos, name);
	}
	else if (type == B_Rectangle && singleLine)
	    CenterText(x, ypos, name, w);
	else {
		char *sp = strchr(name, ' '), buf[100];
		int len = sp? sp-name : w/9;
		strncpy(buf, name, len);
		buf[len] = 0;
		if (sp != NULL) {
			// multi-line
			CenterText(x, ypos+15, buf, w);
			if (this->h >= 30) {
				char *s = name+len;
				while (*s == ' ')
					s++;
				CenterText(x, ypos, s, w);
			}
		}
		else
			CenterText(x, ypos+45, name, w);
	}
	if (textWidth < 0)
		textWidth = 9*strlen(name);
}

void DrawDepressedBox(int x, int y, int w, int h, float *col) {
	if (col)
		Rect(x, y, w, h, col);
	else
		for (int i = 0; i < w; i++)
			for (int j = 0; j < h; j++)
				if ((i+j)%2)
					Rect(x+i, y+j, 1, 1, wht);
	Rect(x, y, w, 1, wht);
	Rect(x+w-1, y, 1, h, wht);
	Rect(x+1, y+1, w-2, 1, ltGry);
	Rect(x+w-2, y+1, 1, h-2, ltGry);
	Rect(x, y+1, 1, h-1, dkGry);
	Rect(x, y+h-1, w-1, 1, dkGry);
	Rect(x+1, y+2, 1, h-3, mdGry);
	Rect(x+1, y+h-2, w-3, 1, mdGry);
}

void DrawUnpressedBox(int x, int y, int w, int h, float *col) {
	if (col)
		Rect(x, y, w, h, col);
	Rect(x, y, w, 1, dkGry);
	Rect(x+w-1, y+1, 1, h-1, dkGry);
	Rect(x+1, y+1, w-2, 1, mdGry);
	Rect(x+w-2, y+1, 1, h-2, mdGry);
	Rect(x, y+1, 1, h-1, wht);
	Rect(x, y+h-1, w-1, 1, wht);
	Rect(x+1, y+2, 1, h-3, ltGry);
	Rect(x+1, y+h-2, w-3, 1, ltGry);
}

void Button::Draw(char *name, float *statusColor) {
	// draw in clip (+/-1) space
	if (winW < 0) {
		winW = glutGet(GLUT_WINDOW_WIDTH);
		winH = glutGet(GLUT_WINDOW_HEIGHT);
	}

	if (type == B_Tube || type == B_Dot) {
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	}
	else {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
	glDisable(GL_DEPTH_TEST);

	if (type == B_Tube) {
		glLineWidth(2.f*radius);
		Line(x-(int)radius, y, x+(int)radius, y, backgroundColor, backgroundColor);
		glLineWidth(2.f*radius-2.f);
		if (statusColor)
			Line(x-(int)radius, y, x+(int)radius, y, statusColor, statusColor);
	}
	if (type == B_Rectangle) {
		Rect(x, y, w, h, offWht, true);
		statusColor? DrawDepressedBox(x, y, w, h) : DrawUnpressedBox(x, y, w, h);
	}
	if (type == B_Dot) {
		glEnable(GL_BLEND);
		glEnable(GL_POINT_SMOOTH);
		Disk(x, y, 2.f*radius, backgroundColor);
		if (statusColor)
			Disk(x, y, 1.3f*radius, statusColor);
	}
	if (type == B_Checkbox) {
		float ltGry[] = {.9f, .9f, .9f};
		float mdGry[] = {.6f, .6f, .6f};
		float dkGry[] = {.4f, .4f, .4f};
		int xx = x-5, yy = y-7, xEnd = xx+w, yEnd = yy+h;
		Rect(xx, yy, w+1, h+1, wht);
		Rect(xx, yy+1, 1, h-1, mdGry);
		Rect(xx, yEnd, w, 1, mdGry);
		Rect(xx+1, yy+2, 1, h-3, dkGry);
		Rect(xx+1, yEnd-1, w-2, 1, dkGry);
		Rect(xx+1, yy+1, w-2, 1, ltGry);
		Rect(xEnd-1, yy+1, 1, h-1, ltGry);
		if (statusColor) {
			int checkMark[7][7] = {0,0,1,0,0,0,0,
								   0,1,1,1,0,0,0,
								   1,1,1,1,1,0,0,
								   1,1,0,1,1,1,0,
								   1,0,0,0,1,1,1,
								   0,0,0,0,0,1,1,
								   0,0,0,0,0,0,1};
			for (int row = 0; row < 7; row++)
				for (int col = 0; col < 7; col++)
					if (checkMark[row][col] == 1) {
						int ix = xx+4+col, iy = yy+5+row;
						Rect(ix, iy, 1, 1, blk); // textColor); // blk);
					}
		}
	}
	if (name)
		ShowName(name, textColor);
}

void Button::Highlight() {
	Rect(x, y, w, h, wht, true, .5f);
	glLineWidth(1.f);
	float x1 = (float)x, x2 = x1+(float)w, y1 = (float)y, y2 = y1+(float)h;
	Line(x1+1, y1+1.5f, x2-1, y1+1.5f, blk, blk, 1);
	Line(x2-1.5f, y2-1, x2-1.5f, y1+1, blk, blk, 1);
}

bool Button::Hit(int ax, int ay) {
	if (type == B_Tube)
		return ax >= x-radius && ax < x+radius && ay >= y-radius && ay < y+radius;
	if (type == B_Rectangle)
	    return ax >= x && ax <= x+w && ay >= y && ay <= y+h;
	if (type == B_Dot) {
		int dx = x-ax, dy = y-ay, absdy = dy < 0? -dy : dy;
		return textWidth < 0?
			dx*dx+dy*dy < radius*radius :
			absdy < radius && ax >= x-radius && ax <= x+textWidth+10;
	}
	if (type == B_Checkbox)
		return  ax >= x-5 && ax <= (textWidth < 0? x-5+w : x-5+w+textWidth+7) && ay >= y-7 && ay <= y-7+w;
	return false;
}

//****** Sliders

Slider::Slider(int x, int y, int size, float min, float max, float init, Orientation o, float *col, bool displayInteger) :
	x(x), y(y), size(size), orientation(o), displayInteger(displayInteger) {
		SetRange(min, max, init);
		color[0] = color[1] = color[2] = 1;
		if (col)
			memcpy(color, col, 3*sizeof(float));
	    winW = -1;
		int off = orientation == Horizontal? x : y;
		loc = (off+(float)(init-min)/(max-min)*size);
}

void Slider::SetValue(float val) {
	int off = orientation == Horizontal? x : y;
    loc = (off+(float)(val-min)/(max-min)*size);
}		

void Slider::SetRange(float min, float max, float init) {
	this->min = min;
	this->max = max;
	int off = orientation == Horizontal? x : y;
    loc = (off+(float)((init-min)/(max-min))*size);
}	

void Slider::Draw(char *name, float *sliderColor) {

	float xPos, yPos;
	float *sCol = sliderColor? sliderColor : color;
	glLineWidth(2);
	int iloc = (int) loc;
	if (orientation == Horizontal) {
		Rect(x, y-10, size, 20, color, true, .5f);
		Line(x, y, x+size, y, color, color);
		Line(iloc, y-10, iloc, y+10, sCol, sCol);
		xPos = (2*(x+size+5))/(float)winW-1.f;
		yPos = (2*(y-5))/(float)winH-1.f;
	}
	else {
		float grays[] = {160, 105, 227, 255};
		for (int i = 0; i < 4; i++) {
			float g = grays[i]/255.f, col[] = {g, g, g};
			Rect(x-1+i, y, 1, size, col);
		}
		Rect(x-1, y, 4, 1, wht);
		Rect(x, y+1, 1, 1, ltGry);
		Rect(x-1, y+size-1, 3, 1, mdGry);
		// slider
		Rect(x-10, iloc-3, 20, 7, offWht);
		Rect(x-10, iloc-3, 20, 1, dkGry);
		Rect(x+10, iloc-3, 1, 7, dkGry);
		Rect(x-10, iloc-2, 1, 6, wht);
		Rect(x-10, iloc+3, 20, 1, wht);
		Rect(x-9, iloc-1, 1, 4, ltGry);
		Rect(x-9, iloc+2, 18, 1, ltGry);
		Rect(x-9, iloc-2, 18, 1, mdGry);
		Rect(x+9, iloc-2, 1, 5, mdGry);
		//Rect(x-16, y-22, 33, size+50, color, false, 1);
		//Line(x, y, x, y+size, color, color, 1);
		//Line(x-10, iloc, x+10, iloc, sCol, sCol, 1);
		xPos = (2*(x+2))/(float)winW-1.f;
		yPos = (2*(y+size-0))/(float)winH-1.f;
	}
	if (name) {
		if (winW == -1) {
			winW = glutGet(GLUT_WINDOW_WIDTH);
			winH = glutGet(GLUT_WINDOW_HEIGHT);
		}
		char buf[100];
		glColor3fv(color);
		double val = GetValue();
		if (orientation == Horizontal) {
			sprintf(buf, "%s: %3.1f", name, GetValue());
			glRasterPos2f(xPos, yPos);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (unsigned char*) buf);
		}
		else {
			sprintf(buf, val >= 1? "%3.2f" : "%3.3f", val);
			char *start = val < 1? buf+1 : buf; // skip leading zero
			int wName = 1+glutBitmapLength(GLUT_BITMAP_HELVETICA_12, (unsigned char *) name);
			int wBuf = glutBitmapLength(GLUT_BITMAP_HELVETICA_12, (unsigned char *) start);
			glRasterPos2f(xPos-(float)wName/(float)winW, yPos+.02f);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (unsigned char *) name);
			glRasterPos2f(xPos-(float)wBuf/(float)winW, yPos-2.f*(float)size/winH-.03f);
			glutBitmapString(GLUT_BITMAP_HELVETICA_12, (unsigned char *) start);
		}
	}
}

float Slider::GetValue() {
    return orientation == Horizontal?
		min+((float)(loc-x)/size)*(max-min) :
		min+((float)(loc-y)/size)*(max-min);
}

bool Slider::Hit(int xx, int yy) {
    return orientation == Horizontal?
		xx >= x && xx <= x+size && yy >= y-10 && yy <= y+10 :
		xx >= x-16 && xx <= x+16 && yy >= y-32 && yy <= y+size+27;
}

void Slider::Mouse(int ax, int ay) {
    // bool hit = orientation == Horizontal?
	// 	x >= this->x && x <= this->x+size && y >= this->y-10 && y <= this->y+10 :
	// 	x >= this->x-10 && x <= this->x+10 && y >= this->y && y <= this->y+size;
	// if (hit)
    loc = (float) (orientation == Horizontal? ax : ay);
	int min = orientation == Horizontal? x : y;
	loc = loc < min? min : loc > min+size? min+size : loc;
}

bool Slider::HitMouse(int x, int y) {
	if (Hit(x, y)) {
		Mouse(x, y);
		return true;
	}
	return false;
}

//****** Movers

float DotProduct(float a[], float b[])
{
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

void Mover::SetPlane(int x, int y, mat4 &modelview, mat4 &persp)
{
	float p1[3], p2[3];
    ScreenLine((float) x, (float) y, modelview, persp, p1, p2);
        // get two points that transform to pixel x,y
	for (int i = 0; i < 3; i++)
        plane[i] = p2[i]-p1[i];
        // set plane normal to that of line p1p2
    // Normalize(plane); // unneeded
	float a = plane[0]*point->x;
	float b = plane[1]*point->y;
	float c = plane[1]*point->z;
	plane[3] = -plane[0]*point->x-plane[1]*point->y-plane[2]*point->z;
        // pass plane through point
}

void Mover::Pick(vec3 *p, int x, int y, mat4 &modelview, mat4 &persp)
{
	point = p;
    SetPlane(x, y, modelview, persp);
}

void Mover::UnPick()
{
	point = NULL;
}

bool Mover::IsPicked()
{
	return point != NULL;
}

void Mover::Drag(int x, int y, mat4 &modelview, mat4 &persp)
{
	if (point) {
		float p1[3], p2[3], axis[3];
		ScreenLine(static_cast<float>(x), static_cast<float>(y), modelview, persp, p1, p2);
			// get two points that transform to pixel x,y
		for (int i = 0; i < 3; i++)
			axis[i] = p2[i]-p1[i];
			// direction of line through p1
		float pdDot = DotProduct(axis, plane);
			// project onto plane normal
		float a = (-plane[3]-DotProduct(p1, plane))/pdDot;
		for (int j = 0; j < 3; j++)
			(*point)[j] = p1[j]+a*axis[j];
			// intersection of line with plane
	}
}

//****** Typescripts

Typescript::Typescript(int x, int y, int w, int h, char *text) : x(x), y(y), w(w), h(h)
{
    Clear();
	if (text)
		SetText(text);
}

void Typescript::Clear()
{
	text[0] = 0;
}

void Typescript::AddChar(char c)
{
	int len = strlen(text);
	if (c == 8) { // backspace
		if (len > 0)
			text[len-1] = 0;
	}
	else
		if (len < maxTypescript-1) {
			text[len] = c;
			text[len+1] = 0;
		}
}

void Typescript::SetText(const char *c)
{
	strcpy(text, c);
}

bool Typescript::Hit(int _x, int _y)
{
    return _x >= x && _x < x+w && _y >= y && _y < y+h;
}

void Typescript::Draw(float *color)
{
	DrawDepressedBox(x, y, w, h);
    glColor3fv(color);
    Text(x+5, y+6, text);
}

//****** Miscellany

bool Newer(SYSTEMTIME &t1, SYSTEMTIME &t2)
{
	// return true if t1 is newer than t2
	return t2.wYear         < t1.wYear?         true : t2.wYear         > t1.wYear?         false :
		   t2.wMonth        < t1.wMonth?        true : t2.wMonth        > t1.wMonth?        false :
		   t2.wDay          < t1.wDay?          true : t2.wDay          > t1.wDay?          false :
		   t2.wHour         < t1.wHour?         true : t2.wHour         > t1.wHour?         false :
		   t2.wMinute       < t1.wMinute?       true : t2.wMinute       > t1.wMinute?       false :
		   t2.wSecond       < t1.wSecond?       true : t2.wSecond       > t1.wSecond?       false :
		   t2.wMilliseconds < t1.wMilliseconds? true : t2.wMilliseconds > t1.wMilliseconds? false :
		   true;
}

void NewestFilename(char *search, string &name)
{
	wchar_t wbuf[1000];
	mbstowcs(wbuf, search, 999);
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(wbuf, &fd);
	if (h != INVALID_HANDLE_VALUE) {
		SYSTEMTIME earliest, time;
		bool once = false;
		do {
			char buf[1000];
			wcstombs(buf, fd.cFileName, 999);
			FileTimeToSystemTime(&fd.ftLastWriteTime, &time);
			if (!once || Newer(time, earliest)) {
				earliest = time;
				name = string(buf);
			}
			once = true;
		} while (FindNextFile(h, &fd));
		FindClose(h);
	}
}

void GetFilenames(char *search, vector<string> &names)
{
	wchar_t wbuf[1000];
	mbstowcs(wbuf, search, 999);
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(wbuf, &fd);
	names.resize(0);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			char buf[1000];
			wcstombs(buf, fd.cFileName, 999);
			names.push_back(string(buf));
		} while (FindNextFile(h, &fd));
		FindClose(h);
	}
}

#include <CommDlg.h>
bool ChooseFile(TypeIO type, string startDir, string &filename, string title) {
	// if no file selected, return false; else, set filename, return true
	// if FileSave, filename can contain suggested name
	// **** really, lpstrFilter should be passed in
	const int MAX = 200;
	wchar_t wname[MAX], wdir[MAX], wtitle[MAX];
    OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; // FindWindow(NULL, (LPCWSTR) L"App Title");
		                  // should open dialog box if program fullscreen, without changing mode; alas, no
	ofn.lpstrFile = wname;
	ofn.lpstrFile[0] = '\0';
	if (type == FileSave)
		mbstowcs(wname, filename.c_str(), MAX);
	ofn.nMaxFile = MAX;
	ofn.lpstrFilter = type == FileRead? L"All\0*.bob;*.stl;*.obj;*.bez\0\0" : L"All\0*.stl\0\0"; // ****
	ofn.nFilterIndex = 1; // 2;
	mbstowcs(wdir, startDir.c_str(), MAX);
	ofn.lpstrInitialDir = wdir;
	mbstowcs(wtitle, title.c_str(), MAX);
	ofn.lpstrFileTitle = wtitle;
	ofn.nMaxFileTitle = 200;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    BOOL ok = type == FileRead? GetOpenFileNameW(&ofn) : GetSaveFileNameW(&ofn);
    if (ok) {
		char name[200];
		wcstombs(name, ofn.lpstrFile, MAX);
		filename = string(name);
	}
    return ok == TRUE;
}
