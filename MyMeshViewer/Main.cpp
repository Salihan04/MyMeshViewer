#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "glut.h"

using namespace std;

#define TRANSFORM_NONE		0
#define TRANSFORM_ROTATE	1
#define TRANSFORM_SCALE		2
#define TRANSFORM_TRANSLATE	3

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

float minX, minY, minZ, maxX, maxY, maxZ, max;
bool isLightEnabled = false;

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

int main(int argc, char **argv)
{
	//Reset data
	clearData();

	//Initialise data needed for rendering
	parseFile("TestModels/cap.m");
	cout << "Model loaded" << endl;
	cout << "Rendering..." << endl;
	initVertices();
	initFaces();
	
	//Initialise half-edge data structures
	initHEMaps();
	assocPairToEdge();

	//Initialise per face normals and per vertex normals
	initPerFaceNormals();
	initPerVertexNormals();

	findBoundingVolDimensions();

	//Initialising Glut
	glutInit(&argc, argv);
	//Use double buffer to get better results on animation
	//Use depth buffer for hidden surface removal
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	//Initialise window size and position
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(300, 100);

	//Create window with OpenGL
	window = glutCreateWindow("CZ4004 MeshViewer (Muhammad Salihan Bin Zaol-kefli)");

	glClearColor(0.8f, 0.8f, 0.8f, 1.0);	//Light grey background

	//Register callbacks
	glutDisplayFunc(renderScene);
	glutMouseFunc(myMouse);
	glutMotionFunc(myMotion);

	cout << "Rendering complete" << endl;

	glutMainLoop();

	return 0;
}

//Function to draw items in scene
void renderScene()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Setup OpenGL lighting
	GLfloat light_position[] = { -5.0f, 1.0f, -5.0f, 0.0f };	//light position
	GLfloat white_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };			//light color
	GLfloat lmodel_ambient[] = { 1.0f, 0.1f, 0.1f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	isLightEnabled = true;

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	//Setup the perspective projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	//Handle the translation
	glTranslatef(tx, ty, 0.0f);
	//Handle the scaling
	glScalef(scale_size, scale_size, scale_size);
	//Handle the rotation
	glRotatef(x_angle, 1.0f, 0.0f, 0.0f);
	glRotatef(y_angle, 0.0f, 1.0f, 0.0f);
	glRotatef(z_angle, 0.0f, 0.0f, 1.0f);

	drawGround();
	drawAxes();
	drawBoundingVol();

	//drawModelPoints();
	//drawModelWireframe();
	//drawModelFlat();
	drawModelSmooth();

	//Swap the buffers
	glutSwapBuffers();
}

//Function to read from model file and separate the strings for Vertex and Face
void parseFile(string fileName)
{
	string str;
	ifstream infile;

	//Open file
	infile.open(fileName);

	//Check if file has been opened
	if (!infile.is_open())
		cout << "Cannot open " << fileName << endl;

	while (infile)
	{
		//Read each line of the file as long as end of file is not encountered
		getline(infile, str);
		if (infile)
		{
			//If the line begins with 'Vertex', add to vertices vector
			if (str[0] == 'V')
				vertices_string.push_back(str);
			//If the line begins with 'Face', add to faces vector
			else if (str[0] == 'F')
				faces_string.push_back(str);
		}
	}

	//Close file
	infile.close();
}

