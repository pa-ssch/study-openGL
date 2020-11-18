#include <glut.h>
#include <math.h>
#include <stdio.h>

#pragma warning(disable:4996)

#define ORTHO 1
#define PERSPECTIVE 2

// Width & height of texture
#define HEIGHT 512
#define WIDTH 512

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
void setProjection(int projType);
void setAntiAliasing(int state);

float vertices[3 * 60000];
float cvnormals[3 * 60000];
float ccolors[3 * 60000];
int triangle[10 * 3 * 60000];
float cnormals[10 * 3 * 60000];
int maxcoords = 0;
float verticesmax[3];
float verticesmin[3];
int vertices_n = 0;

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

GLubyte bitmapImage[HEIGHT][WIDTH][4];
int projType = PERSPECTIVE; // default: perspective projection
int lights = 0;
int shading = 0;
float shininess = 2;
int textureMode = 1;

// light components colors
float ambientLightColor[3] = { 0.1, 0.1, 0.1 };
float diffuseLightColor[3] = { 0.5, 0.5, 0.5 };
float specularLightColor[3] = { 1.0, 1.0, 1.0};

// light position
float lightPosition[4] = { 0, 0, 1, 1};

// short cut color white
float white[3] = { 0.5, 0.5, 0.5 };

int antiAliasing = 0;
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
	printf("'4' Points, color values by file\n'5' Wireframe, color values by file\n'6' Filled, color values by file\n'7' Filled, color values by file + texture\n\n\n");
	printf("Transformations:\n left mouse button and x-y movement -> rotation\n middle mouse button and y-direction -> zoom (scaling)\n");
	printf(" right mouse button and x-y movement -> translation\n\n");
	printf("Change projection:\n");
	printf("'o' orthographic projection, 'p' perspective projection \n\n");
	printf("Light option\n");
	printf("'s' Shading (Flat / Gouraud)\n");
	printf("'l' Toggle light\n");
	printf("'+'/'-' change specular exponent\n\n");
	printf("Texture options\n");
	printf("'t' change automatic texture (Object Linear / Eye Linear)\n\n");
	glutMainLoop();
	return 0;
}


void readBitmap(void) {
	int i, j, k;
	GLubyte c;

	FILE *img;
	img = fopen("./boneTexture.png", "rb");
	fseek(img, sizeof(unsigned char) * 54, 0); // offset to pixel data

	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
			for (k = 0; k < 3; k++) {
				fread(&c, sizeof(GLubyte), 1, img);
				bitmapImage[i][j][k] = (GLubyte)c;
			}
			bitmapImage[i][j][3] = (GLubyte)255;
		}
	}
	fclose(img);
}


