#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <time.h>
#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

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

#define TRACE(A, ...) { trace("[TRACE] " A, __VA_ARGS__); }
#define WARN(A, ...) { trace("[WARN]  " A, __VA_ARGS__); }
#define FATAL(A, ...) { trace("[FATAL] " A, __VA_ARGS__); debug_break(); }
#define ASSERT(A) { if(!(A)) { trace("[ASSERT] " #A); debug_break(); } }


GLuint load_shaders(const char * vertex_file_path, const char * fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else
    {
        FATAL("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !", vertex_file_path);
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    else
    {
        FATAL("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !", fragment_file_path);
        return 0;
    }


    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    TRACE("compiling shader : %s", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
    {
        char* Info = new char[InfoLogLength + 1];
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, Info);
        WARN("%s", Info);
        delete[] Info;
    }

    // Compile Fragment Shader
    TRACE("compiling shader : %s", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
    {
        char* Info = new char[InfoLogLength + 1];
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, Info);
        WARN("%s", Info);
        delete[] Info;
    }

    // Link the program
    TRACE("linking program...");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
    {
        char* Info = new char[InfoLogLength + 1];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, Info);
        WARN("%s", Info);
        delete[] Info;
    }
	else
	{
		TRACE("shaders program compiled.");
	}

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

// screen size.
DEVMODE dmScreenSettings = { 0,0,0,sizeof(DEVMODE),0,DM_PELSWIDTH|DM_PELSHEIGHT, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1024, 768,0,0,0,0,0,0,0,0,0,0 };

// buffer size.
GLboolean frame_buffer_enabled = true;
GLint  frame_buffer_width;
GLint  frame_buffer_height;
GLuint frame_buffer = 0;            // hidden frame buffer.
GLuint frame_buffer_texture = 0;    // we'll be using the frame buffer as a texture for a full screen quad.
GLuint fragment_shader_program = 0;
GLint  frame_time;
GLint  frame_count;

void init_opengl()
{
	glewInit();

	glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void create_buffer()
{
	if(frame_buffer_enabled)
	{
		glGenFramebuffers(1, &frame_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

		// The texture we're going to slap onto the full screen polygon.
		glGenTextures(1, &frame_buffer_texture);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, frame_buffer_texture);

		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_buffer_width, frame_buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set "renderedTexture" as our colour attachement #0
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frame_buffer_texture, 0);

		// add a depth buffer(?).
		GLuint depth_buffer;
		glGenRenderbuffers(1, &depth_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, frame_buffer_width, frame_buffer_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

		// Set the fragments target, same as the frame buffer color attachment #0. use gl_FragData[0] in shader, not gl_FragColor.
		GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, draw_buffers); // "1" is the size of DrawBuffers

		// Always check that our framebuffer is ok.
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			FATAL("[FRAMEBUFFER] error.");
		}
	}
}

GLint load_fragment_shader(const char* fragment_file_path)
{
	TRACE("loading shader '%s'...", fragment_file_path);

    // Create the shaders
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    else
    {
        FATAL("could not to open %s.", fragment_file_path);
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Fragment Shader
    TRACE("compiling shader...");
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
    {
        char* Info = new char[InfoLogLength + 1];
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, Info);
        WARN("%s", Info);
        delete[] Info;
    }

    // Link the program
    TRACE("linking program...");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
    {
        char* Info = new char[InfoLogLength + 1];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, Info);
		WARN("program error : %s", Info);
        delete[] Info;
    }
	else
	{
		TRACE("shader '%s' loaded.", fragment_file_path);
	}

	glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void update_uniform_params_buffer()
{
    // In your code load your shaders, link your program and call:
    int iResolution = glGetUniformLocation(fragment_shader_program, "iResolution");
    int iGlobalTime = glGetUniformLocation(fragment_shader_program, "iGlobalTime");
    glUniform3f(iResolution, (float)frame_buffer_width, (float)frame_buffer_height, 0.0f);
    glUniform1f(iGlobalTime, (float)frame_time / 1000.0f);
}

void scene_to_buffer(void)
{
	if(frame_buffer_enabled)
	{
		glViewport(0, 0, frame_buffer_width, frame_buffer_height);

		// render to buffer.
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		glUseProgram(fragment_shader_program);

		update_uniform_params_buffer();
	    
		// render full quad, using the selected fragment shader.
		glRecti(-1, -1, 1, 1);
	}
}

void update_uniform_params_screen()
{
    // In your code load your shaders, link your program and call:
    int iResolution = glGetUniformLocation(fragment_shader_program, "iResolution");
    int iGlobalTime = glGetUniformLocation(fragment_shader_program, "iGlobalTime");
	glUniform3f(iResolution, (float)dmScreenSettings.dmPelsWidth, (float)dmScreenSettings.dmPelsHeight, 0.0f);
    glUniform1f(iGlobalTime, (float)frame_time / 1000.0f);
}

