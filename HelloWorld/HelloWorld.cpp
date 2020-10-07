#include <glut.h>

const int width = 640;
const int height = 480;

void init(int argc, char** argv) {
	// Init GLUT Lib
	glutInit(&argc, argv);

	// Init Window
	glutInitDisplayMode(GLUT_SINGLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Hello world");

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
}

void display()
{
	const char* display_text = "Hello World!";

	glClearColor(0.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex3f((width / 2) - (width / 4), (height / 2) - (height / 4), 0.0);
	glVertex3f((width / 2) + (width / 4), (height / 2) - (height / 4), 0.0);
	glVertex3f((width / 2) + (width / 4), (height / 2) + (height / 4), 0.0);
	glVertex3f((width / 2) - (width / 4), (height / 2) + (height / 4), 0.0);
	glEnd();

	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i((width / 2) - (width / 4) + 10, (height / 2) - (height / 4) + 10);
	for (int j = 0; j < strlen(display_text); j++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, display_text[j]);

	glFlush();
}


int main(int argc, char** argv)
{
	init(argc, argv);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}