//Function to initialise and populate a collection of vertices based on data from model file
void initVertices()
{
	for (size_t n = 0; n < vertices_string.size(); n++)
	{
		int j = 0;
		int count = 0;
		float coords[3];
		string temp = "";
		Vertex* v = new Vertex();

		for (size_t i = 0; i < vertices_string.at(n).length(); i++)
		{
			if (vertices_string.at(n)[i] == ' ')
			{
				count++;

				if (count < 3)
					continue;
			}

			//The coordinate data only starts after 3 spaces are found in the string
			if (count >= 3)
			{
				//Need to take into account of space characters between x, y, and z coordinate
				//For some vertex, coordinates start at a later string index depending on vertex index
				if (vertices_string.at(n)[i] == ' ')
				{
					//When a space character is encountered, check if temp is empty string
					//If temp is not empty, add to array of cordinates and then reset temp
					if (temp != "")
					{
						coords[j] = stof(temp);
						j++;
						temp = "";
					}
					//If temp is empty, it means no coordinate has been encountered yet
					//Continue traversing
					else
						continue;
				}
				//Build up temp which will then be converted to float value of coordinate
				else
					temp += vertices_string.at(n)[i];
			}
		}
		//When end of string is reached, check if temp is non-empty string
		//If non-empty, add to array of coordinates and then reset temp
		if (temp != "")
		{
			coords[j] = atof(temp.c_str());
			j = 0;
			temp = "";
		}

		//Use coordinates to initialise vertex and add to vertices vector
		v->index = n + 1;
		v->x = coords[0];
		v->y = coords[1];
		v->z = coords[2];
		vertices.push_back(v);
	}
}

//Function to initialise and populate a collection of faces based on data from model file
void initFaces()
{
	for (size_t n = 0; n < faces_string.size(); n++)
	{
		int j = 0;
		int count = 0;
		int vertexIndex[3];
		string temp = "";
		Face* f = new Face();

		for (size_t i = 0; i < faces_string.at(n).length(); i++)
		{
			if (faces_string.at(n)[i] == ' ')
			{
				count++;

				if(count < 3)
					continue;
			}

			//The vertex indices data only starts after 3 spaces are found in the string
			if (count >= 3)
			{
				//Need to take into account of space characters between vertex indices
				//For some face, vertex indices start at a later string index depending on face index
				if (faces_string.at(n)[i] == ' ')
				{
					//When a space character is encountered, check if temp is empty string
					//If temp is not empty, add to array of vertex indices and then reset temp
					if (temp != "")
					{
						vertexIndex[j] = atoi(temp.c_str());
						j++;
						temp = "";
					}
					//If temp is empty, it means no vertex index has been encountered yet
					//Continue traversing
					else
						continue;
				}
				//Build up temp which will then be converted to int value of vertex index
				else
					temp += faces_string.at(n)[i];
			}
		}
		//When end of string is reached, check if temp is non-empty string
		//If non-empty, add to array of vertex indices and then reset temp
		if (temp != "")
		{
			vertexIndex[j] = atoi(temp.c_str());
			j = 0;
			temp = "";
		}

		//Use vertex indices to initialise face and add to faces vector
		f->index = n + 1;
		f->v1 = vertices.at(vertexIndex[0] - 1);
		f->v2 = vertices.at(vertexIndex[1] - 1);
		f->v3 = vertices.at(vertexIndex[2] - 1);
		faces.push_back(f);
	}
}

