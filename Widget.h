/*  =====================================
    Widget.h - buttons, sliders, etc
    Copyright (c) Jules Bloomenthal, 2012
    All rights reserved
    =====================================
	
    Contents
		Header        a titled border
		Button        a checkbox or push-button (dot, rectangle or tube)
		Slider        a vertical or horizontal slider
		Mover         drag a 3D point in space
		Typescript    user text input
		Miscellany    file names
*/

#ifndef WIDGET_HDR
#define WIDGET_HDR

#include <string>
#include <vector>
#include "mat.h"

using std::string;
using std::vector;

#pragma warning(disable:4786) // kill MS warning re long names, nested templates


//****** Headers: create a titled border

class Header
{
public:
	Header(int x1, int y1, int x2, int y2, char *title, float *textColor = NULL);
		// initialize new header boundary from (x1,y1) to (x2,y2)
	void Init(int x1, int y1, int x2, int y2, char *title, float *textColor = NULL);
		// initialize existing header, (x1,y1) to (x2,y2)
	bool Within(int x, int y);
		// is mouse (x,y) within header's boundary?
	void Draw();
private:
	int x1, y1, x2, y2;
	char title[100];
	float textColor[3];
};

	
//****** Buttons: create a checkbox or push-button (dot, rectangle or tube)

class Button
{
public:
	// dot button:
	Button(int x, int y, float radius, float *backgroundColor = NULL, float *textColor = NULL);
	// rectangle button:
    Button(int x, int y, int w, int h, float *backgroundColor = NULL, float *textColor = NULL);
	// checkbox:
	Button(int x, int y, int size, float *textColor = NULL);
	void InitCheckbox(int x, int y, int size = 15, float *textColor = NULL);
	void InitRectangle(int x, int y, int w, int h, float *backgroundColor = NULL, float *textColor = NULL);
	void InitDot(int x, int y, float radius, float *backgroundColor = NULL, float *textColor = NULL);
	void InitTube(int x, int y, int w, float radius, float *backgroundColor = NULL, float *textColor = NULL);
	void ShowName(char *name, float *color);
	    // sets textWidth, which affects Hit
    void Draw(char *name, float *statusColor = NULL);
		// if checkbox, statusColor NULL means unchecked
		// if rectangle, statusColor NULL means unpressed
	void Highlight();
		// draw rectangular highlight
    bool Hit(int x, int y);
		// is mouse(x,y) within button?
private:
	enum {B_Checkbox, B_Dot, B_Rectangle, B_Tube} type;
    int x, y, w, h, winW, winH;				// in pixels
	float radius;							// for dot
	float backgroundColor[3], textColor[3]; // default towhite
	int textWidth;
	void *font;
	void InitButton(Button *b, float *bgrndCol, float *txtCol);
};


//****** Slider: create a vertical or horizontal slider

class Slider
{
public:
	enum Orientation {Horizontal, Vertical};
    Slider(int x, int y, int size, float min, float max, float init,
		   Orientation o = Horizontal, float *color = NULL, bool displayInteger = false);
	void SetValue(float val);
	void SetRange(float min, float max, float init);
    void Draw(char *name, float *sliderColor = NULL);
    float GetValue();
    void Mouse(int x, int y);
		// if mouse hits slider, this should be called (mouse-down or mouse-drag)
    bool Hit(int x, int y);
		// return true if mouse (x,y) hits the slider
	bool HitMouse(int x, int y);
		// if hit, call Mouse, return true (combines above two routines)
private:
	Orientation orientation;
	bool displayInteger;
	int winW, winH;	        // window size
    int x, y;				// lower-left location, in pixels
    int size;				// length, in pixels
	float color[3];
    float loc;				// slider x-position, in pixels
    float min, max;
};


//****** Mover: drag a 3D point in space

class Mover {
public:
    void Pick(vec3 *p, int x, int y, mat4 &modelview, mat4 &persp);
		// associate *p with mover; subsequent calls to Drag will relocate *p
	void UnPick();
		// dis-associate *p with mover
	bool IsPicked();
		// is a *point associated with mover? (if not, Drag is no-op)
    void Drag(int x, int y, mat4 &modelview, mat4 &persp);
		// move *point in response to mouse drag
private:
	vec3 *point;
    float plane[4];
    void SetPlane(int x, int y, mat4 &modelview, mat4 &persp);
};


//****** Typescript: user text input

const int maxTypescript = 200;

class Typescript {
public:
    Typescript(int x, int y, int w, int h, char *text = NULL);
    void Clear();
    void AddChar(char c);
	void SetText(const char *txt);
    bool Hit(int _x, int _y);
    void Draw(float *color);
private:
    int x, y, w, h;
    char text[maxTypescript];
};


//****** Miscellany: file names

void GetFilenames(char *search, vector<string> &names);
void NewestFilename(char *search, string &name);
enum TypeIO {FileRead, FileSave};
bool ChooseFile(TypeIO type, string startDir, string &filename, string title);


#endif
