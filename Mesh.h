/*	==============================
    Mesh.h - 3D mesh of triangles
    Copyright (c) Jules Bloomenthal, 2012
    All rights reserved
	=============================== */

#ifndef MESH_HDR
#define MESH_HDR

#include <vector>
#include "mat.h"

using std::vector;

/*struct int2 {
	int i1, i2;
	int2() {}
	int2(int i1, int i2) :
		i1(i1), i2(i2) {}
};

struct int3 {
	int i1, i2, i3;
	int3() {}
	int3(int i1, int i2, int i3) :
		i1(i1), i2(i2), i3(i3) {}
};*/

bool ReadBinaryObj(char         *filename,
				  vector<vec3>	&vertices,
				  vector<int3>	&triangles,
				  vector<int2>  *edges,
				  vector<vec3>	*vertexNormals  = NULL,
				  vector<vec2>	*vertexTextures = NULL,
				  vector<int>   *vertexTypes    = NULL,
				  vector<int>	*triangleGroups = NULL);


bool ReadAsciiObj(char          *filename,
				  vector<vec3>	&vertices,
				  vector<int3>	&triangles,
				  vector<vec3>	*vertexNormals  = NULL,
				  vector<vec2>	*vertexTextures = NULL,
				  vector<int>	*triangleGroups = NULL);

void Normalize(vector<vec3> &vertices, float scale = 1);
	// translate and apply uniform scale so that vertices all fit in -1,1 in X,Y and 0,1 in Z

void SetNormals(vector<vec3> &vertices,
				vector<int3> &triangles,
				vector<vec3> &normals);

#endif