//Function to initialise HE data structures and add them into their respective maps
void initHEMaps()
{
	//Do the below for each face in the model
	for (size_t i = 0; i < faces.size(); i++)
	{
		Face* f = faces.at(i);
		HE_edge* edge = new HE_edge();
		HE_face* face = new HE_face();
		HE_vert* vert = new HE_vert();

		//1st edge
		//Init starting vertex
		vert->index = f->v1->index;
		vert->x = f->v1->x;
		vert->y = f->v1->y;
		vert->z = f->v1->z;
		vert->edge = edge;
		//Add vertex to HE_verts if it is not already in
		if (HE_verts.count(vert->index) == 0)
			HE_verts[vert->index] = vert;

		//Init incident face
		face->index = f->index;
		face->edge = edge;
		//Add face to HE_faces if it is not already in
		if (HE_faces.count(f->index) == 0)
			HE_faces[f->index] = face;

		//Init 1st edge
		edge->vert = vert;
		edge->face = face;
		if(HE_edges.count(make_pair(f->v1->index, f->v2->index)) == 0)
			HE_edges[make_pair(f->v1->index, f->v2->index)] = edge;

		//2nd edge
		HE_edge* next = new HE_edge();
		vert = new HE_vert();

		//Init starting vertex
		vert->index = f->v2->index;
		vert->x = f->v2->x;
		vert->y = f->v2->y;
		vert->z = f->v2->z;
		vert->edge = next;
		//Add vertex to HE_verts if it is not already in
		if (HE_verts.count(vert->index) == 0)
			HE_verts[vert->index] = vert;

		//Init 2nd edge
		//2nd edge share incident face with 1st and 3rd edge
		next->vert = vert;
		next->face = face;
		if (HE_edges.count(make_pair(f->v2->index, f->v3->index)) == 0)
			HE_edges[make_pair(f->v2->index, f->v3->index)] = next;

		//3rd edge
		HE_edge* prev = new HE_edge();
		vert = new HE_vert();

		//Init starting vertex
		vert->index = f->v3->index;
		vert->x = f->v3->x;
		vert->y = f->v3->y;
		vert->z = f->v3->z;
		vert->edge = prev;
		//Add vertex to HE_verts if it is not already in
		if (HE_verts.count(vert->index) == 0)
			HE_verts[vert->index] = vert;

		//Init 3rd edge
		//3rd edge share incident face with 1st and 2nd edge
		prev->vert = vert;
		prev->face = face;
		if (HE_edges.count(make_pair(f->v3->index, f->v1->index)) == 0)
			HE_edges[make_pair(f->v3->index, f->v1->index)] = prev;

		//Link up 1st, 2nd, and 3rd edges with each other
		edge->next = next;
		edge->prev = prev;

		next->next = prev;
		next->prev = edge;

		prev->next = edge;
		prev->prev = next;
	}
}

//Function to associate each edge of each face with its pair
void assocPairToEdge()
{
	for (size_t i = 0; i < faces.size(); i++)
	{
		Face* f = faces.at(i);

		//1st edge
		HE_edge* edge1 = HE_edges[make_pair(f->v1->index, f->v2->index)];
		//Check that a pair edge exists
		if (HE_edges.count(make_pair(f->v2->index, f->v1->index)) > 0)
		{
			HE_edge* pair = HE_edges[make_pair(f->v2->index, f->v1->index)];

			//Associate edge and pair with each other
			edge1->pair = pair;
			pair->pair = edge1;
		}

		//2nd edge
		HE_edge* edge2 = HE_edges[make_pair(f->v2->index, f->v3->index)];
		//Check that a pair edge exists
		if (HE_edges.count(make_pair(f->v3->index, f->v2->index)) > 0)
		{
			HE_edge* pair = HE_edges[make_pair(f->v3->index, f->v2->index)];

			//Associate edge and pair with each other
			edge2->pair = pair;
			pair->pair = edge2;
		}

		//3rd edge
		HE_edge* edge3 = HE_edges[make_pair(f->v3->index, f->v1->index)];
		//Check that a pair edge exists
		if (HE_edges.count(make_pair(f->v1->index, f->v3->index)) > 0)
		{
			HE_edge* pair = HE_edges[make_pair(f->v1->index, f->v3->index)];

			//Associate edge and pair with each other
			edge3->pair = pair;
			pair->pair = edge3;
		}
	}
}

//Function to find normal of a face
Normal* faceNormal(Face* f)
{
	Vector* v1 = new Vector();
	Vector* v2 = new Vector();
	Normal* n = new Normal();

	//Init v1 which is a vector of 1 edge of face
	v1->x = f->v1->x - f->v2->x;
	v1->y = f->v1->y - f->v2->y;
	v1->z = f->v1->z - f->v2->z;

	//Init v2 which is a vector of another edge of face
	v2->x = f->v3->x - f->v2->x;
	v2->y = f->v3->y - f->v2->y;
	v2->z = f->v3->z - f->v2->z;

	//Normal is found by applying cross product on the 2 vectors
	//Cross product's formula for x, y, and z coordinates as shown below
	n->x = (v1->y * v2->z) - (v1->z * v2->y);
	n->y = (v1->z * v2->x) - (v1->x - v2->z);
	n->z = (v1->x * v2->y) - (v1->y * v2->x);

	return n;
}