void buffer_to_display(void)
{
	glViewport(0, 0, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight);
    
	if(!frame_buffer_enabled)
	{
		update_uniform_params_screen();
		glUseProgram(fragment_shader_program);
		glRecti(-1, -1, 1, 1);
	}
	else
	{
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frame_buffer_texture);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(-1,  1);
		glTexCoord2f(1.0f, 1.0f); glVertex2i( 1,  1);
		glTexCoord2f(1.0f, 0.0f); glVertex2i( 1, -1);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(-1, -1);
		glEnd();
	}
}

void setup_display(int display_id)
{
	DWORD DispNum = 0;
	DWORD DispCount = 0;
    DISPLAY_DEVICE  DisplayDevice;
	bool  display_found = false;

    // initialize DisplayDevice
    ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
    DisplayDevice.cb = sizeof(DisplayDevice);

    // get all display devices
    while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0))
	{
	    DEVMODE defaultMode;
        ZeroMemory(&defaultMode, sizeof(DEVMODE));
        defaultMode.dmSize = sizeof(DEVMODE);
    
		if ( EnumDisplaySettings((LPSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode) )
		{
			int mask = (DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL);
				
			if((defaultMode.dmFields & mask) == mask && 
				defaultMode.dmPelsWidth > 0 && 
				defaultMode.dmPelsHeight > 0 && 
				defaultMode.dmBitsPerPel > 0) 
			{
				DispCount++;

				TRACE("display #%02d [%s] : %dx%dx%dbpp@%dhz", DispCount, DisplayDevice.DeviceName, defaultMode.dmPelsWidth, defaultMode.dmPelsHeight, defaultMode.dmBitsPerPel, defaultMode.dmDisplayFrequency);

				if(display_id == DispCount)
				{
					TRACE("using Display #%02d [%s]", DispNum, DisplayDevice.DeviceName);
					dmScreenSettings = defaultMode;
					display_found = true;
				}
			}
		}
		DispNum++;
	}

	if(!display_found)
	{
		ZeroMemory(&dmScreenSettings, sizeof(dmScreenSettings));
		dmScreenSettings.dmDisplayFlags = (DM_PELSWIDTH|DM_PELSHEIGHT);
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = 1024;
		dmScreenSettings.dmPelsHeight = 768;
		TRACE("display #%02d not found. Using default mode (%d x %d)", display_id, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight);
	}
}

HDC create_window()
{
	PIXELFORMATDESCRIPTOR pfd = { 0,1,PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0 };

	//ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	HDC hDC = GetDC(CreateWindow("edit",0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, dmScreenSettings.dmPosition.x, dmScreenSettings.dmPosition.y, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight, 0, 0, 0, 0));
    SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd) , &pfd);
    wglMakeCurrent(hDC, wglCreateContext(hDC));
    ShowCursor(0);
	return hDC;
}

void main(int argc, char* argv[])
{
    time_t now = time(0);
    tm time_now;
    localtime_s(&time_now, &now);

    int displayId = (argc >= 2)? atoi(argv[2]) : 3;
    const char* fragment_shader_filename = (argc >= 3)? argv[3] : "./shaders/cartoon.shader";
	frame_buffer_width = (argc >= 4)? atoi(argv[3]) : 640;
    frame_buffer_height = (argc >= 5)? atoi(argv[4]) : 480;
	
	setup_display(displayId);
    
    TRACE("------------------------------------------------------");
    TRACE("- SHADERTOY");
	TRACE("---------------------");
    TRACE("- display resolution : %d x %d", dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight);
    TRACE("- frame buffer       : %d x %d", frame_buffer_width, frame_buffer_height);
    TRACE("- shader             : %s", fragment_shader_filename);
    TRACE("- date               : %s", asctime(&time_now));
	
	HDC hDC = create_window();
    
	init_opengl();

    create_buffer();

	fragment_shader_program = load_fragment_shader(fragment_shader_filename);

    int frame_start = GetTickCount();	
	frame_count = 0;

    while(true)
    {
        frame_time = (GetTickCount() - frame_start);
		frame_count++;

        scene_to_buffer();

        buffer_to_display();

        SwapBuffers(hDC);

        Sleep(1);

        if (GetAsyncKeyState(VK_ESCAPE)) 
		{
			TRACE("- average : %.3f fps", frame_count / (float)(frame_time / 1000.0f));
			TRACE("------------------------------------------------------");
            ExitProcess(0);
		}
    }
};
