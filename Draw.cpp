#include "glew.h"
#include "freeglut.h"
#include "Draw.h"
#include "GLSL.h"


//****** Errors

int Errors(char *buf)
{
    int nErrors = 0;
    buf[0] = 0;
    for (;;) {
        GLenum n = glGetError();
        if (n == GL_NO_ERROR)
            break;
        sprintf(buf+strlen(buf), "%s%s", !nErrors++? "" : ", ", gluErrorString(n));
    }
    return nErrors;
}

void  CheckGL_Errors()
{
	char buf[100];
	if (Errors(buf))
		printf("GL error(s): %s\n", buf);
	else
		printf("<no GL errors>\n");
}


//****** Screen Mode

mat4 ScreenMode()
{
	return ScreenMode(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

mat4 ScreenMode(int width, int height)
{
	mat4 scale = Scale(2.f / (float) width, 2.f / (float) height, 1.);
	mat4 tran = Translate(-1, -1, 0);
	return tran*scale;
}

void ScreenPoint(vec3 p, mat4 m, float &xscreen, float &yscreen, float *zscreen)
{
	int width = glutGet(GLUT_WINDOW_WIDTH), height = glutGet(GLUT_WINDOW_HEIGHT);
	vec4 xp = m*vec4(p, 1);
 	xscreen = ((xp.x/xp.w)+1)*.5f*(float)width;
	yscreen = ((xp.y/xp.w)+1)*.5f*(float)height;
	if (zscreen)
		*zscreen = xp.z; // /xp.w;
}

void NDCPoint(vec3 p, mat4 m, float &xscreen, float &yscreen, float *zscreen)
{
	int width = glutGet(GLUT_WINDOW_WIDTH), height = glutGet(GLUT_WINDOW_HEIGHT);
	vec4 xp = m*vec4(p, 1);
 	xscreen = ((xp.x/xp.w)+1)*.5f*(float)width;
	yscreen = ((xp.y/xp.w)+1)*.5f*(float)height;
	if (zscreen)
		*zscreen = xp.z; // /xp.w;
}

vec2 ScreenPoint(vec3 p, mat4 m, float *zscreen)
{
	vec2 ret;
	ScreenPoint(p, m, ret.x, ret.y, zscreen);
	return ret;
}

float ScreenDistSq(int x, int y, vec3 p, mat4 m, int width, int height, float *zscreen)
{
	vec4 xp = m*vec4(p, 1);
 	float xscreen = ((xp.x/xp.w)+1)*.5f*(float) width;
	float yscreen = ((xp.y/xp.w)+1)*.5f*(float) height;
	if (zscreen)
		*zscreen = xp.z;//xp.w;
	float dx = x-xscreen, dy = y-yscreen;
    return dx*dx+dy*dy;
}

void ScreenLine(float xscreen, float yscreen, mat4 &modelview, mat4 &persp, float p1[], float p2[])
{
    // compute 3D world space line, given by p1 and p2, that transforms
    // to a line perpendicular to the screen at (xscreen, yscreen)
	// m is the full transformation matrix (ie, modelView*projection)
	int vp[4];
	double tpersp[4][4], tmodelview[4][4], a[3], b[3];
	// get viewport
	glGetIntegerv(GL_VIEWPORT, vp);
	// create transposes
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			tmodelview[i][j] = modelview[j][i];
			tpersp[i][j] = persp[j][i];
		}
	if (gluUnProject(xscreen, yscreen, .25, (const double*) tmodelview, (const double*) tpersp, vp, &a[0], &a[1], &a[2]) == GL_FALSE)
        printf("UnProject false\n");
	if (gluUnProject(xscreen, yscreen, .50, (const double*) tmodelview, (const double*) tpersp, vp, &b[0], &b[1], &b[2]) == GL_FALSE)
        printf("UnProject false\n");
	for (int i = 0; i < 3; i++) {
		p1[i] = static_cast<float>(a[i]);
		p2[i] = static_cast<float>(b[i]);
	}
}


//***** Draw Shader

int drawShader = 0;

char *drawVShader = "\
	#version 400								\n\
	layout (location = 0) in vec3 position;		\n\
	layout (location = 1) in vec3 color;		\n\
	out vec3 vColor;							\n\
    uniform mat4 view; // persp*modelView       \n\
	void main()									\n\
	{											\n\
		gl_Position = view*vec4(position, 1);	\n\
		vColor = color;							\n\
	}											\n";

char *drawFShader = "\
	#version 400								\n\
	uniform float opacity = 1;					\n\
	in vec3 vColor;								\n\
	out vec4 fColor;							\n\
	void main()									\n\
	{											\n\
	    fColor = vec4(vColor, opacity);			\n\
	}											\n";

void UseDrawShader()
{
	if (!drawShader)
		drawShader = InitShader(drawVShader, drawFShader);
	glUseProgram(drawShader);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
}

void UseDrawShader(mat4 viewMatrix)
{
	UseDrawShader();
	GLSL::SetUniform(drawShader, "view", viewMatrix);
}

class DrawBuffer {
public:
	DrawBuffer() {
		vBuf = -1;
	}
	~DrawBuffer() {
		// if (vBuf > 0)
		//  	glDeleteBuffers(1, &vBuf);
	}
	GLuint vBuf;
} dotBuffer, lineBuffer, quadBuffer;


//***** Display

// Lines

void Stipple(int factor, int a,int b,int c,int d,int e,int f,int g,int h,
                         int i,int j,int k,int l,int m,int n,int o,int p)
{
    GLushort pattern = 0;
    int bits[16] = {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p};
    for (int index = 0; index < 16; index++) {
        int bit = bits[index];
        if (bit == -1)
            break;
        pattern |= 1<<bit;
    }
    glLineStipple(factor, pattern);
}

void DashOn(int factor, int off)
{
	glEnable(GL_LINE_STIPPLE);
	Stipple(factor, (0+off)%16, (1+off)%16, (2+off)%16, (3+off)%16, (8+off)%16, (9+off)%16, (10+off)%16, (11+off)%16);
}

void DashOff()
{
	glDisable(GL_LINE_STIPPLE);
}

void Line(float *p1, float *p2, float *col1, float *col2, float opacity)
{
	bool alreadyUsing = GLSL::CurrentShader() == drawShader;
	if (!alreadyUsing)
		UseDrawShader();

	// align vertex data
	float points[][3] = {{p1[0], p1[1], p1[2]}, {p2[0], p2[1], p2[2]}};
	float colors[][3]  = {{col1[0], col1[1], col1[2]}, {col2[0], col2[1], col2[2]}};

 	// create a vertex buffer for the array
	if (quadBuffer.vBuf < 0) {
		glGenBuffers(1, &quadBuffer.vBuf);
		glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.vBuf);
		// allocate buffer memory and load location and color data
		int bufferSize = sizeof(points)+sizeof(colors);
		glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
	}

    // set active vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, lineBuffer.vBuf);

    // allocate buffer memory and load location and color data
	glBufferData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    // connect shader inputs
	GLSL::VertexAttribPointer(drawShader, "position", 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    GLSL::VertexAttribPointer(drawShader, "color", 3, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(points));

	GLSL::SetUniform(drawShader, "opacity", opacity);

	// draw
	glDrawArrays(GL_LINES, 0, 2);

	// cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (!alreadyUsing)
		glUseProgram(0);
}

void Line(vec2 &p1, vec2 &p2, vec3 &col1, vec3 &col2, float opacity)
{
	float pp1[] = {p1.x, p1.y, 0}, pp2[] = {p2.x, p2.y, 0};
	Line(pp1, pp2, col1, col2, opacity);
}

void Line(vec3 &p1, vec3 &p2, vec3 &col, float opacity, float width)
{
	glLineWidth(width);
	Line(p1, p2, col, col, opacity);
}

void Line(vec3 &p1, vec3 &p2, vec3 &col1, vec3 &col2, float opacity)
{
	Line((float *) &p1.x, (float *) &p2.x, (float *) &col1.x, (float *) &col2, opacity);
}

void Line(float x1, float y1, float x2, float y2, float *col1, float *col2, float opacity)
{
	float p1[] = {x1, y1, 0}, p2[] = {x2, y2, 0};
	Line(p1, p2, col1, col2, opacity);
}

void Line(int x1, int y1, int x2, int y2, float *col1, float *col2, float opacity)
{
	float p1[] = {(float) x1, (float) y1, 0};
	float p2[] = {(float) x2, (float) y2, 0};
	Line(p1, p2, col1, col2, opacity);
}


// Disks

void Disk(float *point, float radius, float *color)
{
	UseDrawShader();

	// create a vertex buffer
	if (dotBuffer.vBuf < 0) {
		glGenBuffers(1, &dotBuffer.vBuf);
		glBindBuffer(GL_ARRAY_BUFFER, dotBuffer.vBuf);
		int bufferSize = 6*sizeof(float);
		glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
	}

	// set active buffer
	glBindBuffer(GL_ARRAY_BUFFER, dotBuffer.vBuf);

    // allocate buffer memory and load location and color data
	glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3*sizeof(float), point);
	glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float), 3*sizeof(float), color);

	// connect shader inputs
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);				// position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(vec3));	// color
	// if not using layouts:
		//GLSL::VertexAttribPointer(drawShader.id, "position", 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
		//GLSL::VertexAttribPointer(drawShader.id, "color", 3, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(vec3));

	// draw, cleanup
	glPointSize(radius);
	glDrawArrays(GL_POINTS, 0, 1);
}

