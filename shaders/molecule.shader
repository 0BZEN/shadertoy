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

// #define BLUE
// #define RED

#define FOV         	1.0
#define ITER_MAX    	20
#define ITER_DETAIL 	0.2
#define SIZE_ATOM 		0.30
#define SIZE_BOND 		0.01
#define SMIN_RADI 		0.20

// a few constants regarding the hexagonal grid
#define PI    	3.141592653589793
#define MULT1 	(1.0/tan(PI/3.0))
#define MULT2 	(1.0/sin(PI/3.0))
#define PHI   	((1.0+sqrt(5.0))/2.0)

float sdSphere(in vec3 p, in vec3 a, in float r) {
	return length(p-a)-r;
}

float sdCapsule(in vec3 p, in vec3 a, in vec3 b, in float r) {
	vec3 pa = p - a, ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return length( pa - ba*h ) - r;
}

float smin(float a, float b, float k) {
	float h = clamp(0.5+0.5*(b-a)/k, 0.0, 1.0);
	return mix(b, a, h) - k*h*(1.0-h);
}

float DE(in vec3 p) {
	float md = 10000.0;

	// ATOMS
	md = smin(md, sdSphere(p, vec3(-2.355, +1.155, -1.173), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-1.896, -0.052, -0.525), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-2.696, -1.058, -0.272), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-4.098, -1.040, -0.618), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-4.564, +0.058, -1.222), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-3.649, +1.212, -1.513), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-1.952, -2.085, +0.360), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-0.582, -1.616, +0.475), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-0.563, -0.389, -0.061), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-1.733, +1.915, -1.360), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-4.695, -1.817, -0.416), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-5.528, +0.112, -1.480), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-4.008, +2.029, -1.963), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+0.190, -2.109, +0.875), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+0.650, +0.486, -0.172), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+0.778, +0.764, -1.124), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+0.532, +1.298, +0.398), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+1.880, -0.312, +0.306), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+3.102, +0.498, +0.200), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+1.746, -0.579, +1.260), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+1.980, -1.131, -0.258), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+3.007, +1.672, +1.083), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+3.842, +2.217, +1.005), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+4.256, -0.318, +0.601), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+5.092, +0.226, +0.531), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+2.894, +1.370, +2.030), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+2.220, +2.228, +0.815), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+4.327, -1.113, -0.000), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(+4.137, -0.624, +1.545), SIZE_ATOM), SMIN_RADI);
	md = smin(md, sdSphere(p, vec3(-2.314, -2.997, +0.675), SIZE_ATOM), SMIN_RADI);

	// BONDS
	md = smin(md, sdCapsule(p, 
		vec3(-2.355, +1.155, -1.173), 
		vec3(-1.896, -0.052, -0.525), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-1.896, -0.052, -0.525), 
		vec3(-2.696, -1.058, -0.272), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-2.696, -1.058, -0.272), 
		vec3(-4.098, -1.040, -0.618), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-4.098, -1.040, -0.618), 
		vec3(-4.564, +0.058, -1.222), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-4.564, +0.058, -1.222), 
		vec3(-3.649, +1.212, -1.513), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-3.649, +1.212, -1.513), 
		vec3(-2.355, +1.155, -1.173), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-1.952, -2.085, +0.360), 
		vec3(-2.696, -1.058, -0.272), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-0.582, -1.616, +0.475), 
		vec3(-1.952, -2.085, +0.360), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-0.563, -0.389, -0.061), 
		vec3(-0.582, -1.616, +0.475), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-1.896, -0.052, -0.525), 
		vec3(-0.563, -0.389, -0.061), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-2.355, +1.155, -1.173), 
		vec3(-1.733, +1.915, -1.360), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-4.098, -1.040, -0.618), 
		vec3(-4.695, -1.817, -0.416), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-4.564, +0.058, -1.222), 
		vec3(-5.528, +0.112, -1.480), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-3.649, +1.212, -1.513), 
		vec3(-4.008, +2.029, -1.963), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-0.582, -1.616, +0.475), 
		vec3(+0.190, -2.109, +0.875), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-0.563, -0.389, -0.061), 
		vec3(+0.650, +0.486, -0.172), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+0.650, +0.486, -0.172), 
		vec3(+0.778, +0.764, -1.124), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+0.650, +0.486, -0.172), 
		vec3(+0.532, +1.298, +0.398), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+0.650, +0.486, -0.172), 
		vec3(+1.880, -0.312, +0.306), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+1.880, -0.312, +0.306), 
		vec3(+3.102, +0.498, +0.200), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+1.880, -0.312, +0.306), 
		vec3(+1.746, -0.579, +1.260), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+1.880, -0.312, +0.306), 
		vec3(+1.980, -1.131, -0.258), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+3.102, +0.498, +0.200), 
		vec3(+3.007, +1.672, +1.083), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+3.007, +1.672, +1.083), 
		vec3(+3.842, +2.217, +1.005), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+3.102, +0.498, +0.200), 
		vec3(+4.256, -0.318, +0.601), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+4.256, -0.318, +0.601), 
		vec3(+5.092, +0.226, +0.531), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+3.007, +1.672, +1.083), 
		vec3(+2.894, +1.370, +2.030), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+3.007, +1.672, +1.083), 
		vec3(+2.220, +2.228, +0.815), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+4.256, -0.318, +0.601), 
		vec3(+4.327, -1.113, -0.000), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(+4.256, -0.318, +0.601), 
		vec3(+4.137, -0.624, +1.545), SIZE_BOND), SMIN_RADI);
	md = smin(md, sdCapsule(p, 
		vec3(-1.952, -2.085, +0.360), 
		vec3(-2.314, -2.997, +0.675), SIZE_BOND), SMIN_RADI);

	return md;
}

