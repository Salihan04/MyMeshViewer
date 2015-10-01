#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "glut.h"

using namespace std;

static int window;
static string testModels[] = { "bimba.m",  "bottle.m", "bunny.m", "cap.m", "eight.m", "gargoyle.m", "knot.m", "statute.m" };

//Half-edge data structures, assuming counter-clockwise orientation
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
	Vertex* vert;	//Vertex at the end of the half-edge
	Face* face;		//The incident face
}HalfEdge;
/*typedef struct
{
	Vertex* v;	
	HalfEdge* he;	//One of the half-edges emanating from the vertex
}HE_vert;
typedef struct
{
	Face* f;
	HalfEdge* he;	//One of the half-edges bordering the face
}HE_face;
typedef struct
{
	HalfEdge* he;
	HE_vert* vert;	//Vertex at the end of the half-edge
	HalfEdge* pair;	//Oppositely oriented half-edge
	HE_face* face;	//The incident face
	HalfEdge* prev;	//Previous half-edge around the face
	HalfEdge* next;	//Next half-edge around the face
}HE_edge;*/
typedef struct
{
	float x, y, z;
}Vector;
typedef struct
{
	float x, y, z;
}Normal;

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
//vector<HE_vert*> HE_verts;
//vector<HE_face*> HE_faces;
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
void addToHE_verts(HE_vert* vert);
void addToHE_faces(HE_face* face);
Normal* faceNormal(Face* f);
void initPerFaceNormals();
vector<Face*> findAdjFaces(Vertex* v);
float calcFaceArea(Normal* n);
Normal* vertexNormal(Vertex* v);
void initPerVertexNormals();
void resetVectors();

int main(int argc, char **argv)
{
	/*
	for (int i = 0; i < sizeof(testModels) / sizeof(testModels[0]); i++)
	{
		string filename = "TestModels/" + testModels[i];

		//Parse M file
		parseFile(filename);

		//Print info of file
		cout << "Model: " << testModels[i] << endl;
		cout << "No. of vertices: " << vertices.size() << endl;
		cout << "No. of faces: " << faces.size() << endl;
		cout << endl;

		//Clear vectors
		vertices.clear();
		faces.clear();
	}
	*/

//	cout << "Cap" << endl;
	parseFile("TestModels/cap.m");
	initVertices();
	initFaces();
	
	initHEMaps();
	assocPairToEdge();

	//Initialising Glut
	glutInit(&argc, argv);
	//Use double buffer to get better results on animation
	//Use depth buffer for hidden surface removal
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);

	//Create window with OpenGL
	window = glutCreateWindow("Muhammad Salihan Bin Zaol-kefli, CZ4004 MeshViewer");

	//Register callbacks
	glutDisplayFunc(renderScene);

	glutMainLoop();

	//Reset vectors
	resetVectors();

	system("pause");

	return 0;
}

//Function to draw items in scene
void renderScene()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);	//Black background

	//Swap the buffers
	glutSwapBuffers();
	//Force the redraw function
	glutPostRedisplay();
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
		cout << "Cannot open dummy.txt" << endl;

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
		float coords[3];
		string temp = "";
		Vertex* v = new Vertex();

		//The coordinates typically start at string index 10 till end of string
		for (size_t i = 10; i < vertices_string.at(n).length(); i++)
		{
			//Need to take into account of space characters between x, y, and z coordinate
			//For some vertex, coordinates start at a later string index depending on vertex index
			if (vertices_string.at(n)[i] == ' ')
			{
				//When a space character is encountered, check if temp is empty string
				//If temp is not empty, add to array of cordinates and then reset temp
				if (temp != "")
				{
					coords[j] = atof(temp.c_str());
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
		int vertexIndex[3];
		string temp = "";
		Face* f = new Face();

		//The vertex indices typically start at string index 8 till end of string
		for (size_t i = 8; i < faces_string.at(n).length(); i++)
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
	while (curr->pair->next != out_edge)
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

//Function to reset vectors
void resetVectors()
{
	vertices_string.clear();
	faces_string.clear();
	vertices.clear();
	faces.clear();
	HE_verts.clear();
	HE_faces.clear();
}