void Disk(vec3 &p, float radius, vec3 &color)
{
	Disk((float *) &p.x, radius, color);
}

void Disk(vec2 &p, float radius, vec3 &color)
{
	float pp[] = {p.x, p.y, 0};
	Disk(pp, radius, color);
}

void Disk(int x, int y, float radius, float *color)
{
	float p[] = {(float) x, (float) y, 0};
	Disk(p, radius, color);
}


// Arrows

void Arrow(vec2 &base, vec2 &head, char *label, double headSize, vec3 &col)
{
	Line(base, head, col, col);
    if (headSize > 0) {
	    vec2 v1 = (float)headSize*normalize(head-base), v2(v1.y/2.f, -v1.x/2.f);
		vec2 head1(head-v1+v2), head2(head-v1-v2);
        Line(head, head1, col, col);
        Line(head, head2, col, col);
    }
    if (label) {
		glColor3fv(&col.x);
        Text((int)head.x+5, (int)head.y, label);
	}
}

void ArrowV(vec3 &base, vec3 &vec, mat4 &m, char *label, double headSize, vec3 &col)
{
	vec2 base2 = ScreenPoint(base, m), head2 = ScreenPoint(base+vec, m);
	mat4 screen = ScreenMode();
	UseDrawShader(screen);
	Arrow(base2, head2, label, headSize, col);
	UseDrawShader(m);
}


