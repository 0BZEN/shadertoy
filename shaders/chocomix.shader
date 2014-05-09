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

varying vec3 s[4];

void main()
{
    float t,b,c,h=0;

    vec3 m, n, p = vec3(.2), d = normalize(.001*gl_FragCoord.rgb-p);

    for(int i=0;i<4;i++)
    {
        t=2;
        for(int i=0;i<4;i++)
        {
            b=dot(d,n=s[i]-p);

            c=b*b+.2-dot(n,n);

            if(b-c<t)if(c>0)
            {
                m=s[i];t=b-c;
            }
        }

       p+=t*d;

       d=reflect(d,n=normalize(p-m));

       h+=pow(n.x*n.x,44.)+n.x*n.x*.2;
    }

    gl_FragColor=vec4(h,h*h,h*h*h*h,h);
};
