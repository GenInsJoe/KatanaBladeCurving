// Patch.cpp

#include "glew.h"
#include "freeglut.h"
#include "Draw.h"
#include "GLSL.h"
#include "Patch.h"
#include "mat.h"

// Evaluation

static vec3 BezTangent(float t, vec3 &b1, vec3 &b2, vec3 &b3, vec3 &b4) {
    float t2 = t*t, t3 = t*t2;
	vec3 p = (-3*t2+6*t-3)*b1+(9*t2-12*t+3)*b2+(6*t-9*t2)*b3+3*t2*b4;
    return normalize(p);
}

static vec3 BezPoint(float t, vec3 &b1, vec3 &b2, vec3 &b3, vec3 &b4) {
    float t2 = t*t, t3 = t*t2;
    return vec3((-t3+3*t2-3*t+1)*b1+(3*t3-6*t2+3*t)*b2+(3*t2-3*t3)*b3+t3*b4);
}

vec3 Patch::Point(float s, float t) {
	vec3 spts[4];
	SPts(s, spts);
	return BezPoint(t, spts[0], spts[1], spts[2], spts[3]);
}

vec3 Patch::Normal(float s, float t) {
	vec3 spts[4], tpts[4];
	for (int c = 0; c < 4; c++) {
		spts[c] = BezPoint(s, pts[0][c].point, pts[1][c].point, pts[2][c].point, pts[3][c].point);
		tpts[c] = BezPoint(t, pts[c][0].point, pts[c][1].point, pts[c][2].point, pts[c][3].point);
	}
	vec3 sTan = BezTangent(s, tpts[0], tpts[1], tpts[2], tpts[3]);
	vec3 tTan = BezTangent(t, spts[0], spts[1], spts[2], spts[3]);
	return normalize(cross(sTan, tTan));
}

void Patch::Eval(float s, float t, vec3 &point, vec3 &sTan, vec3 &tTan, vec3 *normal) {
	vec3 spts[4], tpts[4];
	for (int c = 0; c < 4; c++) {
		spts[c] = BezPoint(s, pts[c][0].point, pts[c][1].point, pts[c][2].point, pts[c][3].point);
		tpts[c] = BezPoint(t, pts[0][c].point, pts[1][c].point, pts[2][c].point, pts[3][c].point);
	}
	point = BezPoint(t, spts[0], spts[1], spts[2], spts[3]);
	tTan = BezTangent(t, spts[0], spts[1], spts[2], spts[3]); // spts define t-curve
	sTan = BezTangent(s, tpts[0], tpts[1], tpts[2], tpts[3]); // tpts define s-curve
	if (normal)
		*normal = normalize(cross(sTan, tTan));
}

// Initialization

void Patch::SetRes(int res) {
	this->res = res;
	SetVertices();
	SetTriangles();
}

void Patch::Init(int res, vec3 p0,  vec3 p1,  vec3 p2,  vec3 p3,
		                  vec3 p4,  vec3 p5,  vec3 p6,  vec3 p7, 
					      vec3 p8,  vec3 p9,  vec3 p10, vec3 p11,
					      vec3 p12, vec3 p13, vec3 p14, vec3 p15) {
    vec3 *tmp[] = {&p0, &p1, &p2,  &p3,  &p4,  &p5,  &p6,  &p7,
		           &p8, &p9, &p10, &p11, &p12, &p13, &p14, &p15};
	for (int i = 0; i < 16; i++){
		pts[i / 4][i % 4].point = *(tmp[i]);
	}
	glGenBuffers(1, &vBufferId);
	SetRes(res);
}

void Patch::Init(int res, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
	float vals[] = {0, 1/3.f, 2/3.f, 1.};
	for (int i = 0; i < 16; i++) {
		float ax = vals[i%4], ay = vals[i/4];
		vec3 p10 = p0+ax*(p1-p0), p32 = p2+ax*(p3-p2);
		pts[i / 4][i % 4].point = p10 + ay*(p32 - p10);
	}
	glGenBuffers(1, &vBufferId);
	SetRes(res);
}

void Patch::SPts(float s, vec3 spts[]) {
	for (int c = 0; c < 4; c++)
		spts[c] = BezPoint(s, pts[c][0].point, pts[c][1].point, pts[c][2].point, pts[c][3].point);
}

void Patch::TPts(float t, vec3 tpts[]) {
	// set tpts[0..3] each t-distance along the 4 t-curves; in other words, tpts[] defines an s-curve
	for (int c = 0; c < 4; c++)
		tpts[c] = BezPoint(t, pts[0][c].point, pts[1][c].point, pts[2][c].point, pts[3][c].point);
}

