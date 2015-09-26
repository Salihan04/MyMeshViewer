#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "glut.h"

using namespace std;

//Function prototypes
void renderScene();
void parseFile(string fileName);
void populateVertices();
void populateFaces();
void resetVectors();

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
	Vertex* vert;
	Face* face;
}HalfEdge;
typedef struct
{
	Vertex* vert;
	HalfEdge* he;
}HE_vert;
typedef struct
{
	Face* face;
	HalfEdge* he;
}HE_face;
typedef struct
{
	HE_vert* vert;	//Vertex at the end of the half-edge
	HalfEdge* pair;	//Oppositely oriented half-edge
	HE_face* face;	//The incident face
	HalfEdge* prev;	//Previous half-edge around the face
	HalfEdge* next;	//Next half-edge around the face
}HE_edge;

vector<string> vertices_string;
vector<string> faces_string;
vector<Vertex*> vertices;
vector<Face*> faces;

int main(int argc, char **argv)
{
	//for (int i = 0; i < sizeof(testModels) / sizeof(testModels[0]); i++)
	//{
	//	string filename = "TestModels/" + testModels[i];

	//	//Parse M file
	//	parseFile(filename);

	//	//Print info of file
	//	cout << "Model: " << testModels[i] << endl;
	//	cout << "No. of vertices: " << vertices.size() << endl;
	//	cout << "No. of faces: " << faces.size() << endl;
	//	cout << endl;

	//	//Clear vectors
	//	vertices.clear();
	//	faces.clear();
	//}

//	cout << "Cap" << endl;
	parseFile("TestModels/cap.m");
	populateVertices();
	populateFaces();

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

	return 0;
}

//Function to draw items in scene
void renderScene()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 1.0, 1.0);	//Blue background

	//Swap the buffers
	glutSwapBuffers();
	//Force the redraw function
	glutPostRedisplay();
}

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

//Function to populate the vertices vector based on data from model file
void populateVertices()
{
	for (int n = 0; n < vertices_string.size(); n++)
	{
		int j = 0;
		float coords[3];
		string temp = "";
		Vertex* v = new Vertex();

		//The coordinates typically start at string index 10 till end of string
		for (int i = 10; i < vertices_string.at(n).length(); i++)
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

//Function to populate the faces vector based on data from model file
void populateFaces()
{
	for (int n = 0; n < faces_string.size(); n++)
	{
		int j = 0;
		int vertexIndex[3];
		string temp = "";
		Face* f = new Face();

		//The vertex indices typically start at string index 8 till end of string
		for (int i = 8; i < faces_string.at(n).length(); i++)
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
//		cout << "Face " << f->index << ": " << f->v1->index << "," << f->v2->index << "," << f->v3->index << endl;
	}
}

//Function to reset vectors
void resetVectors()
{
	vertices_string.clear();
	faces_string.clear();
	vertices.clear();
	faces.clear();
}