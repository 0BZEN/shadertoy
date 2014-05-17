
#include "utils.h"
#include "oculus.h"
#include "body.h"
#include "texture.h"

// oculus device
OculusDevice oculus;

// textures 
Textures textures;

// body
Body body(OVR::Vector3f(0.0f, 0.0f, 0.0f), OVR::Vector3f(0.0f, 0.0f, 0.0f));

// display.
DEVMODE dmScreenSettings;

struct ShaderInputs
{
	struct Vector2f
	{
		float x, y;
		Vector2f(float ix, float iy) { x = ix; y = iy; }
		Vector2f() { x = y = 0; }
	};

	struct Vector3f
	{
		float x, y, z;
		Vector3f(float ix, float iy, float iz) { x = ix; y = iy; z = iz; }
		Vector3f() { x = y = z = 0; }
	};

	struct Vector4f
	{
		float x, y, z, w;
		Vector4f(float ix, float iy, float iz, float iw) { x = ix; y = iy; z = iz; w = iw; }
		Vector4f() { x = y = z = w = 0; }
	};

	ShaderInputs()
	{
		iOffset					= Vector3f(0.0f, 0.0f, 0.0f);
		iResolution				= Vector3f(100.0f, 100.0f, 100.0f);
		iGlobalTime				= 0.0f;
		iChannelTime[0]			= 0.0f;
		iChannelTime[1]			= 0.0f;
		iChannelTime[2]			= 0.0f;
		iChannelTime[3]			= 0.0f;
		iChannelResolution[0]	= Vector3f(100.0f, 100.0f, 100.0f);
		iChannelResolution[1]	= Vector3f(100.0f, 100.0f, 100.0f);
		iChannelResolution[2]	= Vector3f(100.0f, 100.0f, 100.0f);
		iChannelResolution[3]	= Vector3f(100.0f, 100.0f, 100.0f);
		iMouse					= Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
		iDate					= Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
		
		iCameraActive		= 0;
		iCameraScreen		= OVR::Vector3f(2.0f, 1.5f, 1.0f);
		iCameraPosition.SetIdentity();
	}

	void apply_inputs(int program)
	{
		gl_uniform_1f(program, "iGlobalTime",	iGlobalTime);
		gl_uniform_1i(program, "iChannel0",		0);
		gl_uniform_1i(program, "iChannel1",		1);
		gl_uniform_1i(program, "iChannel2",		2);
		gl_uniform_1i(program, "iChannel3",		3);
		
		gl_uniform_3fv(program, "iOffset",				1,	&iOffset.x);
		gl_uniform_3fv(program, "iResolution",			1,	&iResolution.x);
		gl_uniform_1fv(program, "iChannelTime",			4,	iChannelTime);
		gl_uniform_3fv(program, "iChannelResolution",	4,	&iChannelResolution[0].x);
		gl_uniform_4fv(program, "iMouse",				1,	&iMouse.x);
		gl_uniform_4fv(program, "iDate",				1,	&iDate.x);

		gl_uniform_1i	 (program, "iCamera.active",				 iCameraActive);
		gl_uniform_3fv	 (program, "iCamera.screen",		1,		&iCameraScreen.x);
		gl_uniform_mat4f (program, "iCamera.position",		true,	&iCameraPosition.M[0][0]);
	}

	Vector3f		iOffset;					// viewport offset (in pixels)
	Vector3f		iResolution;				// viewport resolution (in pixels)
	float			iGlobalTime;				// shader playback time (in seconds)
	float			iChannelTime[4];			// channel playback time (in seconds)
	Vector3f		iChannelResolution[4];		// channel resolution (in pixels)
	Vector4f		iMouse;						// mouse pixel coords. xy: current (if MLB down), zw: click
	Vector4f		iDate;						// (year, month, day, time in seconds)
	int				iCameraActive;				// if a custom camera is present.
	OVR::Matrix4f	iCameraPosition;			// the matrix of the custom camera.
	OVR::Vector3f	iCameraScreen;				// the dimensions of the custom camera screen (halfsizeX, halfsizeY, distance from camera position).
};

// command line options
const char* command_line_shader = "menger_journey";
int command_line_display = 1;
int command_line_resx = 800;
int command_line_resy = 600;
const char* command_line_tex0 = NULL;
const char* command_line_tex1 = NULL;
const char* command_line_tex2 = NULL;
const char* command_line_tex3 = NULL;
const char* command_line_operator="=";

