shadertoy
=========

goofing around with shadertoy

shaders are almost copypasta from shadertoy.com. With some things cut out until I get round fixing the various inputs, texture samplers, ect...

all shaders should be in the 'shaders' folder, and with the '.shader' extension. SHADERS!!!

run shadertoy.exe <shadername> <display_id> <frame_buffer_width> <frame_buffer_height>

e.g. : "shadertoy.exe leizex 2 1024 768" will run the leizex shader, on display #2, at 1024x768.

the Shadertoys are very demanding on the GPU (most run a complex ray-marching algo on procedural content). 

Reducing the frame buffer resolution increases framerate but decreases quality.


command line parameters : e.g. shader=cartoon display=2 resx=1024 resy=1024 tex0=empty tex1=nyan


shader : what shader to select from the 'shaders' folder.

display : what display deviceto use.

resx, resy : resolution of the back buffer.

tex0, tex1, tex2, tex3 : what texture to select from the 'textures' folder, for each texture channel. See ShaderToy website to be sure.
