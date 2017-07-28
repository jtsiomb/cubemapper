#include <stdlib.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "app.h"

static void display();
static void reshape(int x, int y);
static void keydown(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static int win_width, win_height;

int main(int argc, char **argv)
{
	glutInitWindowSize(1024, 768);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutCreateWindow("cubemapper");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	if(!app_init(argc, argv)) {
		return 1;
	}

	glutMainLoop();
	return 0;
}

void app_quit()
{
	app_cleanup();
	exit(0);
}

void app_redisplay()
{
	glutPostRedisplay();
}

void app_swap_buffers()
{
	glutSwapBuffers();
}

void app_resize(int x, int y)
{
	glutReshapeWindow(x, y);
}

void app_print_text(int x, int y, const char *str)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, 0, win_height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glRasterPos2i(x, y);

	while(*str) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str++);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static void display()
{
	app_draw();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	app_reshape(x, y);
}

static void keydown(unsigned char key, int x, int y)
{
	app_keyboard(key, true);
}

static void mouse(int bn, int st, int x, int y)
{
	app_mouse_button(bn - GLUT_LEFT_BUTTON, st == GLUT_DOWN, x, y);
}

static void motion(int x, int y)
{
	app_mouse_motion(x, y);
}
