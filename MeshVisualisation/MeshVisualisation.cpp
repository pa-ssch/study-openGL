#include <glut.h>
#include <math.h>
#include <stdio.h>

#pragma warning(disable:4996)

#define ORTHO 1
#define PERSPECTIVE 2

void mouse(int button, int state, int x, int y);
void key(unsigned char key, int x, int y);
void init(void);
void reshape(int, int);
void display(void);
int main(int, char**);
void define_menu();
void idle();
void timer(int value);
void readcloud(char* filename);
void mouseactive(int x, int y);
void mouse(int button, int state, int x, int y);

float vertices[3 * 60000];
float ccolors[3 * 60000];
int triangle[10 * 3 * 60000];
int maxcoords = 0;
float cpointsmax[3];
float cpointsmin[3];
int cpoints_n = 0;

float xoff;
float yoff;
float zoff;
float zoom;
int angle1;
int angle2;

const float stepsize = 0.05;
const float anglestepsize = 0.01;
int displaymodus = 1;
int pressedbutton = 0;
int startx, starty, startz;
int startangle1;
int startangle2;
float startxoff;
float startyoff;
float startzoff;

int projType = PERSPECTIVE; // default: perspective projection

int main(int argc, char** argv)
{
	char path[] = "./bones.txt";
	readcloud(path);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(400, 100);
	glutCreateWindow("Mesh Visualisation");
	init();
	glutMouseFunc(mouse);
	glutMotionFunc(mouseactive);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	printf("\n\CONTROL\nDisplay modes:\n");
	printf("'0' only the Box\n'1' Points, color values by coordinate\n'2' Wireframe, color values by coordinate\n'3' Filled, color values by coordinate\n");
	printf("'4' Points, color values by file\n'5' Wireframe, color values by file\n'6' Filled, color values by file\n\n\n");
	printf("Transformations:\n left mouse button and x-y movement -> rotation\n middle mouse button and y-direction -> zoom (scaling)\n");
	printf(" right mouse button and x-y movement -> translation\n\n");
	printf("Change projection:\n");
	printf("'o' orthographic projection, 'p' perspective projection \n\n");
	glutMainLoop();
	return 0;
}