void displaycloud(int modus)
{
	int i = 0;
	float range[3];
	float directionVector[3][2];
	float n[3];
	float currentColor[3];
	int counter = 0;

	glEnable(GL_NORMALIZE);
	glFrontFace(GL_CW);

	for (i = 0; i < 3; i++)
		range[i] = verticesmax[i] - verticesmin[i];


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
		if (modus == 3 || modus == 6 || modus == 7)
		{
			// Display filled polygons
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (modus == 7) {
			// Display texture
			glEnable(GL_TEXTURE_2D);
		}
		else {
			glDisable(GL_TEXTURE_2D);
		}

		// using the polygone mode "GL_TRIANGLES"
		glBegin(GL_TRIANGLES);

		for (i = 0; i < maxcoords + 1; i++)
		{
			if (modus > 3)
			{
				// Displaying colors saved in the mesh file (node wise definition!)
				currentColor[0] = ccolors[triangle[i] * 3];
				currentColor[1] = ccolors[triangle[i] * 3 + 1];
				currentColor[2] = ccolors[triangle[i] * 3 + 2];
			}
			else
			{ 
				// Displaying interpolated colors according to the x-/y-/z-value of the point coordinates (node wise definition!)
				currentColor[0] = (vertices[triangle[i] * 3] - verticesmin[0]) / range[0];
				currentColor[1] = (vertices[triangle[i] * 3 + 1] - verticesmin[1]) / range[1];
				currentColor[2] = (vertices[triangle[i] * 3 + 2] - verticesmin[2]) / range[2];
			}

			if (lights == 1) {
				// Material definition
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, currentColor);
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, currentColor);
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, currentColor);
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
			}
			else {
				glColor3f(currentColor[0], currentColor[1], currentColor[2]);
			}

			// for flat shading: one normal per triangle (before defintion of vertices) is sufficient
			// cnormals contains the surface normal
			if (counter == 0) {
				if (shading == 0) {
					glNormal3f(cnormals[i], cnormals[i + 1], cnormals[i + 2]);
				}
			}
			counter++;
			if (counter == 3) {
				counter = 0;
			}

			// for gouraud shading we need the normal of each vertex
			// cvnormals contains the vertex normals
			if (shading == 1) {
				glNormal3f(cvnormals[triangle[i] * 3], cvnormals[triangle[i] * 3 + 1], cvnormals[triangle[i] * 3 + 2]);
			}

			glVertex3f(vertices[triangle[i] * 3], vertices[triangle[i] * 3 + 1], vertices[triangle[i] * 3 + 2]);
		}
		glEnd();

	}

}
void display(void)
{
	// automatic generation of texture coordinates (object/eye linear)

	// Define s and t planes
	GLfloat s_plane[] = { 0,0,1,0 };
	GLfloat t_plane[] = { 0,1,0,0 };

	if (textureMode == 1) {
		// Object linear
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	}
	else if (textureMode == 2)
		{
		// eye linear
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	}

	glTexGenfv(GL_S, GL_OBJECT_PLANE, s_plane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, t_plane);

	if (lights == 1) {
		// light definition
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLightColor);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLightColor);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specularLightColor);
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

		glEnable(GL_LIGHTING);

		// shading definition
		if (shading == 0)
		{ 
			// Flat Shading
			glShadeModel(GL_FLAT);
		}
		else if (shading == 1) 
		{ 
			// Gouraud Shading
			glShadeModel(GL_SMOOTH);
		}
	}
	else {
		glDisable(GL_LIGHTING);
	}
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

	glPushMatrix();

	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glColor3f(0.0, 0.0, 0.0);
	// center and rotate
	glTranslatef(xoff, yoff, 0);
	glRotatef(angle2, 1.0, 0.0, 0.0);
	glRotatef(angle1, 0.0, 1.0, 0.0);
	
	//display
	displaycloud(displaymodus);
	// draw box
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(verticesmax[0], verticesmax[1], verticesmax[2]);
	glVertex3f(verticesmin[0], verticesmax[1], verticesmax[2]);
	glVertex3f(verticesmin[0], verticesmin[1], verticesmax[2]);
	glVertex3f(verticesmax[0], verticesmin[1], verticesmax[2]);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(verticesmax[0], verticesmax[1], verticesmin[2]);
	glVertex3f(verticesmin[0], verticesmax[1], verticesmin[2]);
	glVertex3f(verticesmin[0], verticesmin[1], verticesmin[2]);
	glVertex3f(verticesmax[0], verticesmin[1], verticesmin[2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(verticesmax[0], verticesmax[1], verticesmax[2]);
	glVertex3f(verticesmax[0], verticesmax[1], verticesmin[2]);
	glVertex3f(verticesmin[0], verticesmax[1], verticesmax[2]);
	glVertex3f(verticesmin[0], verticesmax[1], verticesmin[2]);
	glVertex3f(verticesmin[0], verticesmin[1], verticesmax[2]);
	glVertex3f(verticesmin[0], verticesmin[1], verticesmin[2]);
	glVertex3f(verticesmax[0], verticesmin[1], verticesmax[2]);
	glVertex3f(verticesmax[0], verticesmin[1], verticesmin[2]);
	glEnd();
	glPopMatrix();
	glPopMatrix();

	// Buffer for animation needs to be swapped
	glutSwapBuffers();
}

void init(void)
{

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glClearColor(0.99, 0.99, 0.99, 0.0);
	glLoadIdentity();
	xoff = 0.0;
	yoff = 0.0;
	zoff = 0.0;
	zoom = 1;
	angle1 = 45;
	angle2 = 45;

	// Read bitmap file
	readBitmap();

	// Texture wrap settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Filter settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Connecting lighting and texture
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Initialize texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmapImage);

	// enable automatic texture generation
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
}



void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT);
}