// render buffer.
GLboolean	frame_buffer_enabled = true;
GLint		frame_buffer_width;
GLint		frame_buffer_height;
GLuint		frame_buffer = 0;
GLuint		frame_buffer_texture = 0;
GLint		frame_time_total;
GLint		frame_count;

bool sdl_recompile = false;
bool sdl_quit = false;
bool sdl_snapshot = false;
SDL_Window *sdl_window=NULL;
SDL_GLContext sdl_opengl_context;
SDL_Joystick* sdl_joystick=NULL;
bool sdl_key[1024];

// fragment shader.
GLuint			fragment_shader_program = 0;
const char*		fragment_shader_name="";
ShaderInputs	fragment_shader_inputs;
const char*		fragment_shader_tex0=NULL;
const char*		fragment_shader_tex1=NULL;
const char*		fragment_shader_tex2=NULL;
const char*		fragment_shader_tex3=NULL;

void update_sdl_events()
{
	SDL_Event event;	
	while( SDL_PollEvent( &event ) )
	{
		if( event.type == SDL_KEYDOWN )
		{
			if(event.key.keysym.sym < sizeof(sdl_key)) 
				sdl_key[event.key.keysym.sym] = true;

			if(event.key.keysym.sym == SDLK_F7)
			{
				sdl_recompile = true;
			}
			if(event.key.keysym.sym == SDLK_ESCAPE)
			{
				sdl_quit = true;
			}
			else if(event.key.keysym.sym == SDLK_PRINTSCREEN)
			{
				sdl_snapshot = true;
			}
			else
			{
				//on_sdl_key_down(event.key.keysym.sym);
			}
		}
		else if( event.type == SDL_KEYUP )
		{
			if(event.key.keysym.sym < sizeof(sdl_key)) 
				sdl_key[event.key.keysym.sym] = false;

			//on_sdl_key_up(event.key.keysym.sym);
		}
		else if( event.type == SDL_QUIT )
		{
			sdl_quit = true;
		}
	}
}

void init_sdl(DEVMODE dmScreenSettings)
{
	// initialise out rendering context.
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return;    
	
	sdl_window = SDL_CreateWindow("ShaderToyCulus!", 
		dmScreenSettings.dmPosition.x, 
		dmScreenSettings.dmPosition.y, 
		dmScreenSettings.dmPelsWidth, 
		dmScreenSettings.dmPelsHeight, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED);

	sdl_opengl_context = SDL_GL_CreateContext(sdl_window);

	// set window bounds into oculus display.
	SDL_SetWindowPosition(sdl_window, dmScreenSettings.dmPosition.x, dmScreenSettings.dmPosition.y);
	SDL_SetWindowSize(sdl_window, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight);
	//SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN);

	sdl_joystick = SDL_JoystickOpen(0);
	memset(sdl_key, 0, sizeof(sdl_key));
}

void init_opengl()
{
	glewInit();

	attach_opengl_debug_callbacks();
	
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
	// common inputs for all the shaders.
	char fragment_inputs_filename[128];
	sprintf_s(fragment_inputs_filename, sizeof(fragment_inputs_filename), "./shaders/%s.shader", "shader_inputs");

	std::list<GLint> shaders;
	shaders.push_back(load_shader(fragment_file_path, GL_FRAGMENT_SHADER, fragment_inputs_filename));
	GLint program_id = build_program(shaders);
	delete_shaders(shaders);
	return program_id;
}

void update_shader_inputs(void)
{
	int x, y;
	unsigned int buttons = SDL_GetMouseState(&x, &y);
	fragment_shader_inputs.iMouse.x		= (float)x;
	fragment_shader_inputs.iMouse.y		= (float)y;
	fragment_shader_inputs.iMouse.z		= (buttons &SDL_BUTTON(1))? 1.0f : 0.0f;
	fragment_shader_inputs.iGlobalTime	= (float)frame_time_total / 1000.0f;
	fragment_shader_inputs.iOffset		= ShaderInputs::Vector3f(0.0f, 0.0f, 0.0f);
	fragment_shader_inputs.iResolution	= ShaderInputs::Vector3f((float)frame_buffer_width, (float)frame_buffer_height, 0.0f);
}