void Patch::SetVertices() {
	// make GPU vertex buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vBufferId);
	// get pointers to GPU memory for vertices, normals
	int nVerts = res*res, vSize = nVerts*sizeof(vec3);
	glBufferData(GL_ARRAY_BUFFER, 2*vSize, NULL, GL_STATIC_DRAW);
	vec3 *vPtr = (vec3 *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	vec3 *nPtr = vPtr+nVerts;
	// set res by res vertices
	vec3 spts[4], tpts[4];
	for (int i = 0; i < res; i++) {
		float s = (float) i /(res-1);
		SPts(s, spts); // defines a t-curve
		for (int j = 0; j < res; j++) {
			float t = (float) j/(res-1);
			TPts(t, tpts); // defines an s-curve
			vec3 sTan = BezTangent(s, tpts[0], tpts[1], tpts[2], tpts[3]);
			vec3 tTan = BezTangent(t, spts[0], spts[1], spts[2], spts[3]);
			// write directly to GPU memory
			*vPtr++ = BezPoint(t, spts[0], spts[1], spts[2], spts[3]);
			*nPtr++ = normalize(cross(sTan, tTan));
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Patch::SetTriangles() {
	int tri = 0;
	triangles.resize(2*(res-1)*(res-1));
	for (int j1 = 1; j1 < res; j1++)
		for (int i1 = 1; i1 < res; i1++) {
			int i0 = i1-1, j0 = j1-1;
			int v1 = j0*res+i0, v2 = j0*res+i1, v3 = j1*res+i1, v4 = j1*res+i0;
			triangles[tri++] = int3(v1, v2, v3);
			triangles[tri++] = int3(v1, v3, v4);
		}
	SetSegments();
}

static int CompareInt2(const void *arg1, const void *arg2) {
  int2 *p1 = (int2*) arg1, *p2 = (int2*) arg2;
  return p1->i1 == p2->i1? (p1->i2 < p2->i2? -1 : 1) : p1->i1 < p2->i1? -1 : 1;
}

void Patch::SetSegments() {
	// there are res rows and res columns of res-1 segments
	int nsegments = 2*res*(res-1), count = 0;
	segments.resize(nsegments);
	for (int i = 0; i < res; i++)
		for (int j = 0; j < res-1; j++)
			// in row i, add segment from column j to j+1, ie vertex i*(res-1)+j to i*(res-1)+j+1
			segments[count++] = int2(i*res+j, i*res+j+1);
	for (int j = 0; j < res; j++)
		for (int i = 0; i < res-1; i++)
			// in column j, add segment from row i to row i+1, ie vertex i*(res-1)+j to i*(res-1)+j+res
			segments[count++] = int2(i*res+j, i*res+j+res);
}

// GLSL Rendering

char *gouraudVShader = "\
	#version 400															\n\
	layout (location = 0) in vec3 position;									\n\
	layout (location = 1) in vec3 normal;									\n\
	out vec4 vPosition;														\n\
	out float intensity;													\n\
    uniform mat4 modelview;													\n\
	uniform mat4 persp;														\n\
	uniform vec3 light;														\n\
	void main()																\n\
	{																		\n\
		vPosition = modelview*vec4(position, 1);							\n\
		gl_Position = persp*vPosition;										\n\
		vec3 lightV = normalize(light-vPosition.xyz);						\n\
		vec4 xnormal = modelview*vec4(normal, 0);							\n\
		intensity = clamp(abs(dot(normalize(xnormal.xyz), lightV)), 0, 1);	\n\
	}\n";

char *gouraudFShader = "\
	#version 400															\n\
	in float intensity;														\n\
	uniform vec3 color;														\n\
	out vec4 fColor;														\n\
	void main()																\n\
	{																		\n\
		fColor = vec4(intensity*color, 1);									\n\
	}\n";

static GLuint shaderProgram = 0;

void Patch::UseShader(mat4 &modelview, mat4 &proj) {
	int ntris = triangles.size();
	int nVerts = res*res, vSize = nVerts*sizeof(vec3);
	if (!shaderProgram) {
		shaderProgram = GLSL::LinkProgramViaCode(gouraudVShader, gouraudFShader);
		if (!shaderProgram) {
			printf("Bezier.cpp: can't link shader program\n");
			return;
		}
	}
	glUseProgram(shaderProgram);
	glBindBuffer(GL_ARRAY_BUFFER, vBufferId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) vSize);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	GLSL::SetUniform(shaderProgram, "modelview", modelview);
	GLSL::SetUniform(shaderProgram, "persp", proj);
}

void Patch::Shade(mat4 &modelview, mat4 &proj, vec3 &light, vec3 &color) {
	int ntris = triangles.size();
	UseShader(modelview, proj);
	GLSL::SetUniform(shaderProgram, "light", light);
	GLSL::SetUniform(shaderProgram, "color", color);
	glDrawElements(GL_TRIANGLES, 3*ntris, GL_UNSIGNED_INT, &triangles[0]);
}

void Patch::Draw(mat4 &modelview, mat4 &proj, vec3 &color) {
	UseShader(modelview, proj);
	GLSL::SetUniform(shaderProgram, "color", color);
	glDrawElements(GL_LINES, 2*segments.size(), GL_UNSIGNED_INT, &segments[0]);
}

void Patch::DrawControlMesh(mat4 &fullview, vec3 &lineColor, vec3 &dotColor) {
	UseDrawShader(fullview);
    for (int k = 0; k < 16; k++)
		Disk(pts[k / 4][k % 4].point, 7, dotColor);
	DashOn();
    for (int i = 0; i < 4; i++) {
        for (int t = 0; t < 3; t++)
			Line(pts[i][t].point, pts[i][t + 1].point, lineColor);
        for (int s = 0; s < 3; s++)
			Line(pts[s][i].point, pts[s + 1][i].point, lineColor);
    }
	DashOff();
}

void Patch::SetControlSegments() {
	int segs[][2] = {{0,1},{1,2},{2,3},{4,5},{5,6},{6,7},{8,9},{9,10},{10,11},{12,13},{13,14},{14,15},
					 {0,4},{4,8},{8,12},{1,5},{5,9},{9,13},{2,6},{6,10},{10,14},{3,7},{7,11},{11,15}};
	controlSegments.resize(24);
	for (int i = 0; i < 24; i++)
		controlSegments[i] = int2(segs[i][0], segs[i][1]);
}