void readcloud(char* filename)
{
	int i = 0;
	int j = 0;
	FILE* file;
	int k = 0;
	int numVertices = 0;
	int counter = 0;
	float directionVector[3][2];
	float n[3];
	float x, y, z;
	float temp;
	int index;
	int indexBegin;
	int numNeighbouringFaces = 0;
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
			verticesmax[i % 3] = vertices[i];
			verticesmin[i % 3] = vertices[i];
		}

		// Stop if all points are 0, (works only if the origin is not a valid point)
		if (i > 3 && vertices[i - 2] == 0 && vertices[i - 1] == 0 && vertices[i] == 0)
			abbruch = 1;

		// Renew extreme values if necessary
		if (vertices[i] > verticesmax[i % 3] && vertices[i] != 0)
			verticesmax[i % 3] = vertices[i];
		if (vertices[i] < verticesmin[i % 3] && vertices[i] != 0)
			verticesmin[i % 3] = vertices[i];
		i++;
	}
	vertices_n = i - 1;
	printf("Read %i Vertices\n", vertices_n / 3);
	printf("the coordinates are in the intervals  [%f,%f]  [%f,%f] [%f,%f]\n\n", verticesmin[0], verticesmax[0], verticesmin[1], verticesmax[1], verticesmin[2], verticesmax[2]);
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

	for (j = 0; j < vertices_n; j++) {
		// normalize
		vertices[j] = vertices[j] - verticesmin[j % 3];
		vertices[j] = 2 * vertices[j] / (verticesmax[j % 3] - verticesmin[j % 3]);
		vertices[j] = vertices[j] - 1;
	}

	verticesmin[0] = -1;
	verticesmin[1] = -1;
	verticesmin[2] = -1;

	verticesmax[0] = 1;
	verticesmax[1] = 1;
	verticesmax[2] = 1;

	for (j = 0; j < vertices_n; j++)
	{
		if (j % 3 != 1)
			continue;

		// swap y with z
		float swap = vertices[j];
		vertices[j] = vertices[j + 1];
		vertices[j + 1] = swap;
	}
	printf("Calculate surface and vertex normals...\n");
	counter = 0;
	for (i = 0; i < maxcoords + 1; i++) {
		if (counter == 0) {
			directionVector[0][0] = vertices[triangle[i + 1] * 3] - vertices[triangle[i] * 3];
			directionVector[1][0] = vertices[triangle[i + 1] * 3 + 1] - vertices[triangle[i] * 3 + 1];
			directionVector[2][0] = vertices[triangle[i + 1] * 3 + 2] - vertices[triangle[i] * 3 + 2];

			directionVector[0][1] = vertices[triangle[i + 2] * 3] - vertices[triangle[i] * 3];
			directionVector[1][1] = vertices[triangle[i + 2] * 3 + 1] - vertices[triangle[i] * 3 + 1];
			directionVector[2][1] = vertices[triangle[i + 2] * 3 + 2] - vertices[triangle[i] * 3 + 2];

			n[0] = (directionVector[1][0] * directionVector[2][1]) - (directionVector[2][0] * directionVector[1][1]);
			n[1] = (directionVector[2][0] * directionVector[0][1]) - (directionVector[0][0] * directionVector[2][1]);
			n[2] = (directionVector[0][0] * directionVector[1][1]) - (directionVector[1][0] * directionVector[0][1]);

			cnormals[i] = n[0];
			cnormals[i + 1] = n[1];
			cnormals[i + 2] = n[2];

			cvnormals[triangle[i] * 3] = cvnormals[triangle[i] * 3] + n[0];
			cvnormals[triangle[i] * 3 + 1] = cvnormals[triangle[i] * 3 + 1] + n[1];
			cvnormals[triangle[i] * 3 + 2] = cvnormals[triangle[i] * 3 + 2] + n[2];

			cvnormals[triangle[i + 1] * 3] = cvnormals[triangle[i + 1] * 3] + n[0];
			cvnormals[triangle[i + 1] * 3 + 1] = cvnormals[triangle[i + 1] * 3 + 1] + n[1];
			cvnormals[triangle[i + 1] * 3 + 2] = cvnormals[triangle[i + 1] * 3 + 2] + n[2];

			cvnormals[triangle[i + 2] * 3] = cvnormals[triangle[i + 2] * 3] + n[0];
			cvnormals[triangle[i + 2] * 3 + 1] = cvnormals[triangle[i + 2] * 3 + 1] + n[1];
			cvnormals[triangle[i + 2] * 3 + 2] = cvnormals[triangle[i + 2] * 3 + 2] + n[2];
		}
		counter++;
		if (counter == 3) {
			counter = 0;
		}
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
	case 'l':
		if (lights == 0)
			lights = 1;
		else
			lights = 0;
		break;
	case '+':
		shininess += 0.5;
		printf("Shininess: %f\n", shininess);
		break;
	case '-':
		shininess -= 0.5;
		printf("Shininess: %f\n", shininess);
		break;
	case 's':
		if (shading == 1) {
			shading = 0;
			printf("Shading = FLAT\n");
		}
		else if (shading == 0) {
			shading = 1;
			printf("Shading = GOURAUD\n");
		}
		break;
	case 't':
		if (textureMode == 1) {
			textureMode = 2;
			printf("Texture Mode = EYE LINEAR\n");
		}
		else if (textureMode == 2) {
			textureMode = 1;
			printf("Texture Mode = OBJECT LINEAR\n");
		}
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		displaymodus = k - '0';
		printf("Display mode: %i\n", displaymodus);
		break;
	default:
		printf("Key %c (%i) is not a valid input\n", k, k);
		break;
	}
	glutPostRedisplay();
}