void update_shader_textures(void)
{
	textures.set_channel_texture(0, fragment_shader_tex0);
	textures.set_channel_texture(1, fragment_shader_tex1);
	textures.set_channel_texture(2, fragment_shader_tex2);
	textures.set_channel_texture(2, fragment_shader_tex3);
}

// dump texture to the screen, to double check.
void debug_texture(const char* name)
{
	// Render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
		
	// use the debug texture as the only texture.
	textures.set_channel_texture(0, frame_buffer_texture);
	textures.unset_channel_texture(1);
	textures.unset_channel_texture(2);
	textures.unset_channel_texture(3);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(-1,  1);
	glTexCoord2f(1.0f, 1.0f); glVertex2i( 1,  1);
	glTexCoord2f(1.0f, 0.0f); glVertex2i( 1, -1);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(-1, -1);
	glEnd();
}

OVR::Matrix4f neckPosition;

void scene_to_buffer(void)
{
	if(frame_buffer_enabled)
	{
		glViewport(0, 0, frame_buffer_width, frame_buffer_height);
		
		if(oculus.has_display())
		{
			neckPosition.M[3][2] += 0.05f;

			// render to buffer.
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			glUseProgram(fragment_shader_program);

			fragment_shader_inputs.iOffset		= ShaderInputs::Vector3f(0.0f, 0.0f, 0.0f);
			fragment_shader_inputs.iResolution	= ShaderInputs::Vector3f((float)frame_buffer_width / 2, (float)frame_buffer_height, 0.0f);
			fragment_shader_inputs.iCameraActive =	oculus.calculateEyeScreen(fragment_shader_inputs.iCameraScreen) && 
													oculus.calculateEyePosition(fragment_shader_inputs.iCameraPosition, body.get_matrix(), OVR::Util::Render::StereoEye_Left);
			fragment_shader_inputs.apply_inputs(fragment_shader_program);		
			glRecti(-1, -1, 0, 1);

			fragment_shader_inputs.iOffset		 = ShaderInputs::Vector3f((float)frame_buffer_width / 2, 0.0f, 0.0f);
			fragment_shader_inputs.iResolution	 = ShaderInputs::Vector3f((float)frame_buffer_width / 2, (float)frame_buffer_height, 0.0f);
			fragment_shader_inputs.iCameraActive =	oculus.calculateEyeScreen(fragment_shader_inputs.iCameraScreen) && 
													oculus.calculateEyePosition(fragment_shader_inputs.iCameraPosition, body.get_matrix(), OVR::Util::Render::StereoEye_Right);
			fragment_shader_inputs.apply_inputs(fragment_shader_program);
			glRecti(0, -1, 1, 1);
		}
		else
		{
			// render to buffer.
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			glUseProgram(fragment_shader_program);

			fragment_shader_inputs.iCameraActive = false;
			fragment_shader_inputs.iOffset		= ShaderInputs::Vector3f(0.0f, 0.0f, 0.0f);
			fragment_shader_inputs.iResolution	= ShaderInputs::Vector3f((float)frame_buffer_width, (float)frame_buffer_height, 0.0f);
			fragment_shader_inputs.apply_inputs(fragment_shader_program);
			glRecti(-1, -1, 1, 1);
		}
	}
}

