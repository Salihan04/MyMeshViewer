#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
typedef struct
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
}HE_edge;
typedef struct
{
	float x, y, z;
}Vector;
typedef struct
{
	float x, y, z;
}Normal;

/*
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
*/

vector<string> vertices_string;
vector<string> faces_string;
vector<Vertex*> vertices;
vector<Face*> faces;
vector<HE_vert*> HE_verts;
vector<HE_face*> HE_faces;
vector<Normal*> perFaceNormals;
vector<Normal*> perVertexNormals;

//Function prototypes
void renderScene();
void parseFile(string fileName);
void populateVertices();
void populateFaces();
void initHEDataStructs();
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
	populateVertices();
	populateFaces();

	initHEDataStructs();
	cout << "Vertices: " << vertices.size() << "," << HE_verts.size() << endl;
	cout << "Faces: " << faces.size() << "," << HE_faces.size() << endl;

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

/*
Function to populate the vertices vector based on data from model file
In the for loops, you will see I used size_t to declare my iterator
This is to resolve warnings produced if I declared my iterators with int instead
Its a personal preference. I don't like errors and warnings
*/
void populateVertices()
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
//		cout << "Vertex " << v->index << ": " << v->x << "," << v->y << "," << v->z << endl;
	}
}

/*
Function to populate the faces vector based on data from model file
In the for loops, you will see I used size_t to declare my iterator
This is to resolve warnings produced if I declared my iterators with int instead
Its a personal preference. I don't like errors and warnings
*/
void populateFaces()
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
//		cout << "Face " << f->index << ": " << f->v1->index << "," << f->v2->index << "," << f->v3->index << endl;
	}
}

/*
Function to initialise the half edge data structures
In the for loops, you will see I used size_t to declare my iterator
This is to resolve warnings produced if I declared my iterators with int instead
Its a personal preference. I don't like errors and warnings
*/
void initHEDataStructs()
{
	for (size_t n = 0; n < faces.size(); n++)
	{
		HalfEdge* he = new HalfEdge();
		HalfEdge* pair = new HalfEdge();
		HalfEdge* prev = new HalfEdge();
		HalfEdge* next = new HalfEdge();
		HE_vert* vert = new HE_vert();
		HE_face* face = new HE_face();
		HE_edge* edge = new HE_edge();
		Face* f = faces.at(n);

		he->vert = f->v1;
		he->face = f;

		//Initialising HE_vert
		vert->v = f->v1;
		vert->he = he;
		addToHE_verts(vert);

		//Initialising HE_face
		face->f = f;
		face->he = he;
		addToHE_faces(face);

		pair->vert = f->v2;
		for (size_t i = 0; i < faces.size(); i++)
		{
			if (faces.at(i)->v1 == pair->vert)
				pair->face = faces.at(i);
		}

		next->vert = f->v2;
		next->face = f;

		prev->vert = f->v3;
		prev->face = f;

		//Initialising HE_edge
		edge->he = he;
		edge->vert = vert;
		edge->face = face;
		edge->pair = pair;
		edge->prev = prev;
		edge->next = next;


		he = new HalfEdge();
		vert = new HE_vert();
		pair = new HalfEdge();
		prev = new HalfEdge();
		next = new HalfEdge();
		edge = new HE_edge();

		he->vert = f->v2;
		he->face = f;

		vert->v = f->v2;
		vert->he = he;
		addToHE_verts(vert);

		pair->vert = f->v3;
		for (size_t i = 0; i < faces.size(); i++)
		{
			if (faces.at(i)->v1 == pair->vert)
				pair->face = faces.at(i);
		}

		next->vert = f->v3;
		next->face = f;

		prev->vert = f->v1;
		prev->face = f;

		edge->he = he;
		edge->vert = vert;
		edge->face = face;
		edge->pair = pair;
		edge->prev = prev;
		edge->next = next;


		he = new HalfEdge();
		vert = new HE_vert();
		pair = new HalfEdge();
		prev = new HalfEdge();
		next = new HalfEdge();
		edge = new HE_edge();

		he->vert = f->v3;
		he->face = f;

		vert->v = f->v3;
		vert->he = he;
		addToHE_verts(vert);

		pair->vert = f->v1;
		for (size_t i = 0; i < faces.size(); i++)
		{
			if (faces.at(i)->v1 == pair->vert)
				pair->face = faces.at(i);
		}

		next->vert = f->v1;
		next->face = f;

		prev->vert = f->v2;
		prev->face = f;

		edge->he = he;
		edge->vert = vert;
		edge->face = face;
		edge->pair = pair;
		edge->prev = prev;
		edge->next = next;
	}
}

