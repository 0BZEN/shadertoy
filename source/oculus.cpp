#include "oculus.h"

OculusDevice::OculusDevice() 
{}

OculusDevice::~OculusDevice()
{
}

void OculusDevice::stop()
{
	glDeleteProgram(m_shader_program);
	m_eye_patch[OVR::Util::Render::StereoEye_Center].cleanup();
	m_eye_patch[OVR::Util::Render::StereoEye_Right].cleanup();
	m_eye_patch[OVR::Util::Render::StereoEye_Left].cleanup();
	m_shader_program = 0;

	m_shader_program = load_shader_program();
	m_eye_patch[OVR::Util::Render::StereoEye_Center].setup(OVR::Util::Render::StereoEye_Center);
	m_eye_patch[OVR::Util::Render::StereoEye_Right].setup(OVR::Util::Render::StereoEye_Right);
	m_eye_patch[OVR::Util::Render::StereoEye_Left].setup(OVR::Util::Render::StereoEye_Left);
	
	OVR::MessageHandler::RemoveHandlerFromDevices();
	m_sensor.Clear();
	m_HMD.Clear();
	m_device_manager.Clear();
	
	OVR::System::Destroy();
	OVR_DEBUG_STATEMENT(_CrtDumpMemoryLeaks());
}

void OculusDevice::start()
{
	m_shader_program = load_shader_program();
	m_eye_patch[OVR::Util::Render::StereoEye_Center].setup(OVR::Util::Render::StereoEye_Center);
	m_eye_patch[OVR::Util::Render::StereoEye_Right].setup(OVR::Util::Render::StereoEye_Right);
	m_eye_patch[OVR::Util::Render::StereoEye_Left].setup(OVR::Util::Render::StereoEye_Left);
	
	OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
	
	m_device_manager = *OVR::DeviceManager::Create();
	
	// We'll handle it's messages in this case.
	m_device_manager->SetMessageHandler(this);

	// Release Sensor/HMD in case this is a retry.
	m_sensor.Clear();
	m_HMD.Clear();
	
	m_HMD = *m_device_manager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
	if (m_HMD)
	{
		m_sensor = *m_HMD->GetSensor();

		// This will initialize HMDInfo with information about configured IPD,
		// screen size and other variables needed for correct projection.
		// We pass HMD DisplayDeviceName into the renderer to select the
		// correct monitor in full-screen mode.
		m_HMD->GetDeviceInfo(&m_HMD_info);

		m_stereo_config.SetHMDInfo(m_HMD_info);
		m_stereo_config.SetFullViewport(OVR::Util::Render::Viewport(0,0, m_HMD_info.HResolution, m_HMD_info.VResolution));
	    m_stereo_config.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
		
		if (m_HMD_info.HScreenSize > 0.140f) // 7"
			m_stereo_config.SetDistortionFitPointVP(-1.0f, 0.0f);
		else
			m_stereo_config.SetDistortionFitPointVP(0.0f, 1.0f);
	}
	else
	{            
		// If we didn't detect an HMD, try to create the sensor directly.
		// This is useful for debugging sensor interaction; it is not needed in
		// a shipping app.
		m_sensor = m_device_manager->EnumerateDevices<OVR::SensorDevice>().CreateDevice();
	}

	const char* detectionMessage=0;
	if (!m_HMD && !m_sensor)
		detectionMessage = "Oculus Rift not detected.";
	else if (!m_HMD)
		detectionMessage = "Oculus Sensor detected; HMD Display not detected.";
	else if (!m_sensor)
		detectionMessage = "Oculus HMD Display detected; Sensor not detected.";
	else if (m_HMD_info.DisplayDeviceName[0] == '\0')
		detectionMessage = "Oculus Sensor detected; HMD display EDID not detected.";
	else
		detectionMessage = 0;

	if (detectionMessage)
	{
		::MessageBoxA(0, detectionMessage, "Oculus Rift ERROR", MB_OK);
	}
	
	if (m_sensor)
	{
		// We need to attach sensor to SensorFusion object for it to receive 
		// body frame messages and update orientation. SFusion.GetOrientation() 
		// is used in OnIdle() to orient the view.
		m_sensor_fusion.AttachToSensor(m_sensor);
		m_sensor_fusion.SetPredictionEnabled(true);
		m_sensor_fusion.SetDelegateMessageHandler(this);			
	}
}

