#include <iostream>
#include <fstream>
#include <string>

#include "glut.h"

using namespace std;

//Function prototypes
void renderScene();
void parseFile(char* fileName);

static int window;

int main(int argc, char **argv)
{
	/*parseFile("dummy.txt");
	cout << endl;*/

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

	//system("pause");

	return 0;
}

void renderScene()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 1.0, 1.0);	//Blue background

	glutSwapBuffers();
	glutPostRedisplay();
}

void parseFile(char* fileName)
{
	string str;
	ifstream infile;
	infile.open(fileName);

	if (!infile.is_open())
	{
		cout << "Cannot open dummy.txt" << endl;
	}
	
	while (infile)
	{
		getline(infile, str);
		if (infile)
			cout << str << endl;
	}

	infile.close();
}