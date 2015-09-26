#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "glut.h"

using namespace std;

//Function prototypes
void renderScene();
void parseFile(string fileName);

static int window;
static string testModels[] = { "bimba.m",  "bottle.m", "bunny.m", "cap.m", "eight.m", "gargoyle.m", "knot.m", "statute.m" };

//Half-edge data structures, assuming counter-clockwise orientation
typedef struct
{
	float x, y, z;	//The vertex coordinates
}Vertex;
typedef struct
{
	Vertex* vert1, vert2, vert3;
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

	cout << "Cap" << endl;
	parseFile("TestModels/cap.m");

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

	return 0;
}

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
			{
				int i = 10;
				int j = 0;
				string temp = "";
				Vertex* v = new Vertex();
				float coords[3];
				for (int i = 10; i < str.length(); i++)
				{
					if (str[i] == ' ')
					{
						if (temp != "")
						{
							coords[j] = atof(temp.c_str());
							j++;
							temp = "";
						}
						else
						{
							continue;
						}
					}
					else
					{
						temp += str[i];
					}
				}
				if (temp != "")
				{
					coords[j] = atof(temp.c_str());
					j = 0;
					temp = "";
				}
				v->x = coords[0];
				v->y = coords[1];
				v->z = coords[2];
				vertices.push_back(v);
				cout << v->x << "," << v->y << "," << v->z << endl;
			}
			//If the line begins with 'Face', add to faces vector
			//else if (str[0] == 'F')
				//faces.push_back(str);
		}
	}

	//Close file
	infile.close();
}