//Function to append to HE_verts vector
void addToHE_verts(HE_vert* vert)
{
	//If HE_verts is empty, append vert
	if (HE_verts.size() == 0)
		HE_verts.push_back(vert);
	//HE_verts is not empty
	else
	{
		//Check that vert is not already in HE_verts before appending
		for (size_t i = 0; i < HE_verts.size(); i++)
		{
			if (HE_verts.at(i)->v->index == vert->v->index)
				return;
		}
		HE_verts.push_back(vert);
	}
}

//Function to append to HE_faces vector
void addToHE_faces(HE_face* face)
{
	//If HE_faces is empty, append face
	if (HE_faces.size() == 0)
		HE_faces.push_back(face);
	//HE_faces is not empty
	else
	{
		//Check that face is not already in HE_faces before appending
		for (size_t i = 0; i < HE_faces.size(); i++)
		{
			if (HE_faces.at(i)->f->index == face->f->index)
				return;
		}
		HE_faces.push_back(face);
	}
}

Normal* faceNormal(Face* f)
{
	Vector* v1 = new Vector();
	Vector* v2 = new Vector();
	Normal* n = new Normal();

	v1->x = f->v1->x - f->v2->x;
	v1->y = f->v1->y - f->v2->y;
	v1->z = f->v1->z - f->v2->z;

	v2->x = f->v3->x - f->v2->x;
	v2->y = f->v3->y - f->v2->y;
	v2->z = f->v3->z - f->v2->z;

	n->x = (v1->y * v2->z) - (v1->z * v2->y);
	n->y = (v1->z * v2->x) - (v1->x - v2->z);
	n->z = (v1->x * v2->y) - (v1->y * v2->x);

	return n;
}

void initPerFaceNormals()
{
	for (size_t i = 0; i < faces.size(); i++)
	{
		Normal* n = faceNormal(faces.at(i));
		perFaceNormals.push_back(n);
	}
}

vector<Face*> findAdjFaces(Vertex* v)
{
	vector<Face*> adjFaces;

	for (size_t i = 0; i < faces.size(); i++)
	{
		Face* f = faces.at(i);
		if ((f->v1->index == v->index) || (f->v2->index == v->index) || (f->v2->index == v->index))
			adjFaces.push_back(f);
	}

	return adjFaces;
}

float calcFaceArea(Normal* n)
{
	float length = sqrt((n->x * n->x) + (n->y * n->y) + (n->z * n->z));

	return length / 2;
}

Normal* vertexNormal(Vertex* v)
{
	float totalArea = 0.0f;
	float faceArea;
	vector<float> faceAreas;
	Normal* n = new Normal();
	n->x = 0.0f;
	n->y = 0.0f;
	n->z = 0.0f;

	vector<Face*> adjFaces = findAdjFaces(v);

	for (size_t i = 0; i < adjFaces.size(); i++)
	{
		Face* f = adjFaces.at(i);
		int index = f->index;
		Normal* faceNormal = perFaceNormals.at(index - 1);
		faceArea = calcFaceArea(faceNormal);
		faceAreas.push_back(faceArea);
		totalArea += faceArea;
	}

	for (size_t j = 0; j < adjFaces.size(); j++)
	{
		Face* f = adjFaces.at(j);
		int index = f->index;
		Normal* faceNormal = perFaceNormals.at(index - 1);

		float weight = faceAreas.at(j) / totalArea;
		n->x += weight * faceNormal->x;
		n->y += weight * faceNormal->y;
		n->z += weight * faceNormal->z;
	}

	return n;
}

void initPerVertexNormals()
{
	for (size_t i = 0; i < vertices.size(); i++)
	{
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