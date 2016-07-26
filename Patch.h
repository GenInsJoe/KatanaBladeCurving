// Patch.h

#include <vector>
#include "mat.h"

using std::vector;

class Patch {
public:
	struct patchPoints{
		vec3 point;
		vec3 origPoint;
		float xresis;					// how resistant to being moved a point is in x-direction
		float yresis;					// how resistant to being moved a point is in y-direction
	};
	patchPoints  pts[4][4];             // 16 control points, indexed by [s][t]
	int          res;                   // res*res vertices
	vector<int3> triangles;				// 2(res-1)**2 triangles
	vector<int2> segments;				// triangle outlines
	vector<int2> controlSegments;		// control mesh
	unsigned int vBufferId;				// GPU vertex buffer
	void SetRes(int res);
	void Init(int res, vec3 p0,  vec3 p1,  vec3 p2,  vec3 p3);
		// create patch of 16 control points from quadrilateral
	void Init(int res, vec3 p0,  vec3 p1,  vec3 p2,  vec3 p3,
		               vec3 p4,  vec3 p5,  vec3 p6,  vec3 p7, 
					   vec3 p8,  vec3 p9,  vec3 p10, vec3 p11,
					   vec3 p12, vec3 p13, vec3 p14, vec3 p15);
		// create patch given 16 control points
	void SetVertices();
	void SetTriangles();
	void SetSegments();
	void Reset();
    void Shade(mat4 &modelview, mat4 &proj, vec3 &light, vec3 &color);
	void Draw(mat4 &modelview, mat4 &proj, vec3 &color);
	void DrawControlMesh(mat4 &fullview, vec3 &lineColor, vec3 &dotColor);
	void SetControlSegments();
	// support
	void UseShader(mat4 &modelview, mat4 &proj);
	void SPts(float s, vec3 spts[]);
	void TPts(float t, vec3 tpts[]);
	// geometry
	vec3 Point(float s, float t);
	vec3 Normal(float s, float t);
	void Eval(float s, float t, vec3 &point, vec3 &stan, vec3 &ttan, vec3 *normal = NULL);
		// the tangent vectors are unit length
};