void buffer_to_display(void)
{
	glViewport(0, 0, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight);
    
	if(!frame_buffer_enabled)
	{
		fragment_shader_inputs.iCameraActive = false;
		fragment_shader_inputs.iOffset		 = ShaderInputs::Vector3f(0.0f, 0.0f, 0.0f);
		fragment_shader_inputs.iResolution	 = ShaderInputs::Vector3f((float)frame_buffer_width, (float)frame_buffer_height, 0.0f);
		fragment_shader_inputs.apply_inputs(fragment_shader_program);
		glUseProgram(fragment_shader_program);
		glRecti(-1, -1, 1, 1);
	}
	else if(oculus.has_display())
	{
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		
		// use the frame buffer as the only texture.
		textures.set_channel_texture(0, frame_buffer_texture);
		textures.unset_channel_texture(1);
		textures.unset_channel_texture(2);
		textures.unset_channel_texture(3);
				
		oculus.render(OVR::Util::Render::StereoEye_Left);
		oculus.render(OVR::Util::Render::StereoEye_Right);
	}
	else
	{
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		
		// use the frame buffer as the only texture.
		textures.set_channel_texture(0, frame_buffer_texture);
		textures.unset_channel_texture(1);
		textures.unset_channel_texture(2);
		textures.unset_channel_texture(3);
		
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
	else
	{
		TRACE("using Display #%02d [%s]", display_id, dmScreenSettings.dmDeviceName);
	}
}

void update_joystick_inputs(OVR::Vector3f& lin_inputs, OVR::Vector3f& ang_inputs)
{
	static float fast_speed = 16.0f;
	static float move_speed = 4.0f;
	static float look_speed = 3.0f;
	
	#define sign(a) (((a) < 0.0f)? -1.0f : 1.0f)

	if(sdl_joystick == NULL)
		return;

	static float deadZone = 8186.0f;
	static float maxZone = 32768.0f;
	
	float axes[32];
	int numAxes = SDL_JoystickNumAxes(sdl_joystick);
	for(int i = 0; i < numAxes; ++i)
	{
		float val = SDL_JoystickGetAxis(sdl_joystick, i);
		axes[i] = (fabs(val) > deadZone)? sign(val) * (fabs(val) - deadZone) / (maxZone - deadZone) : 0.0f;
	}

	int buttons[32];
	int numButtons = SDL_JoystickNumButtons(sdl_joystick);
	for(int i = 0; i < numButtons; ++i)
		buttons[i] = SDL_JoystickGetButton(sdl_joystick, i);

	lin_inputs += OVR::Vector3f(axes[0], 0.0f, axes[1]) * (move_speed + fast_speed * (axes[5] - axes[4]));
	ang_inputs += OVR::Vector3f(axes[3], -axes[2], buttons[9] * 0.25f - buttons[8] * 0.25f) * look_speed;
}

void update_mouse_keyboard_inputs(OVR::Vector3f& lin_inputs, OVR::Vector3f& ang_inputs)
{	
	static float fast_speed = 16.0f;
	static float move_speed = 4.0f;
	static float look_speed = 40.0f;
	
	int x, y;
	SDL_SetRelativeMouseMode(SDL_TRUE);
	unsigned int butts = SDL_GetRelativeMouseState(&x, &y);

	float axes[32];
	int numAxes = 2;
	axes[0] = x / (float)dmScreenSettings.dmPelsWidth;
	axes[1] = y / (float)dmScreenSettings.dmPelsHeight;

	float buttons[32];
	int numButtons = 4;
	buttons[0] = (float)(sdl_key[SDLK_w])? 1.0f : 0.0f;
	buttons[1] = (float)(sdl_key[SDLK_s])? 1.0f : 0.0f;
	buttons[2] = (float)(sdl_key[SDLK_a])? 1.0f : 0.0f;
	buttons[3] = (float)(sdl_key[SDLK_d])? 1.0f : 0.0f;
	buttons[4] = (float)(butts &  1)? 1.0f : 0.0f;
	buttons[5] = (float)(butts &  4)? 1.0f : 0.0f;
	buttons[6] = (float)(butts &  8)? 1.0f : 0.0f;
	
	ang_inputs += OVR::Vector3f(axes[1], -axes[0], buttons[5] * 0.25f - buttons[4] * 0.25f) * look_speed;
	lin_inputs += OVR::Vector3f(buttons[3] - buttons[2], 0.0f, buttons[1] - buttons[0]) * (move_speed + fast_speed * buttons[6]);
}

void update_body(float dt)
{
	OVR::Vector3f lin_inputs;
	OVR::Vector3f ang_inputs;
	update_joystick_inputs(lin_inputs, ang_inputs);
	update_mouse_keyboard_inputs(lin_inputs, ang_inputs);

	body.apply_inputs(lin_inputs, ang_inputs, dt);
	body.integrate(dt);
}

// main loop. Do various bits till we want to quit.
void sdl_main_loop()
{
	unsigned int sdl_frame_begin = SDL_GetTicks();
	unsigned int sdl_frame_count = 0;
	unsigned int frame_time_last = SDL_GetTicks();
		
	while(!sdl_quit)
	{
		unsigned int frame_time_delta = SDL_GetTicks() - frame_time_last;
		frame_time_last = SDL_GetTicks();
		frame_time_total = frame_time_last - sdl_frame_begin;

		update_sdl_events();

		update_body(frame_time_delta / 1000.0f);

		update_shader_inputs();
	
		update_shader_textures();

		scene_to_buffer();

		buffer_to_display();
	
		if (sdl_snapshot) 
		{
			time_t now = time(0);
			char filename[256];
			sprintf_s(filename, sizeof(filename), "./screenshots/%s_%d.tga", fragment_shader_name, now);
			save_screenshot_tga(filename);
			sdl_snapshot = false;
		}

		if(sdl_recompile)
		{
			char fragment_shader_filename[128];
			sprintf_s(fragment_shader_filename, sizeof(fragment_shader_filename), "./shaders/%s.shader", fragment_shader_name);
			fragment_shader_program = load_fragment_shader_program(fragment_shader_filename);
			sdl_recompile = false;
		}
		
        sdl_frame_count++;

		SDL_GL_SwapWindow(sdl_window);

		SDL_Delay(1);

		if (sdl_quit) 
		{
			unsigned int sdl_frame_end = SDL_GetTicks();
			TRACE("- average : %.3f fps", sdl_frame_count / (float)((sdl_frame_end - sdl_frame_begin) / 1000.0f));
			TRACE("------------------------------------------------------");
		}
	}
}

void parse_command_line(int argc, char * argv[])
{
	for(int i = 1; i < argc; ++i)
	{
		char seps[] = "=";
		char *value = NULL;
		char *token = NULL;
		token = strtok_s(argv[i], seps, &value);
		if(!token || !value || token[0] == 0 || value[0] == 0) 
			continue;
		
		if(strstr(token, "shader") != 0)
		{
			command_line_shader = value;
		}
		else if(token && strstr(token, "display") != 0)
		{
			int integer = atoi(value);
			if(integer > 0) command_line_display = integer;
		}
		else if(token && strstr(token, "resx") != 0)
		{
			int integer = atoi(value);
			if(integer > 0) command_line_resx = integer;
		}
		else if(token && strstr(token, "resy") != 0)
		{
			int integer = atoi(value);
			if(integer > 0) command_line_resy = integer;
		}
		else if(token && strstr(token, "tex0") != 0)
		{
			command_line_tex0 = value;
		}
		else if(token && strstr(token, "tex1") != 0)
		{
			command_line_tex1 = value;
		}
		else if(token && strstr(token, "tex2") != 0)
		{
			command_line_tex2 = value;
		}
		else if(token && strstr(token, "tex3") != 0)
		{
			command_line_tex3 = value;
		}			
	}
}

int SDL_main(int argc, char * argv[])
{
    parse_command_line(argc, argv);

    int displayId			= command_line_display;
	frame_buffer_width		= command_line_resx;
    frame_buffer_height		= command_line_resy;
    fragment_shader_name	= command_line_shader;
	fragment_shader_tex0	= command_line_tex0;
	fragment_shader_tex1	= command_line_tex1;
	fragment_shader_tex2	= command_line_tex2;
	fragment_shader_tex3	= command_line_tex3;

	char fragment_shader_filename[128];
	sprintf_s(fragment_shader_filename, sizeof(fragment_shader_filename), "./shaders/%s.shader", fragment_shader_name);
	
	time_t now = time(0);
    tm time_now;
    localtime_s(&time_now, &now);
	char date_now[128];
	asctime_s(date_now, sizeof(date_now), &time_now);
	
	TRACE("------------------------------------------------------");
    TRACE("- SHADERTOY");
	TRACE("---------------------");
	setup_display(displayId);
	TRACE("---------------------");
    TRACE("- display resolution : %d x %d", dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight);
    TRACE("- frame buffer       : %d x %d", frame_buffer_width, frame_buffer_height);
    TRACE("- shader             : %s", fragment_shader_filename);
    TRACE("- date               : %s", date_now);

	oculus.start();
    
	init_sdl(dmScreenSettings);
		
	init_opengl();

	textures.load_textures("./textures");

	oculus.load_shaders();

	create_buffer();

	fragment_shader_program = load_fragment_shader_program(fragment_shader_filename);

	sdl_main_loop();

	textures.unload_textures();
	
	oculus.stop();
	
	glDeleteProgram(fragment_shader_program);

	return 0;
};