//Function to initialise and populate a collection of per face normals
void initPerFaceNormals()
{
	//For each face
	for (size_t i = 0; i < faces.size(); i++)
	{
		//Find normal and add to collection
		Normal* n = faceNormal(faces.at(i));
		perFaceNormals.push_back(n);
	}
}

//Function to find the faces adjacent to a vertex
vector<Face*> findAdjFaces(Vertex* v)
{
	HE_vert* vert = HE_verts[v->index];
	HE_edge* out_edge = vert->edge;
	HE_edge* curr = out_edge;
	vector<Face*> adjFaces;
	
	adjFaces.push_back(faces.at(curr->face->index - 1));

	//Using one-ring neighbour algo to find and add to collection of adjacent faces
	while ((curr->pair != NULL) && (curr->pair->next != out_edge))
	{
		curr = curr->pair->next;
		adjFaces.push_back(faces.at(curr->face->index - 1));
	}

	return adjFaces;
}

//Function to calculate the area of each face
float calcFaceArea(Normal* n)
{
	//Length of normal is equivalent to area of parallelogram formed by 2 vectors
	//Since face is a triangle, area of face is half of length of face's normal
	float length = sqrt((n->x * n->x) + (n->y * n->y) + (n->z * n->z));

	return length / 2;
}

//Function to calculate vertex normal
Normal* vertexNormal(Vertex* v)
{
	float totalArea = 0.0f;
	float faceArea;
	vector<float> faceAreas;
	Normal* n = new Normal();
	n->x = 0.0f;
	n->y = 0.0f;
	n->z = 0.0f;

	//Get all faces adjacent to the vertex
	vector<Face*> adjFaces = findAdjFaces(v);

	//For each adjacent face
	for (size_t i = 0; i < adjFaces.size(); i++)
	{
		Face* f = adjFaces.at(i);
		int index = f->index;

		//Get the normal of the face
		Normal* faceNormal = perFaceNormals.at(index - 1);

		//Calculate the area of the face
		faceArea = calcFaceArea(faceNormal);
		//Add to collection of face areas
		faceAreas.push_back(faceArea);

		//Add area of face to totalArea
		totalArea += faceArea;
	}

	//Calculate vertex normal
	//For each face adjacent to vertex
	for (size_t j = 0; j < adjFaces.size(); j++)
	{
		Face* f = adjFaces.at(j);
		int index = f->index;

		//Get normal of face
		Normal* faceNormal = perFaceNormals.at(index - 1);

		//Weight is area of 1 face / total area
		float weight = faceAreas.at(j) / totalArea;

		//Vertex normal is summation of weight * face normal
		n->x += weight * faceNormal->x;
		n->y += weight * faceNormal->y;
		n->z += weight * faceNormal->z;
	}

	return n;
}

//Function to initialise and populate a collection of per vertex normals
void initPerVertexNormals()
{
	//For each vertex
	for (size_t i = 0; i < vertices.size(); i++)
	{
		//Find the normal and add to collection
		Normal* n = vertexNormal(vertices.at(i));
		perVertexNormals.push_back(n);
	}
}

//Function to clear vectors and maps
void clearData()
{
	//Clear vectors
	vertices_string.clear();
	faces_string.clear();
	vertices.clear();
	faces.clear();
	perFaceNormals.clear();
	perVertexNormals.clear();

	//Clear maps
	HE_verts.clear();
	HE_faces.clear();
	HE_edges.clear();
}

//Function to draw grid lines on the ground which is the xy plane
void drawGround()
{
	//Set color of lines to black
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

	glBegin(GL_LINES);
		for (int x = -10; x < 11; x++)
		{
			glVertex2i(x, -10);
			glVertex2i(x, 10);
		}
		for (int y = -10; y < 11; y++)
		{
			glVertex2i(-10, y);
			glVertex2i(10, y);
		}
	glEnd();
}

