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

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
}