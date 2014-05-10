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


GLint load_shader(const char* file_path, GLint shader_type)
{
	TRACE("loading shader '%s'...", file_path);

    // Create the shaders
	GLuint shader_id = glCreateShader(shader_type);
    
    // Read the Fragment Shader code from the file
    std::string shader_code;
    std::ifstream shader_stream(file_path, std::ios::in);
    if(shader_stream.is_open())
    {
        std::string Line = "";
        while(getline(shader_stream, Line))
            shader_code += "\n" + Line;
        shader_stream.close();
    }
    else
    {
        FATAL("could not to open %s.", file_path);
        return 0;
    }

    // Compile Fragment Shader
    TRACE("compiling shader (%d)...", shader_id);
    char const * shader_code_name = shader_code.c_str();
    glShaderSource(shader_id, 1, &shader_code_name , NULL);
    glCompileShader(shader_id);

    // Check Fragment Shader
    GLint result = GL_FALSE;
    int info_log_length;

	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if ( result == GL_FALSE )
    {
        std::vector<char> message(info_log_length+1);
        glGetShaderInfoLog(shader_id, info_log_length, NULL, &message[0]);
        FATAL("%s", &message[0]);

		glDeleteShader(shader_id);
		shader_id = 0;
    }
	else
	{
		TRACE("shader(%d) built from '%s'.", shader_id, file_path);
	}

	return shader_id;
}

GLint build_program(const std::list<GLint>& shaders)
{
	TRACE("creating program (%d shaders)...", shaders.size());
    GLuint program_id = glCreateProgram();
	
    TRACE("attaching shaders (%d shaders)...", shaders.size());    
	for(std::list<GLint>::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
		if(*it > 0) glAttachShader(program_id, *it);
	
	TRACE("linking program (%d shaders)...", shaders.size());
    glLinkProgram(program_id);

    // Check the program
	GLint result = GL_FALSE;
    int info_log_length;

    glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetShaderiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if ( result == GL_FALSE )
    {
		std::vector<char> message(info_log_length+1);
        glGetProgramInfoLog(program_id, info_log_length, NULL, &message[0]);
        FATAL("%s", &message[0]);
		
		glDeleteProgram(program_id);
		program_id = 0;
    }

	if(program_id > 0)
	{	
		TRACE("program(%d) built (%d shaders).", program_id, shaders.size());
	}
    return program_id;
}

void delete_shaders(const std::list<GLint>& shaders)
{
	TRACE("deleting shaders (%d shaders)...", shaders.size());
	for(std::list<GLint>::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
		glDeleteShader(*it);
}

void screen_capture_to_clipboard(HWND hWND)
{
	if ( OpenClipboard(hWND) )
	{
		EmptyClipboard();

		HDC hDC = GetDC(hWND);

		// and a device context to put it in
		HDC hMemoryDC = CreateCompatibleDC(hDC);

		int width = GetDeviceCaps(hDC, HORZRES);
		int height = GetDeviceCaps(hDC, VERTRES);

		// maybe worth checking these are positive values
		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, width, height);

		// get a new bitmap
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

		BOOL result = BitBlt(hMemoryDC, 0, 0, width, height, hDC, 0, 0, SRCCOPY);

		hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

		HANDLE handle = SetClipboardData(CF_BITMAP, hBitmap);
	
		result = CloseClipboard();

		// clean up
		DeleteDC(hMemoryDC);
	}
}