mat2 rot(in float a) {
	return mat2(cos(a),sin(a),-sin(a),cos(a));	
}

float hexDist(vec2 p) {
	float dx = abs(p.x);
	float dy = abs(p.y);
	return max(dx+dy*MULT1, max(dx, dy*MULT2));
}

vec2 getHexCenter(vec2 p) {
	vec3 o = vec3(p.x, p.y/MULT2, 0);
	o.z = -0.5*o.x-o.y;
	o.y = -0.5*o.x+o.y;
	vec3 i = floor(o+0.5);
	float s = i.x+i.y+i.z;
	vec3 d = abs(i-o);
	if(d.x > d.y && d.x > d.z)
		i.x -= s;
	else if(d.y > d.x && d.y > d.z)
		i.y -= s;
	else
		i.z -= s;
	return vec2(i.x, (i.y-i.z)*0.5*MULT2);
}

vec3 hexgrid(vec2 p) {
	
	vec2 c = getHexCenter(p);
	float hdist = hexDist(c-p)*1.5;
	
	// add a pulse depending on axis
	float cdist = length(c-p)*1.5;
	float pulse1 = smoothstep(-1.0, +1.0,
		sin(cdist+iGlobalTime*0.412-p.y*PI*0.11-p.x*PI*0.2));
	float pulse2 = smoothstep(-1.0, +1.0,
		sin(cdist+iGlobalTime*0.842-p.y*PI*0.42-p.x*PI*0.1));
	vec3 result = vec3(0.3, 0.8, 0.2);
	result += vec3(0.062, 0.41, 0.03)*pulse1;
	result += vec3(0.008, 0.23, 0.04)*pulse2;
	
	// add another pulse
	float pulse3 = (sin(iGlobalTime*3.0-p.y*0.2-p.x*PI*0.1)*.5+.5);
	result += 1.0-smoothstep(0.0, pulse3, hdist);
	
	// and an antialiased black border
	result -= smoothstep(0.6, 1.0, hdist);
	
	return clamp(result, 0.0, 1.0);
}

// see http://www.math.utah.edu/~bresslof/publications/01-1.pdf
void distort(inout vec2 p) {
	p+=p*length(p)*0.9;
	p = vec2(atan(p.x, p.y)/PI*3.0, log(length(p))+iGlobalTime*0.15);
	p.x += p.y/PHI + iGlobalTime*0.1;
}

vec3 getBackground(vec2 uv) {
	float luv = length(uv);
	distort(uv);
	uv *= 10.0;
	vec3 result = hexgrid(uv);
	return result - smoothstep(0.0, 1.0, 1.0-luv);
}

void main(void) {
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;
	uv.y *= iResolution.y / iResolution.x;
	
	vec3 from = vec3(-10, 0, 0);
	vec3 dir = normalize(vec3(uv * FOV, 1.0));
	dir.xz *= rot(3.1415*.5);
	
	vec2 mouse=(iMouse.xy / iResolution.xy - 0.5) * 3.0;
	if (iMouse.z < 1.0) mouse = vec2(0.0);
	
	mat2 rotxz = rot(iGlobalTime*0.652+mouse.x);
	mat2 rotxy = rot(sin(iGlobalTime*0.814)*.3+mouse.y);
	
	from.xz *= rotxz;
	from.yz *= rotxy;
	dir.xz  *= rotxz;
	dir.yz  *= rotxy;

	float ao = 1.0;
	float haze = 1000.0;
	
	float totdist = 0.0;
	bool set = false;
	
	for (int steps = 0 ; steps < ITER_MAX ; steps++) {
		if (set) continue;
		vec3 p = from + totdist * dir;
		float dist = DE(p);
		haze = min(haze, dist);
		totdist += dist;
		if (dist < ITER_DETAIL) {
			set = true;
			ao = float(steps) / float(ITER_MAX);
		}
	}
	
	vec3 color = getBackground(uv);
	
	if (set) {
		color = vec3(0.454, 1.0, 0.42)*(1.0-ao);
		color -= totdist*0.02;
		color += (haze*haze)*0.8;
	} else {
		if (haze < 0.4) color = vec3(0.0, 0.1, 0.0);
		else color -= (1.0 - clamp(haze, 0.0, 1.0))*.5;
	}
	
	gl_FragColor = vec4(color, 1.0);
	
	#ifdef BLUE
	gl_FragColor.rgb = gl_FragColor.rbg;
	#endif
	
	#ifdef RED
	gl_FragColor.rgb = gl_FragColor.grb;
	#endif
}