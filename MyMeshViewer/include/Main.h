#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <math.h>

using namespace std;

#define TRANSFORM_NONE		0
#define TRANSFORM_ROTATE	1
#define TRANSFORM_SCALE		2
#define TRANSFORM_TRANSLATE	3

#define OBJ_POINT			0
#define OBJ_WIREFRAME		1
#define OBJ_FLAT			2
#define OBJ_SMOOTH			3

//projection modes
#define VIEW_PERS   0
#define VIEW_ORTH   1

static int window;
static string testModels[] = { "bimba.m",  "bottle.m", "bunny.m", "cap.m", "eight.m", "gargoyle.m", "knot.m", "statute.m" };

static int press_x, press_y;
static float x_angle = 0.0f;
static float y_angle = 0.0f;
static float z_angle = 0.0f;
static float scale_size = 1.0f;
static float tx = 0.0f;
static float ty = 0.0f;
static int xform_mode = 0;
static int obj_mode = 0;
static int view_mode = 0;

float minX, minY, minZ, maxX, maxY, maxZ, max;
bool isLightEnabled = false;
string filename = "TestModels/";

//Data structures for vertex, face, vector and normal
typedef struct
{
	int index;
	float x, y, z;	//The vertex coordinates
}Vertex;
typedef struct
{
	int index;
	Vertex *v1, *v2, *v3;
}Face;
typedef struct
{
	float x, y, z;
}Vector;
typedef struct
{
	float x, y, z;
}Normal;
//Half-edge data structures, assuming counter-clockwise orientation
struct HE_edge
{
	struct HE_vert *vert;
	struct HE_edge *pair;
	struct HE_face *face;
	struct HE_edge *prev;
	struct HE_edge *next;
};
struct HE_face
{
	int index;
	HE_edge *edge;
};
struct HE_vert
{
	int index;
	float x, y, z;
	HE_edge *edge;
};

vector<string> vertices_string;
vector<string> faces_string;
vector<Vertex*> vertices;
vector<Face*> faces;
vector<Normal*> perFaceNormals;
vector<Normal*> perVertexNormals;

map<pair<int, int>, HE_edge*> HE_edges;
map<int, HE_vert*> HE_verts;
map<int, HE_face*> HE_faces;
map<int, vector<Face*>> adjFaces;

//Function prototypes
void renderScene();
void parseFile(string fileName);
void initVertices();
void initFaces();
void initHEMaps();
void assocPairToEdge();
Normal* faceNormal(Face* f);
void initPerFaceNormals();
vector<Face*> findAdjFaces(Vertex* v);
float calcFaceArea(Normal* n);
Normal* vertexNormal(Vertex* v);
void initPerVertexNormals();
void clearData();
void drawGround();
void drawAxes();
void findBoundingVolDimensions();
void drawBoundingVol();
void drawModelPoints();
void drawModelWireframe();
void drawModelFlat();
void drawModelSmooth();
void myMouse(int button, int state, int x, int y);
void myMotion(int x, int y);
void mykey(unsigned char key, int x, int y);
void mySpecial(int key, int x, int y);
void init(string filename);

#endif