void displaycloud(int modus)
{
	int i = 0;
	float range[3];
	for (i = 0; i < 3; i++)
		range[i] = cpointsmax[i] - cpointsmin[i];
	if (modus > 0)
	{
		if (modus == 1 || modus == 4)
		{
			// Display only the vertices
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		}
		if (modus == 2 || modus == 5)
		{
			// Display the outlines of the polygons
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		if (modus == 3 || modus == 6)
		{
			// Display filled polygons
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// using the polygone mode "GL_TRIANGLES"
		glBegin(GL_TRIANGLES);

		for (i = 0; i < maxcoords + 1; i++)
		{
			if (modus > 3)
			{
				// Displaying colors saved in the mesh file (node wise definition!)
				glColor3f(ccolors[triangle[i] * 3], ccolors[triangle[i] * 3 + 1], ccolors[triangle[i] * 3 + 2]);
			}
			else
			{ 
				// Displaying interpolated colors according to the x-/y-/z-value of the point coordinates (node wise definition!)
				glColor3f((vertices[triangle[i] * 3] - cpointsmin[0]) / range[0], (vertices[triangle[i] * 3 + 1] - cpointsmin[1]) / range[1], (vertices[triangle[i] * 3 + 2] - cpointsmin[2]) / range[2]);
			}

			glVertex3f(vertices[triangle[i] * 3], vertices[triangle[i] * 3 + 1], vertices[triangle[i] * 3 + 2]);
		}
		glEnd();

	}

}
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (projType) {
	case ORTHO:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-2 - zoff, 2 + zoff, -2 - zoff, 2 + zoff, 3, 7);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		break;

	case PERSPECTIVE:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, 1.0, 3.0, 7.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0, 0.0, 5.0 + zoff, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		break;
	}

	glColor3f(0.0, 0.0, 0.0);
	// center and rotate
	glTranslatef(xoff, yoff, 0);
	glRotatef(angle2, 1.0, 0.0, 0.0);
	glRotatef(angle1, 0.0, 1.0, 0.0);
	
	//display
	displaycloud(displaymodus);
	// draw box
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(cpointsmax[0], cpointsmax[1], cpointsmax[2]);
	glVertex3f(cpointsmin[0], cpointsmax[1], cpointsmax[2]);
	glVertex3f(cpointsmin[0], cpointsmin[1], cpointsmax[2]);
	glVertex3f(cpointsmax[0], cpointsmin[1], cpointsmax[2]);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(cpointsmax[0], cpointsmax[1], cpointsmin[2]);
	glVertex3f(cpointsmin[0], cpointsmax[1], cpointsmin[2]);
	glVertex3f(cpointsmin[0], cpointsmin[1], cpointsmin[2]);
	glVertex3f(cpointsmax[0], cpointsmin[1], cpointsmin[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(cpointsmax[0], cpointsmax[1], cpointsmax[2]);
	glVertex3f(cpointsmax[0], cpointsmax[1], cpointsmin[2]);
	glVertex3f(cpointsmin[0], cpointsmax[1], cpointsmax[2]);
	glVertex3f(cpointsmin[0], cpointsmax[1], cpointsmin[2]);
	glVertex3f(cpointsmin[0], cpointsmin[1], cpointsmax[2]);
	glVertex3f(cpointsmin[0], cpointsmin[1], cpointsmin[2]);
	glVertex3f(cpointsmax[0], cpointsmin[1], cpointsmax[2]);
	glVertex3f(cpointsmax[0], cpointsmin[1], cpointsmin[2]);
	glEnd();

	// Buffer for animation needs to be swapped
	glutSwapBuffers();
}

void init(void)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.99, 0.99, 0.99, 0.0);
	glLoadIdentity();
	xoff = 0.0;
	yoff = 0;
	zoff = 0.0;
	zoom = 1;
	angle1 = 90;
	angle2 = 0;
}


void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
}

void timer(int value)
{
}

