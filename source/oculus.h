#pragma once

#include "utils.h"
#include "OVR.h"

class OculusDevice : public OVR::MessageHandler
{
public:
	OculusDevice();
	~OculusDevice();
	
	void start();
	void stop();
	void render(OVR::Util::Render::StereoEye eye);
	
	bool get_sensor_position(float& x, float& y, float& z) const;
	bool get_sensor_orientation(float& yaw, float& pitch, float& roll) const;
	
	const OVR::HMDInfo& get_HMD_info() const;
	OVR::Util::Render::StereoConfig get_stereo_config() const;

	bool has_display() const { return m_HMD != 0; }
	bool has_sensor() const { return m_sensor != 0; }

private:
	void OnMessage(const OVR::Message& msg);

	int load_shader_program();
	void update_uniform_params(OVR::Util::Render::StereoEye eye);

	struct EyePatch
	{
		// various patches for eyes.
		void setup(OVR::Util::Render::StereoEye eye);
		void render();
		void cleanup();
		GLuint m_vertex_arrays;
		GLuint m_vertex_buffer;
		GLuint m_uv_buffer;
	};
	EyePatch m_eye_patch[3];
	int m_shader_program;
	
	OVR::Ptr<OVR::DeviceManager>			m_device_manager;
    OVR::Ptr<OVR::SensorDevice>				m_sensor;
    OVR::Ptr<OVR::HMDDevice>				m_HMD;
    OVR::HMDInfo							m_HMD_info;
	OVR::SensorFusion						m_sensor_fusion;
    OVR::Util::Render::StereoConfig			m_stereo_config;
};