//Function to draw x,y,z axes
void drawAxes()
{
	GLUquadric* quad;
	quad = gluNewQuadric();

	//Drawing y-axis which is made up of a cylinder and a cone
	//Need to rotate along x-axis since gluCylinder & glutSolidCone draws along z-axis
	glPushMatrix();
		glRotatef(270.0, 1.0f, 0.0f, 0.0f);
		glScalef(1 / 10.0f, 1 / 10.0f, 1.0f);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		gluCylinder(quad, 0.1f, 0.1f, 1.0f, 32, 32);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0f, 1.0f, 0.0f);
		glRotatef(270.0, 1.0f, 0.0f, 0.0f);
		glScalef(1 / 10.0f, 1 / 10.0f, 1.0f);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glutSolidCone(0.3, 0.1, 32, 32);
	glPopMatrix();

	//Drawing x-axis which is made up of a cylinder and a cone
	//Need to rotate along y-axis since gluCylinder & glutSolidCone draws along z-axis
	glPushMatrix();
		glRotatef(90.0, 0.0f, 1.0f, 0.0f);
		glScalef(1 / 10.0f, 1 / 10.0f, 1.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		gluCylinder(quad, 0.1f, 0.1f, 1.0f, 32, 32);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(1.0f, 0.0f, 0.0f);
		glRotatef(90.0, 0.0f, 1.0f, 0.0f);
		glScalef(1 / 10.0f, 1 / 10.0f, 1.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glutSolidCone(0.3, 0.1, 32, 32);
	glPopMatrix();

	//Drawing z-axis which is made up of a cylinder and a cone
	glPushMatrix();
		glScalef(1 / 10.0f, 1 / 10.0f, 1.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		gluCylinder(quad, 0.1f, 0.1f, 1.0f, 32, 32);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(0.0f, 0.0f, 1.0f);
		glScalef(1 / 10.0f, 1 / 10.0f, 1.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glutSolidCone(0.3, 0.1, 32, 32);
	glPopMatrix();
}

//Function to find the dimensions of the bounding volume to enclose the model renderred
void findBoundingVolDimensions()
{
	minX = minY = minZ = maxX = maxY = maxZ = 0.0f;

	for (size_t i = 0; i < vertices.size(); i++)
	{
		Vertex* v = vertices.at(i);
		if (v->x < minX)
			minX = v->x;
		if (v->x > maxX)
			maxX = v->x;

		if (v->y < minY)
			minY = v->y;
		if (v->y > maxY)
			maxY = v->y;

		if (v->z < minZ)
			minZ = v->z;
		if (v->z > maxZ)
			maxZ = v->z;
	}

	max = maxX;
	if (maxY > max)
		max = maxY;
	if (maxZ > max)
		max = maxZ;
}

//Function to draw the bounding volume to enclose the model renderred
void drawBoundingVol()
{
	glPushMatrix();
		glScalef(1 / (max - minX), 1 / (max - minY), 1 / (max - minZ));
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
		//Back face
		glBegin(GL_LINE_LOOP);
			glVertex3f(minX, minY, minZ);
			glVertex3f(maxX, minY, minZ);
			glVertex3f(maxX, maxY, minZ);
			glVertex3f(minX, maxY, minZ);
		glEnd();
		//Front face
		glBegin(GL_LINE_LOOP);
			glVertex3f(minX, minY, maxZ);
			glVertex3f(maxX, minY, maxZ);
			glVertex3f(maxX, maxY, maxZ);
			glVertex3f(minX, maxY, maxZ);
		glEnd();
		//Top face
		glBegin(GL_LINE_LOOP);
			glVertex3f(minX, maxY, maxZ);
			glVertex3f(maxX, maxY, maxZ);
			glVertex3f(maxX, maxY, minZ);
			glVertex3f(minX, maxY, minZ);
		glEnd();
		//Bottom face
		glBegin(GL_LINE_LOOP);
			glVertex3f(minX, minY, maxZ);
			glVertex3f(maxX, minY, maxZ);
			glVertex3f(maxX, minY, minZ);
			glVertex3f(minX, minY, minZ);
		glEnd();
	glPopMatrix();
}

//Function to draw model as points
void drawModelPoints()
{
	//For pint rendering, disabling the lighting would allow us to see the color better
	if (isLightEnabled)
	{
		glDisable(GL_LIGHTING);
		isLightEnabled = false;
	}

	glPushMatrix();
		glScalef(1 / (max - minX), 1 / (max - minY), 1 / (max - minZ));
		glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
		glBegin(GL_POINTS);
			for (size_t i = 0; i < vertices.size(); i++)
			{
				Vertex* v = vertices.at(i);

				glVertex3f(v->x, v->y, v->z);
			}
		glEnd();
	glPopMatrix();
}

//Function to draw model as wireframe
void drawModelWireframe()
{
	//For wireframe rendering, disabling the lighting would allow us to see the color better
	if (isLightEnabled)
	{
		glDisable(GL_LIGHTING);
		isLightEnabled = false;
	}

	for (size_t i = 0; i < faces.size(); i++)
	{
		Face* f = faces.at(i);
		Vertex* v1 = f->v1;
		Vertex* v2 = f->v2;
		Vertex* v3 = f->v3;

		glPushMatrix();
			glScalef(1 / (max - minX), 1 / (max - minY), 1 / (max - minZ));
			glBegin(GL_LINE_LOOP);
				glColor4f(1.0f, 0.0f, 1.0f, 1.0f); glVertex3f(v1->x, v1->y, v1->z);
				glColor4f(1.0f, 0.0f, 1.0f, 1.0f); glVertex3f(v2->x, v2->y, v2->z);
				glColor4f(1.0f, 0.0f, 1.0f, 1.0f); glVertex3f(v3->x, v3->y, v3->z);
			glEnd();
		glPopMatrix();
	}
}

//Function to draw model with flat shading
void drawModelFlat()
{
	//For flat shading, need to turn on light for better effect
	if (!isLightEnabled)
	{
		glEnable(GL_LIGHTING);
		isLightEnabled = true;
	}

	//Use flat shading mode
	glShadeModel(GL_FLAT);

	glPushMatrix();
		glScalef(1 / (max - minX), 1 / (max - minY), 1 / (max - minZ));
		glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
		glBegin(GL_TRIANGLES);
			for (size_t i = 0; i < faces.size(); i++)
			{
				Face* f = faces.at(i);
				Vertex* v1 = f->v1;
				Vertex* v2 = f->v2;
				Vertex* v3 = f->v3;
				Normal* n1 = perVertexNormals.at(v1->index - 1);
				Normal* n2 = perVertexNormals.at(v2->index - 1);
				Normal* n3 = perVertexNormals.at(v3->index - 1);

				glNormal3f(n1->x, n1->y, n1->z); glVertex3f(v1->x, v1->y, v1->z);
				glNormal3f(n2->x, n2->y, n2->z); glVertex3f(v2->x, v2->y, v2->z);
				glNormal3f(n3->x, n3->y, n3->z); glVertex3f(v3->x, v3->y, v3->z);
			}
		glEnd();
	glPopMatrix();
}

//Function to draw model with smooth shading
void drawModelSmooth()
{
	//For smooth shading, need to turn on light for better effect
	if (!isLightEnabled)
	{
		glEnable(GL_LIGHTING);
		isLightEnabled = true;
	}

	//Use flat shading mode
	glShadeModel(GL_SMOOTH);

	glPushMatrix();
		glScalef(1 / (max - minX), 1 / (max - minY), 1 / (max - minZ));
		glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
		glBegin(GL_TRIANGLES);
			for (size_t i = 0; i < faces.size(); i++)
			{
				Face* f = faces.at(i);
				Vertex* v1 = f->v1;
				Vertex* v2 = f->v2;
				Vertex* v3 = f->v3;
				Normal* n1 = perVertexNormals.at(v1->index - 1);
				Normal* n2 = perVertexNormals.at(v2->index - 1);
				Normal* n3 = perVertexNormals.at(v3->index - 1);
		
				glNormal3f(n1->x, n1->y, n1->z); glVertex3f(v1->x, v1->y, v1->z);
				glNormal3f(n2->x, n2->y, n2->z); glVertex3f(v2->x, v2->y, v2->z);
				glNormal3f(n3->x, n3->y, n3->z); glVertex3f(v3->x, v3->y, v3->z);
			}
		glEnd();
	glPopMatrix();
}

//Function to capture mouse information such as button clicked and position in window
void myMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		press_x = x;
		press_y = y;

		//When the middle button is held down, do translation
		if (button == GLUT_MIDDLE_BUTTON)
			xform_mode = TRANSFORM_TRANSLATE;
		else if (button == GLUT_RIGHT_BUTTON)
			xform_mode = TRANSFORM_SCALE;
		else if (button == GLUT_LEFT_BUTTON)
			xform_mode = TRANSFORM_ROTATE;
	}
	//If no button is held down, do not do any transformation
	else if (state == GLUT_UP)
		xform_mode = TRANSFORM_NONE;
}

//Function to capture mouse motion
void myMotion(int x, int y)
{
	if (xform_mode == TRANSFORM_TRANSLATE)
	{
		//Update the translation factor based on the difference of current mouse position and previous mouse position
		//Need to divide by 100.0f or else translation will be too wild
		tx += (x - press_x) / 100.0f;

		//For the y translation factor, we do a subtraction of y from press_y 
		//because in window coordinate system, y value increases as you go down
		ty += (press_y - y) / 100.0f;
	}
	else if (xform_mode == TRANSFORM_SCALE)
	{
		float old_size = scale_size;

		//We do a subtraction of y from press_y because in window coordinate system, y value increases as you go down
		//If we don't do the above, the camera will move further from object as we move the mouse upward
		//We want the camera to move closer to the object as we move the mouse upward instead
		scale_size *= (1 + (press_y - y) / 100.0f);

		if (scale_size < 0.0f)
			scale_size = old_size;
	}
	else if (xform_mode == TRANSFORM_ROTATE)
	{
		//Rotate about x-axis when mouse moves vertically
		if ((((x - press_x) <= 3.0f) || ((x - press_x) >= -3.0f)) && (y != press_y))
		{
			y_angle += 0.0f;
			z_angle += 0.0f;

			x_angle += (y - press_y) / 5.0f;
			if (x_angle > 180.0f)
				x_angle -= 360.0f;
			else if (x_angle < -180.0f)
				x_angle += 360.0f;
		}
		//Rotate about y-axis when mouse moves horizontally
		if ((((y - press_y) <= 3.0f) || ((y - press_y) >= -3.0f)) && (x != press_x))
		{
			x_angle += 0.0f;
			z_angle += 0.0f;

			y_angle += (x - press_x) / 5.0f;
			if (y_angle > 180.0f)
				y_angle -= 360.0f;
			else if (y_angle < -180.0f)
				y_angle += 360.0f;
		}
		//Rotate about z-axis when mouse moves circularly
		if ((((y - press_y) > 5.0f) || ((y - press_y) < -5.0f)) && (((x - press_x) > 5.0f) || ((x - press_x) < -5.0f)))
		{
			x_angle += 0.0f;
			y_angle += 0.0f;

			z_angle += ((x - press_x) + (press_y - y)) / 5.0f;
			if (z_angle > 180.0f)
				z_angle -= 360.0f;
			else if (z_angle < -180.0f)
				z_angle += 360.0f;
		}
	}

	press_x = x;
	press_y = y;

	//Force the redraw function
	glutPostRedisplay();
}