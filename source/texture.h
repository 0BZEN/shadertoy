#pragma once

#include "utils.h"
#include "SDL_image.h"

class Textures
{
public:
	Textures();
	bool load_textures(const char* directory);
	void unload_textures();
	
	GLuint find_texture(const char* texture_name);
	void set_channel_texture(int channel, GLuint texture);
	void set_channel_texture(int channel, const char* texture_name);

	bool create_empty_texture();
	typedef std::map<std::string, GLuint> TextureMap;
	typedef std::pair<std::string, GLuint> TextureEntry;
	TextureMap m_textures;
};