// Support

void Quad(vec3 &p1, vec3 &p2, vec3 &p3, vec3 &p4, float *col, float opacity)
{
	UseDrawShader();

	// align vertex data
	float points[][3] = {p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, p4.x, p4.y, p4.z};
	float colors[][3] = {col[0], col[1], col[2], col[0], col[1], col[2], col[0], col[1], col[2], col[0], col[1], col[2]};

 	// create vertex buffer
	if (lineBuffer.vBuf < 0) {
		glGenBuffers(1, &quadBuffer.vBuf);
		glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.vBuf);
		int bufferSize = sizeof(points)+sizeof(colors);
		glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
	}
    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.vBuf);

    // allocate buffer memory and load location and color data
	glBufferData(GL_ARRAY_BUFFER, sizeof(points)+sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    // connect shader inputs
	GLSL::VertexAttribPointer(drawShader, "position", 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    GLSL::VertexAttribPointer(drawShader, "color", 3, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(points));
	GLSL::SetUniform(drawShader, "opacity", opacity);

	// draw, cleanup
	glDrawArrays(GL_QUADS, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
}

#define N_CIRCLE_POINTS 12
vec2 circle[N_CIRCLE_POINTS];

static bool SetCircle()
{
    for (int i = 0; i < N_CIRCLE_POINTS; i++) {
        double angle = 2*3.141592*i/N_CIRCLE_POINTS;
        circle[i] = vec2((GLfloat) cos(angle), (GLfloat) sin(angle));
    }
    return true;
}

static bool circleSet = SetCircle();


// Text

void Text(int x, int y, const char *text)
{
	int program = GLSL::CurrentShader();
	int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
	glUseProgram(0); // no text support in GLSL
	glRasterPos2f((float) (2*x)/w-1, (float) (2*y)/h-1);
	glutBitmapString(GLUT_BITMAP_9_BY_15, (unsigned char*) text);
	glUseProgram(program);
}

void Text(vec3 &p, mat4 &m, char *text, vec3 &col)
{
	vec2 p2 = ScreenPoint(p, m);
	glColor3fv(&col.x);
	Text((int)p2.x+5, (int)p2.y, text);
}

#define FormatString(buffer, maxBufferSize, format) {  \
    if (format) {                                      \
        va_list ap;                                    \
        va_start(ap, format);                          \
        _vsnprintf(buffer, maxBufferSize, format, ap); \
        va_end(ap);                                    \
    }                                                  \
    else                                               \
        (buffer)[0] = 0;                               \
}

void Text(vec3 &p, mat4 &m, vec3 &color, char *format, ...) {
    char buf[500];
    FormatString(buf, 500, format);
	Text(p, m, buf, color);
}


// Miscellany

void Rect(int x, int y, int w, int h, float *col, bool solid, float opacity)
{
	float linewidth;
	glGetFloatv(GL_LINE_WIDTH, &linewidth);
	float halfw = .5f*linewidth;
	float x1 = (float) x, x2 = (float) (x+w), y1 = (float) y, y2 = (float) (y+h);
	if (solid)
		Quad(vec3(x1, y1, 0), vec3(x2, y1, 0), vec3(x2, y2, 0), vec3(x1, y2, 0), col, opacity);
	else {
		Line(x1-halfw, y1, x2+halfw, y1, col, col, opacity);
		Line(x2, y1-halfw, x2, y2+halfw, col, col, opacity);
		Line(x1-halfw, y2, x2+halfw, y2, col, col, opacity);
		Line(x1, y1-halfw, x1, y2+halfw, col, col, opacity);
	}
}

void Circle(vec2 &p, float dia, vec3 &color)
{
    float radius = dia/2;
	vec2 p1 = p+radius*circle[N_CIRCLE_POINTS-1];
    for (int i = 0; i < N_CIRCLE_POINTS; i++) {
		vec2 p2 = p+radius*circle[i];
		Line(p1, p2, color, color);
		p1 = p2;
    }
}

void Cross(float p[], float s, float col[])
{
	float p1[3], p2[3];
	for (int n = 0; n < 3; n++)
		for (int i = 0; i < 3; i++) {
			float offset = i==n? s : 0;
			p1[i] = p[i]+offset;
			p2[i] = p[i]-offset;
			Line(p1, p2, col, col);
		}
}

void Asterisk(float p[], float s, float col[])
{
	float p1[3], p2[3];
	for (int i = 0; i < 8; i++) {
		p1[0] = p[0]+(i<4? -s : s);
		p1[1] = p[1]+(i%2? -s : s);
		p1[2] = p[2]+((i/2)%2? -s : s);
		for (int k = 0; k < 3; k++)
			p2[k] = 2*p[k]-p1[k];
		Line(p1, p2, col, col);
	}
}

void Crosshairs(vec2 &s, float radius, vec3 &color)
{
    float innerRad = .4f*radius;
	Circle(s, .5f, color);
    Circle(s, 2*innerRad, color);
    Line(s.x-innerRad, s.y, s.x-radius, s.y, color, color);
    Line(s.x+innerRad, s.y, s.x+radius, s.y, color, color);
    Line(s.x, s.y-innerRad, s.x, s.y-radius, color, color);
    Line(s.x, s.y+innerRad, s.x, s.y+radius, color, color);
}

void Sun(vec2 &p, vec3 *flashColor)
{
	vec3 yel(1, 1, 0), red(1, 0, 0), *col = flashColor? flashColor : &red;
    Disk(p, 12, *col);
    Disk(p, 8, yel);
	glLineWidth(1.);
    for (int r = 0, nRays = 16; r < nRays; r++) {
        float a = 2*3.141592f*(float)r/(nRays-1), dx = cos(a), dy = sin(a);
        float len = 11*(r%2? 1.8f : 2.5f);
        Line(p.x+9*dx, p.y+9*dy, p.x+len*dx, p.y+len*dy, *col, *col);
    }
}
