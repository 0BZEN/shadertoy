#include "utils.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "libovr.lib")

void trace(const char* format, ...) 
{
    char* temp = new char[65536];
    va_list args;
    va_start(args, format);
    vsprintf_s(temp, 65536, format, args);
    va_end(args);

    OutputDebugString(temp);
    OutputDebugString("\n");

    std::ofstream fout("log.txt", std::ofstream::out | std::ofstream::app);
    std::cout << temp << std::endl;
    fout << temp << std::endl;

    delete[] temp;
}

void debug_break() 
{ 
    __asm int 3; 
}

void gl_uniform_1f(int p, const char* varname, float a)								{ GLuint varid = glGetUniformLocation(p, varname); glUniform1f(varid, a); }
void gl_uniform_2f(int p, const char* varname, float a, float b)					{ GLuint varid = glGetUniformLocation(p, varname); glUniform2f(varid, a, b); }
void gl_uniform_3f(int p, const char* varname, float a, float b, float c)			{ GLuint varid = glGetUniformLocation(p, varname); glUniform3f(varid, a, b, c); }
void gl_uniform_4f(int p, const char* varname, float a, float b, float c, float d)	{ GLuint varid = glGetUniformLocation(p, varname); glUniform4f(varid, a, b, c, d); }
void gl_uniform_1i(int p, const char* varname, int value)							{ GLuint varid = glGetUniformLocation(p, varname); glUniform1i(varid, value); }
