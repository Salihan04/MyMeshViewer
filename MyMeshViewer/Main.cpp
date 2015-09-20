#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "glut.h"

using namespace std;

//Function prototypes
void renderScene();
void parseFile(char* fileName);

static int window;
vector<string> vertices;
vector<string> faces;

int main(int argc, char **argv)
{
	//Parse M file
	parseFile("TestModels/cap.m");
	
	cout << "No. of vertices: " << vertices.size() << endl;
	cout << "No. of faces: " << faces.size() << endl;
	/*
	//Print data from vertices vector
	for (int i = 0; i < vertices.size(); i++)
		cout << vertices.at(i) << endl;
	//Print data from faces vector
	for (int i = 0; i < faces.size(); i++)
		cout << faces.at(i) << endl;
	*/

	/*
	//The below code is just to test converting a string to a float
	string str = "777";
	float num = atof(str.c_str());	//Convert string to float
	num = num + 1.2f;
	cout << num << endl;
	*/

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

void parseFile(char* fileName)
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
				vertices.push_back(str);
			//If the line begins with 'Face', add to faces vector
			else if (str[0] == 'F')
				faces.push_back(str);
		}
	}

	//Close file
	infile.close();
}