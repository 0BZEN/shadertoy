uniform vec3      iOffset;				 // viewport offset (in pixels)
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform sampler2D iChannel0;	         // input channel. XX = 2D/Cube
uniform sampler2D iChannel1;	         // input channel. XX = 2D/Cube
uniform sampler2D iChannel2;	         // input channel. XX = 2D/Cube
uniform sampler2D iChannel3;	         // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)

struct Camera
{
	int  active;	// external camera active.
	mat4 position;	// external camera position.
	vec3 screen;	// external camera screen specifications (halfSizeX, halfSizeY, distanceZ).
};
uniform Camera iCamera; // Oculus HMD eye position.