void readcloud(char* filename)
{
	int i = 0;
	int j = 0;
	FILE* file;
	int abbruch = 0;
	char str[200] = "";

	printf("Read file '%s' \n", filename);
	fopen_s(&file, filename, "r");
	printf("Skip first Line\n");
	while (!feof(file) && str[0] != '[')
		fscanf(file, "%s", str);
	printf("Read verticy\n");

	// Read verticies
	while (!feof(file) && abbruch == 0)
	{
		if (((i + 1) % 3) == 0)
			fscanf(file, "%f %c", &vertices[i], str);
		else
			fscanf(file, "%f", &vertices[i]);

		// Init extreme values
		if (i < 3)
		{
			cpointsmax[i % 3] = vertices[i];
			cpointsmin[i % 3] = vertices[i];
		}

		// Stop if all points are 0, (works only if the origin is not a valid point)
		if (i > 3 && vertices[i - 2] == 0 && vertices[i - 1] == 0 && vertices[i] == 0)
			abbruch = 1;

		// Renew extreme values if necessary
		if (vertices[i] > cpointsmax[i % 3] && vertices[i] != 0)
			cpointsmax[i % 3] = vertices[i];
		if (vertices[i] < cpointsmin[i % 3] && vertices[i] != 0)
			cpointsmin[i % 3] = vertices[i];
		i++;
	}
	cpoints_n = i - 1;
	printf("Read %i Vertices\n", cpoints_n / 3);
	printf("the coordinates are in the intervals  [%f,%f]  [%f,%f] [%f,%f]\n\n", cpointsmin[0], cpointsmax[0], cpointsmin[1], cpointsmax[1], cpointsmin[2], cpointsmax[2]);
	abbruch = 0; i = 0;

	while (!feof(file) && str[0] != '[')
		fscanf(file, "%s", str);
	printf("Read colors\n");

	while (!feof(file) && abbruch == 0)
	{
		if (((i + 1) % 3) == 0)
			fscanf(file, "%f %c", &ccolors[i], str);
		else
			fscanf(file, "%f", &ccolors[i]);
		// Stop if all colors are 0, (works only if the origin is not a valid point)
		if (i > 3 && ccolors[i - 2] == 0 && ccolors[i - 1] == 0 && ccolors[i] == 0)
			abbruch = 1;
		i++;
	}
	printf("Read %i colors\n\n", (i - 1) / 3);
	abbruch = 0; i = 0;

	while (!feof(file) && str[0] != '[')
		fscanf(file, "%s", str);

	printf("Read coordinates for the triangles...\n");
	while (!feof(file) && abbruch < 2)
	{
		fscanf_s(file, "%i %c", &triangle[i], str);
		// Stop if all triangles are 0, (works only if the origin is not a valid point)
		if (triangle[i] == -1)
		{
			i--;
			abbruch++;
		}
		else
			abbruch = 0;
		i++;
	}
	maxcoords = i - 1;
	printf("Read %i triangles\n", (maxcoords + 1) / 3);
	fclose(file);
	printf("End read data \n\n");

	for (j = 0; j < cpoints_n; j++) {
		// normalize
		vertices[j] = vertices[j] - cpointsmin[j % 3];
		vertices[j] = 2 * vertices[j] / (cpointsmax[j % 3] - cpointsmin[j % 3]);
		vertices[j] = vertices[j] - 1;
	}

	cpointsmin[0] = -1;
	cpointsmin[1] = -1;
	cpointsmin[2] = -1;

	cpointsmax[0] = 1;
	cpointsmax[1] = 1;
	cpointsmax[2] = 1;

	for (j = 0; j < cpoints_n; j++)
	{
		if (j % 3 != 1)
			continue;

		// swap y with z
		float swap = vertices[j];
		vertices[j] = vertices[j + 1];
		vertices[j + 1] = swap;
	}
}

void key(unsigned char k, int x, int y);
void mouseactive(int x, int y)
{
	if (pressedbutton == GLUT_LEFT_BUTTON)
	{
		angle1 = startangle1 + (x - startx) / 10;
		angle2 = startangle2 + (y - starty) / 10;
	}
	if (pressedbutton == GLUT_RIGHT_BUTTON)
	{
		xoff = startxoff + (float)(x - startx) / 100;
		yoff = startyoff + (float)(y - starty) / 100;
	}
	if (pressedbutton == GLUT_MIDDLE_BUTTON)
	{
		zoff = startzoff + ((float)(y - startz) / 100);
	}
	glutPostRedisplay();
}
void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		pressedbutton = button;
		startx = x;
		starty = y;
		startz = y;
		startangle1 = angle1;
		startangle2 = angle2;
		startxoff = xoff;
		startyoff = yoff;
		startzoff = zoff;
	}
	else
		pressedbutton = 0;

}


void MainMenu(int value)
{
	switch (value) {

	case 2:
		key('q', 0, 0);
		break;

	}
}

void
submenu1(int value)
{
}


void define_menu()
{
}

void key(unsigned char k, int x, int y)
{
	switch (k) {
	case 8:  //BACKSPACE
		init();
		break;
	case 27:
	case 'q':
	case 'Q':
		exit(0);
	case 'o':
		projType = ORTHO;
		printf("Projektion: ORTHOGRAPHIC\n");
		glutPostRedisplay();
		break;
	case 'p':
		projType = PERSPECTIVE;
		printf("Projektion: PERSPECTIVE\n");
		glutPostRedisplay();
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
		displaymodus = k - '0';
		printf("Display mode: %i\n", displaymodus);
		break;
	default:
		printf("Key %c (%i) is not a valid input\n", k, k);
		break;
	}
	glutPostRedisplay();
}