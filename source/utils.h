#pragma once

#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "OVR.h"

extern void trace(const char* format, ...);
void debug_break();

#define TRACE(A, ...) { trace("[TRACE] " A, __VA_ARGS__); }
#define WARN(A, ...) { trace("[WARN]  " A, __VA_ARGS__); }
#define FATAL(A, ...) { trace("[FATAL] " A, __VA_ARGS__); debug_break(); }
#define ASSERT(A) { if(!(A)) { trace("[ASSERT] " #A); debug_break(); } }

extern void gl_uniform_1f(int p, const char* varname, float a);
extern void gl_uniform_2f(int p, const char* varname, float a, float b);
extern void gl_uniform_3f(int p, const char* varname, float a, float b, float c);
extern void gl_uniform_4f(int p, const char* varname, float a, float b, float c, float d);
extern void gl_uniform_1i(int p, const char* varname, int value);

