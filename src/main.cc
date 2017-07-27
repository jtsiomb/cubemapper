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

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(1024, 768);
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

static void display()
{
	app_draw();
}

static void reshape(int x, int y)
{
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
