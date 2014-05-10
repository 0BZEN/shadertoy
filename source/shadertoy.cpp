
#include "utils.h"
#include "oculus.h"

// oculus device
OculusDevice oculus;

// display.
DEVMODE dmScreenSettings;

// render buffer.
GLboolean	frame_buffer_enabled = true;
GLint		frame_buffer_width;
GLint		frame_buffer_height;
GLuint		frame_buffer = 0;
GLuint		frame_buffer_texture = 0;
GLint		frame_time;
GLint		frame_count;

// fragment shader.
GLuint		fragment_shader_program = 0;

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

GLint load_fragment_shader_program(const char* fragment_file_path)
{
	std::list<GLint> shaders;
	shaders.push_back(load_shader(fragment_file_path, GL_FRAGMENT_SHADER));
	GLint program_id = build_program(shaders);
	delete_shaders(shaders);
	return program_id;
}

void update_uniform_params_buffer(OVR::Util::Render::StereoEye eye)
{
	if(eye == OVR::Util::Render::StereoEye_Center)
	{
		// In your code load your shaders, link your program and call:
		gl_uniform_3f(fragment_shader_program, "iOffset",	  0.0f, 0.0f, 0.0f);
		gl_uniform_3f(fragment_shader_program, "iResolution", (float)frame_buffer_width, (float)frame_buffer_height, 0.0f);
		gl_uniform_1f(fragment_shader_program, "iGlobalTime", (float)frame_time / 1000.0f);
	}
	else if(eye == OVR::Util::Render::StereoEye_Left)
	{
		// In your code load your shaders, link your program and call:
		gl_uniform_3f(fragment_shader_program, "iOffset",	  0.0f, 0.0f, 0.0f);
		gl_uniform_3f(fragment_shader_program, "iResolution", (float)frame_buffer_width / 2, (float)frame_buffer_height, 0.0f);
		gl_uniform_1f(fragment_shader_program, "iGlobalTime", (float)frame_time / 1000.0f);
	}
	else if(eye == OVR::Util::Render::StereoEye_Right)
	{
		// In your code load your shaders, link your program and call:
		gl_uniform_3f(fragment_shader_program, "iOffset",	  (float)frame_buffer_width / 2, 0.0f, 0.0f);
		gl_uniform_3f(fragment_shader_program, "iResolution", (float)frame_buffer_width / 2, (float)frame_buffer_height, 0.0f);
		gl_uniform_1f(fragment_shader_program, "iGlobalTime", (float)frame_time / 1000.0f);
	}
	else
	{
		FATAL("invalid eye(%d)", eye);
	}
}

void scene_to_buffer(void)
{
	if(frame_buffer_enabled)
	{
		glViewport(0, 0, frame_buffer_width, frame_buffer_height);

		if(oculus.has_display())
		{
			// render to buffer.
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			glUseProgram(fragment_shader_program);

			update_uniform_params_buffer(OVR::Util::Render::StereoEye_Left);
			glRecti(-1, -1, 0, 1);

			update_uniform_params_buffer(OVR::Util::Render::StereoEye_Right);
			glRecti(0, -1, 1, 1);
		}
		else
		{
			// render to buffer.
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			glUseProgram(fragment_shader_program);

			update_uniform_params_buffer(OVR::Util::Render::StereoEye_Center);
			glRecti(-1, -1, 1, 1);
		}
	}
}

void update_uniform_params_screen()
{
    gl_uniform_3f(fragment_shader_program, "iResolution", (float)dmScreenSettings.dmPelsWidth, (float)dmScreenSettings.dmPelsHeight, 0.0f);
    gl_uniform_1f(fragment_shader_program, "iGlobalTime", (float)frame_time / 1000.0f);
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
	else if(oculus.has_display())
	{
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frame_buffer_texture);

		oculus.render(OVR::Util::Render::StereoEye_Left);
		oculus.render(OVR::Util::Render::StereoEye_Right);
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

HWND create_window()
{
	PIXELFORMATDESCRIPTOR pfd = { 0,1,PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0 };

	//ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	HWND hWND = CreateWindow("edit",0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, dmScreenSettings.dmPosition.x, dmScreenSettings.dmPosition.y, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight, 0, 0, 0, 0);
	HDC hDC = GetDC(hWND);
    SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd) , &pfd);
    wglMakeCurrent(hDC, wglCreateContext(hDC));
    ShowCursor(0);
	return hWND;
}

 
void main(int argc, char* argv[])
{
    time_t now = time(0);
    tm time_now;
    localtime_s(&time_now, &now);
	char date_now[128];
	asctime_s(date_now, sizeof(date_now), &time_now);

    const char* shader_file = (argc > 1)? argv[1] : "elevated";
	int displayId = (argc > 2)? atoi(argv[2]) : 1;
    frame_buffer_width = (argc > 3)? atoi(argv[3]) : 800;
    frame_buffer_height = (argc > 4)? atoi(argv[4]) : 600;

	char fragment_shader_filename[128];
	sprintf_s(fragment_shader_filename, sizeof(fragment_shader_filename), "./shaders/%s.shader", shader_file);
	
	setup_display(displayId);
    
    TRACE("------------------------------------------------------");
    TRACE("- SHADERTOY");
	TRACE("---------------------");
    TRACE("- display resolution : %d x %d", dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight);
    TRACE("- frame buffer       : %d x %d", frame_buffer_width, frame_buffer_height);
    TRACE("- shader             : %s", fragment_shader_filename);
    TRACE("- date               : %s", date_now);
	
	HWND hWND = create_window();
	HDC hDC = GetDC(hWND);    

	init_opengl();

	oculus.start();

    create_buffer();

	fragment_shader_program = load_fragment_shader_program(fragment_shader_filename);

    int frame_start = GetTickCount();	
	frame_count = 0;

    while(true)
    {
        frame_time = (GetTickCount() - frame_start);
		frame_count++;

        scene_to_buffer();

        buffer_to_display();

		if (GetAsyncKeyState(VK_SNAPSHOT)) 
		{
			screen_capture_to_clipboard(hWND);
		}

        if (GetAsyncKeyState(VK_ESCAPE)) 
		{
			TRACE("- average : %.3f fps", frame_count / (float)(frame_time / 1000.0f));
			TRACE("------------------------------------------------------");
			oculus.stop();
            ExitProcess(0);
		}

		SwapBuffers(hDC);

        Sleep(1);
    }
};