int OculusDevice::load_shader_program()
{
	static const char* gl_vertex_shader_code = 
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec3 Position;\n"
		"layout(location = 1) in vec2 TexCoord;\n"
		"out vec2 oTexCoord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(Position, 1);\n"
		"   oTexCoord = TexCoord;\n"
		"};\n";

	static const char* gl_fragment_shader_code = 
		"#version 330\n"
		"\n"
		"uniform vec2 LensCenter;\n"
		"uniform vec2 ScreenCenter;\n"
		"uniform vec2 Scale;\n"
		"uniform vec2 ScaleIn;\n"
		"uniform vec4 HmdWarpParam;\n"
		"uniform sampler2D texture0;\n"
		"varying vec2 oTexCoord;\n"
		"out vec4 outcolor;\n"
		"\n"
		"vec2 HmdWarp(vec2 in01)\n"
		"{\n"
		"   vec2  theta = (in01 - LensCenter) * ScaleIn; // Scales to [-1, 1]\n"
		"   float rSq = theta.x * theta.x + theta.y * theta.y;\n"
		"   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + \n"
		"                           HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
		"   return LensCenter + Scale * theta1;\n"
		"}\n"
		"void main()\n"
		"{\n"
		"   vec2 tc = HmdWarp(oTexCoord);\n"
		"   if (!all(equal(clamp(tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tc)))\n"
		"       outcolor = vec4(0);\n"
		"   else\n"
		"	   outcolor = texture2D(texture0, tc);\n"
		"};\n";

	// Now create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;
	
    // Compile Vertex Shader
    trace("[OCULUS] ""compiling vertex shader...");
    glShaderSource(VertexShaderID, 1, &gl_vertex_shader_code , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
	{
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            FATAL("%s", &VertexShaderErrorMessage[0]);
    }

	// Compile Fragment Shader
    trace("[OCULUS] ""compiling fragment shader...");
    glShaderSource(FragmentShaderID, 1, &gl_fragment_shader_code , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
	{
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            FATAL("%s", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    trace("[OCULUS] ""linking program...");
    int program = glCreateProgram();
    glAttachShader(program, VertexShaderID);
    glAttachShader(program, FragmentShaderID);
    glLinkProgram(program);

    // Check the program
    glGetProgramiv(program, GL_LINK_STATUS, &Result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 1 )
	{
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        FATAL("%s", &ProgramErrorMessage[0]);
    }
	else
	{
		trace("[OCULUS] ""program compiled.");    
	}

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

	return program;
}

bool OculusDevice::get_sensor_position(float& x, float& y, float& z) const
{
	// TODO. Some form of positional tracking.
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	return false;
}

bool OculusDevice::get_sensor_orientation(float& yaw, float& pitch, float& roll) const
{
	yaw = 0.0f;
	pitch = 0.0f;
	roll = 0.0f;

	if(!m_sensor_fusion.IsAttachedToSensor())
		return false;

	OVR::Quatf hmdOrient = m_sensor_fusion.GetOrientation();
	hmdOrient.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&yaw, &pitch, &roll);
	return true;
}

const OVR::HMDInfo& OculusDevice::get_HMD_info() const
{
	return m_HMD_info;
}

OVR::Util::Render::StereoConfig OculusDevice::get_stereo_config() const 
{ 
	return m_stereo_config; 
}

void OculusDevice::OnMessage(const OVR::Message& msg)
{
	if(msg.pDevice == m_sensor)
	{
		if (msg.Type == OVR::Message_BodyFrame)
		{
		}
	}
	else if(msg.pDevice == m_device_manager)
	{
		if (msg.Type == OVR::Message_DeviceAdded)
		{
			trace("[OCULUS] " "DeviceManager reported device added.");
		}
		else if (msg.Type == OVR::Message_DeviceRemoved)
		{
			trace("[OCULUS] " "DeviceManager reported device removed.");
		}
		else if (msg.Type == OVR::Message_DeviceAdded)
		{
			trace("[OCULUS] " "Sensor reported device added.");
		}
		else if (msg.Type == OVR::Message_DeviceRemoved)
		{
			trace("[OCULUS] " "Sensor reported device removed.");
		}
	}
}

void OculusDevice::update_uniform_params(OVR::Util::Render::StereoEye eye)
{
	// apply distortion for each eye.
	const OVR::Util::Render::StereoEyeParams& params = get_stereo_config().GetEyeRenderParams(eye);
	if(params.pDistortion)
	{
		GLsizei width = get_HMD_info().HResolution;
		GLsizei height = get_HMD_info().VResolution;
		const OVR::Util::Render::DistortionConfig& distortion = *params.pDistortion;

		float w  = float(params.VP.w) / float(width),
			  h  = float(params.VP.h) / float(height),
			  x  = float(params.VP.x) / float(width),
			  y  = float(params.VP.y) / float(height);
		float as = float(params.VP.w) / float(params.VP.h);

		// We are using 1/4 of DistortionCenter offset value here, since it is
		// relative to [-1,1] range that gets mapped to [0, 0.5].
		float HmdWarpParam[4]	= { distortion.K[0], distortion.K[1], distortion.K[2], distortion.K[3] };
		float ChromAbParam[4]	= { distortion.ChromaticAberration[0], distortion.ChromaticAberration[1], distortion.ChromaticAberration[2], distortion.ChromaticAberration[3] };
		float scaleFactor		= 1.0f / distortion.Scale;
		OVR::Vector2f LensCenter	(x + (w + distortion.XCenterOffset * 0.5f)*0.5f,	y + h*0.5f);
		OVR::Vector2f ScreenCenter	(x + w*0.5f,										y + h*0.5f);
		OVR::Vector2f Scale			((w/2) * scaleFactor,								(h/2) * scaleFactor * as);
		OVR::Vector2f ScaleIn		((2/w),												(2/h) / as);

		// fragment shader.
		gl_uniform_1i(m_shader_program, "Texture0",			0);
		gl_uniform_2f(m_shader_program, "LensCenter",		LensCenter.x,	 LensCenter.y);
		gl_uniform_2f(m_shader_program, "ScreenCenter",		ScreenCenter.x,	 ScreenCenter.y);
		gl_uniform_2f(m_shader_program, "Scale",			Scale.x,		 Scale.y);
		gl_uniform_2f(m_shader_program, "ScaleIn",			ScaleIn.x,		 ScaleIn.y);
		gl_uniform_4f(m_shader_program, "HmdWarpParam",		HmdWarpParam[0], HmdWarpParam[1], HmdWarpParam[2], HmdWarpParam[3]);
		gl_uniform_4f(m_shader_program, "ChromAbParam",		ChromAbParam[0], ChromAbParam[1], ChromAbParam[2], ChromAbParam[3]);
	}
}

void OculusDevice::render(OVR::Util::Render::StereoEye eye)
{
	glUseProgram(m_shader_program);
	update_uniform_params(eye);
	m_eye_patch[eye].render();
}

// various patches for eyes.
void OculusDevice::EyePatch::setup(OVR::Util::Render::StereoEye eye)
{
	static float g_vertex_buffer_data[3][4][3]=
	{	
		// centre eye
		{	{ -1.0f, -1.0f, 0.0f, },
			{  1.0f, -1.0f, 0.0f, },
			{  1.0f,  1.0f, 0.0f, },
			{ -1.0f,  1.0f, 0.0f, }, },

		// left eye
		{	{ -1.0f, -1.0f, 0.0f, },
			{  0.0f, -1.0f, 0.0f, },
			{  0.0f,  1.0f, 0.0f, },
			{ -1.0f,  1.0f, 0.0f, }, },

		// right eye
		{	{  0.0f, -1.0f, 0.0f, },
			{  1.0f, -1.0f, 0.0f, },
			{  1.0f,  1.0f, 0.0f, },
			{  0.0f,  1.0f, 0.0f, }, },
	};

	static float g_uv_buffer_data[3][4][2] = 
	{
		// center eye
		{	{ 0.0f, 0.0f, },
			{ 1.0f, 0.0f, },
			{ 1.0f, 1.0f, },
			{ 0.0f, 1.0f, }, },

		// left eye
		{	{ 0.0f, 0.0f, },
			{ 0.5f, 0.0f, },
			{ 0.5f, 1.0f, },
			{ 0.0f, 1.0f, }, },

		// right eye
		{	{ 0.5f, 0.0f, },
			{ 1.0f, 0.0f, },
			{ 1.0f, 1.0f, },
			{ 0.5f, 1.0f, }, }
	};

	// load up the eye quad.
	glGenVertexArrays(1, &m_vertex_arrays);
	glBindVertexArray(m_vertex_arrays);

	glGenBuffers(1, &m_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data[eye]), &g_vertex_buffer_data[eye][0][0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data[eye]), &g_uv_buffer_data[eye][0][0], GL_STATIC_DRAW);
}

void OculusDevice::EyePatch::render()
{
	// render the quad for the eye patch on Oculus display.
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
	glBindBuffer(GL_ARRAY_BUFFER, m_uv_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);  

	glDrawArrays(GL_QUADS, 0, 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void OculusDevice::EyePatch::cleanup()
{
	glDeleteBuffers(1, &m_vertex_buffer);
	glDeleteBuffers(1, &m_uv_buffer);
	glDeleteVertexArrays(1, &m_vertex_arrays);
}
