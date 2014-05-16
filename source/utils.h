#pragma once

#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <limits>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "OVR.h"
#include "sdl.h"

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
extern void gl_uniform_1fv(int p, const char* varname, int count, const float* values);
extern void gl_uniform_3fv(int p, const char* varname, int count, const float* values);
extern void gl_uniform_4fv(int p, const char* varname, int count, const float* values);
extern void gl_uniform_mat4f(int p, const char* varname, bool transpose, const float* values);

extern void gl_mult_matrix(const OVR::Matrix4f& matrix);

extern GLint load_shader(const char* file_path, GLint shader_type, const char* shader_inputs=NULL);
extern GLint build_program(const std::list<GLint>& shaders);
extern void delete_shaders(const std::list<GLint>& shaders);

void attach_opengl_debug_callbacks();
bool save_screenshot_tga(const char* filename);