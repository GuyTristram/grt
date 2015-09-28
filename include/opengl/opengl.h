#ifndef OPENGL_H
#define OPENGL_H

#ifdef _WIN32
#define NOMINMAX
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include "GL3/gl3w.h"
#include <gl/gl.h>
#else
#include <GLES2/gl2.h>
#endif

#endif // OPENGL_H
