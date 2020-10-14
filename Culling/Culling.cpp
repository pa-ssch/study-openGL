#include <glut.h>


const int width = 800;
const int height = 600;

void init(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glViewport(0, 0, width, height);
	glutCreateWindow("Culling");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
	gluLookAt(0.25, 0.5, 0.1, 0, 0, 0, 0, 1, 0);
}


void display(void)
{

	glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glBegin(GL_QUADS);

	glColor3f(1.0F, 0.0F, 0.0F);
	// FRONT
	glVertex3f(-0.5F, -0.5F, 0.5F);
	glVertex3f(0.5F, -0.5F, 0.5F);
	glVertex3f(0.5F, 0.5F, 0.5F);
	glVertex3f(-0.5F, 0.5F, 0.5F);
	// BACK
	glVertex3f(-0.5F, -0.5F, -0.5F);
	glVertex3f(-0.5F, 0.5F, -0.5F);
	glVertex3f(0.5F, 0.5F, -0.5F);
	glVertex3f(0.5F, -0.5F, -0.5F);

	glColor3f(0.0F, 1.0F, 0.0F);
	// LEFT
	glVertex3f(-0.5F, -0.5F, 0.5F);
	glVertex3f(-0.5F, 0.5F, 0.5F);
	glVertex3f(-0.5F, 0.5F, -0.5F);
	glVertex3f(-0.5F, -0.5F, -0.5F);
	// RIGHT
	glVertex3f(0.5F, -0.5F, -0.5F);
	glVertex3f(0.5F, 0.5F, -0.5F);
	glVertex3f(0.5F, 0.5F, 0.5F);
	glVertex3f(0.5F, -0.5F, 0.5F);

	glColor3f(0.0F, 0.0F, 1.0F);
	// BOTTOM
	glVertex3f(-0.5F, -0.5F, 0.5F);
	glVertex3f(-0.5F, -0.5F, -0.5F);
	glVertex3f(0.5F, -0.5F, -0.5F);
	glVertex3f(0.5F, -0.5F, 0.5F);
	glEnd();

	glFlush();
}


int main(int argc, char** argv)
{
	init(argc, argv);
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

