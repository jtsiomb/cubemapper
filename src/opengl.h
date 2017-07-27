#ifndef OPENGL_H_
#define OPENGL_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>
#endif

bool init_opengl();

#endif	// OPENGL_H_
