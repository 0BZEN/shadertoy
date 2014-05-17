#pragma once

#include "utils.h"

struct Body
{
	Body(const OVR::Vector3f& position, const OVR::Vector3f& angles);

	void apply_inputs(const OVR::Vector3f& lin_inputs, const OVR::Vector3f& ang_inputs, float dt);

	void integrate(float dt);

	OVR::Matrix4f get_matrix() const;

private:
	void updateController(SDL_Joystick* joystick, float dt, OVR::Vector3f& linImpulse, OVR::Vector3f& angImpulse);

	OVR::Vector3f	m_position;					// position of the centre of the body
	OVR::Quatf		m_orientation;				// Orientation of the body in quaternion form
	OVR::Vector3f	m_linear_velocity;			// linear velocity
	OVR::Vector3f	m_angular_velocity;			// rotational velocity
};
