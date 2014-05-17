#include "body.h"

Body::Body(const OVR::Vector3f& position, const OVR::Vector3f& angles)
: m_position(position)
, m_orientation(OVR::Quatf(OVR::Vector3f(0.0f, 1.0f, 0.0f), angles.y) * OVR::Quatf(OVR::Vector3f(1.0f, 0.0f, 0.0f), angles.x) * OVR::Quatf(OVR::Vector3f(0.0f, 0.0f, 1.0f), angles.z))
{}

OVR::Vector3f scale(const OVR::Vector3f& a, const OVR::Vector3f& b)
{
	return OVR::Vector3f(a.x*b.x, a.y*b.y, a.z*b.z);
}

OVR::Vector3f clamp(const OVR::Vector3f& v, const OVR::Vector3f& min, const OVR::Vector3f& max)
{
	return OVR::Vector3f(	(v.x < min.x)? min.x : (v.x > max.x)? max.x : v.x,
							(v.y < min.y)? min.y : (v.y > max.y)? max.y : v.y,
							(v.z < min.z)? min.z : (v.z > max.z)? max.z : v.z);
}

OVR::Matrix4f Body::get_matrix() const
{
	OVR::Matrix4f orient = OVR::Matrix4f(m_orientation);
	OVR::Matrix4f mtx = OVR::Matrix4f::Translation(m_position) * orient;
	return mtx;
}

void Body::apply_inputs(const OVR::Vector3f& lin_inputs, const OVR::Vector3f& ang_inputs, float dt)
{
	static OVR::Vector3f angAccel(5.0f, 5.0f, 5.f);
	static OVR::Vector3f angSpeed(1.0f, 1.0f, 1.0f);
	static float angFriction = 0.075f;
	m_angular_velocity -= m_angular_velocity * angFriction;
	m_angular_velocity += scale(ang_inputs, angAccel * dt);
	m_angular_velocity = clamp(m_angular_velocity, -angSpeed, angSpeed);

	static OVR::Vector3f linAccel(1.0f, 1.0f, 1.0f);
	static OVR::Vector3f linSpeed(5.0f, 5.0f, 5.0f);
	static float linFriction = 0.075f;
	m_linear_velocity -= m_linear_velocity * linFriction;
	m_linear_velocity += scale(lin_inputs, linAccel * dt);
	m_linear_velocity = clamp(m_linear_velocity, -linSpeed, linSpeed);
}

void Body::integrate(float dt)
{
	OVR::Quatf quat_vel = m_orientation * OVR::Quatf(m_angular_velocity.x, m_angular_velocity.y, m_angular_velocity.z, 0.0f) * 0.5f;
	m_orientation += quat_vel * dt;
	m_orientation.Normalize();
	
	OVR::Vector3f lin_vel = OVR::Matrix4f(m_orientation).Transform(m_linear_velocity); 
	m_position += lin_vel * dt;
}

