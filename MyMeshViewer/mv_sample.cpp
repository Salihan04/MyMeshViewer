/* 
	What's new in this program?

  1. this program supports the mouse-based camera control. 
       to rotate the object, click the left button and move the mouse
	   to scale the object, click the middle button and move the mouse in y-direction, i.e., up and down

  2. this program draws the objects (cube, torus and teapot) in three modes, wireframe, solid, edges+solid
       press 'w', 's', and 'e' to set the display modes 
       
*/

#include <iostream>
#include "glut.h"
#include <cstdlib>
using namespace std;

#define TRANSFORM_NONE    0 
#define TRANSFORM_ROTATE  1
#define TRANSFORM_SCALE 2 

#define OBJ_WIREFRAME	0
#define OBJ_SOLID		1
#define OBJ_EDGE		2 

static int win;
static int menid;
static int submenid;
static int primitive = 0;

static int press_x, press_y; 
static float x_angle = 0.0; 
static float y_angle = 0.0; 
static float scale_size = 1; 

static int obj_mode = 0;
static int xform_mode = 0; 

void menu(int value)
{
	if (value == 0)
	{
		glutDestroyWindow(win);
		exit(0);
	}
	else
	{
		primitive=value;
	}
  
	// you would want to redraw now
	glutPostRedisplay();
}

void createmenu(void)
{
	// Create a submenu, this has to be done first.
	submenid = glutCreateMenu(menu);

	// Add sub menu entry
	glutAddMenuEntry("Teapot", 2);
	glutAddMenuEntry("Cube", 3);
	glutAddMenuEntry("Torus", 4);

	// Create the menu, this menu becomes the current menu
	menid = glutCreateMenu(menu);

	// Create an entry
	glutAddMenuEntry("Clear", 1);

	glutAddSubMenu("Draw", submenid);
	// Create an entry
	glutAddMenuEntry("Quit", 0);

	// Let the menu respond on the right mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void disp(void)
{
	glEnable(GL_DEPTH_TEST); 

	// Just clean the screen
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// setup the perspective projection
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	gluPerspective(60, 1, .1, 100); 

	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 
	gluLookAt(0,0,5,0,0,0,0,1,0); 

	// rotate and scale the object
	glRotatef(x_angle, 0, 1,0); 
	glRotatef(y_angle, 1,0,0); 
	glScalef(scale_size, scale_size, scale_size); 

	// draw what the user asked
	if (primitive == 1)
	{
		glutPostRedisplay();
	}
	else if (primitive == 2)
	{
		if (obj_mode == OBJ_WIREFRAME)
		{
			glColor3f(0, 0, 1);
			glutWireTeapot(0.5);
		}
		else if (obj_mode == OBJ_SOLID)
		{
			glColor3f(0, 0, 1);
			glutSolidTeapot(0.5);
		}
		else
		{
			// set the color for the solid teapot
			glColor3f(0, 0, 1); 
			 // draw a solid teapot
			glutSolidTeapot(0.5);  
			// set the color for the edges
			glColor3f(1, 0, 0); 
			// store the existing line width
			double width;
			glGetDoublev(GL_LINE_WIDTH, &width);
			// set the current line width to 2
			glLineWidth(2); 
			// draw the teapot again
			glutWireTeapot(0.501);   
			// restore the line width
			glLineWidth(width); 
		}
	}
	else if (primitive == 3)
	{
		if (obj_mode == OBJ_WIREFRAME)
		{
			glColor3f(0, 0, 1); 
			glutWireCube(0.5);
		}
		else if (obj_mode == OBJ_SOLID)
		{
			glColor3f(0, 0, 1); 
			glutSolidCube(0.5);
		}
		else 
		{
			glColor3f(0, 0, 1); 
			glutSolidCube(0.5);  
			glColor3f(1, 0, 0); 
			double width;
			glGetDoublev(GL_LINE_WIDTH, &width);
			glLineWidth(2); 
			glutWireCube(0.501);   
			glLineWidth(width); 
		}
	}
	else if (primitive == 4)
	{
		if (obj_mode == OBJ_WIREFRAME)
		{
			glColor3f(0, 0, 1); 
			glutWireTorus(0.3, 0.6, 100, 100);
		}
		else if (obj_mode == OBJ_SOLID)
		{
			glColor3f(0, 0, 1);			
			glutSolidTorus(0.3, 0.6, 100, 100);
		}
		else
		{
			glColor3f(0, 0, 1); 
			glutSolidTorus(0.3, 0.6, 100, 100);  
			glColor3f(1, 0, 0); 
			double width;
			glGetDoublev(GL_LINE_WIDTH, &width);
			glLineWidth(2); 
			glutWireTorus(0.305, 0.605, 100, 100);   
			glLineWidth(width); 
		}
	}
  
	// swap the buffers
	glutSwapBuffers(); 
}


void mymouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) 
	{
		press_x = x; press_y = y; 
		if (button == GLUT_LEFT_BUTTON)
			xform_mode = TRANSFORM_ROTATE; 
		else if (button == GLUT_MIDDLE_BUTTON) 
			xform_mode = TRANSFORM_SCALE; 
	}
	else if (state == GLUT_UP) 
	{
		xform_mode = TRANSFORM_NONE; 
	}
}

void mymotion(int x, int y)
{
	if (xform_mode == TRANSFORM_ROTATE) 
	{
		x_angle += (x - press_x)/5.0; 

		if (x_angle > 180) 
			x_angle -= 360; 
		else if (x_angle <-180) 
			x_angle += 360; 
      
		press_x = x; 
	   
		y_angle += (y - press_y)/5.0; 

		if (y_angle > 180) 
			y_angle -= 360; 
		else if (y_angle <-180) 
			y_angle += 360; 
      
		press_y = y; 
    }
	else if (xform_mode == TRANSFORM_SCALE)
	{
		float old_size = scale_size;
		
		scale_size *= (1 + (y - press_y)/60.0); 

		if (scale_size <0) 
			scale_size = old_size; 
		press_y = y; 
    }

	// force the redraw function
	glutPostRedisplay(); 
}

void mykey(unsigned char key, int x, int y)
{
	switch(key) 
	{
	case 'w': 
		cout << "key 'w' is pressed! draw the object in wireframe" << endl;
		obj_mode = OBJ_WIREFRAME;
		break; 
	case 's':
		cout << "key 's' is pressed! draw the object in solid" << endl;
		obj_mode = OBJ_SOLID;
		break;
	case 'e':
		cout << "key 'e' is pressed! draw the object in solid+wireframe" << endl;
		obj_mode = OBJ_EDGE;
		break;
	}

	// force the redraw function
	glutPostRedisplay(); 
}


int main(int argc, char **argv)
{
	// normal initialisation
	glutInit(&argc, argv);
	// use double buffer to get better results on animation
	// use depth buffer for hidden surface removal
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	
	glutInitWindowSize(500,500);
	glutInitWindowPosition(100,100);

	win = glutCreateWindow("GLUT Transformation");
  
	// put all the menu functions in one nice procedure
	createmenu();

	// set the clearcolor and the callback
	glClearColor(0.0,0.0,0.0,0.0);

	// register your callback functions
	glutDisplayFunc(disp);
	glutMouseFunc(mymouse);
	glutMotionFunc(mymotion);
	glutKeyboardFunc(mykey);

	// enter the main loop
	glutMainLoop();

	return 1;
}
