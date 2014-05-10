#pragma once

#include "texture.h"

Textures::Textures()
{
	unload_textures();
}
void Textures::unload_textures()
{
	// remove previous texture.
	for(TextureMap::iterator it = m_textures.begin();it != m_textures.end(); ++it)
	{
		glDeleteTextures(1, &it->second);
	}
	m_textures.clear();
}

bool Textures::create_empty_texture()
{
	char* name = "empty";

	// remove previous texture.
	TextureMap::iterator it = m_textures.find(name);
	if(it != m_textures.end())
	{
		glDeleteTextures(1, &it->second);
		m_textures.erase(it);
	}

	char data[16][16][3];
	memset(data, 0, sizeof(data));

	// generate the testure.
	GLuint texture;
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
	
	TextureEntry entry(name, texture);
	m_textures.insert(entry);
	return true;
}

bool Textures::load_textures(const char* directory)
{
	create_empty_texture();

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;
    
	char path[256];
	sprintf_s(path, sizeof(path), "%s/*.jpg", directory);

	if((hFind = FindFirstFile(path, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        FATAL("jpg files not found in '%s'.", directory);
        return false;
    }

	do
    {
        trace("[TEXTURES]" "loading image: %s.", fdFile.cFileName);

		char filename[256];
		sprintf_s(filename, sizeof(filename), "%s/%s", directory, fdFile.cFileName);

		SDL_Surface* surface = IMG_Load(filename);
		
		// yus. found the file.
		if(surface)
		{
			// remove previous texture.
			TextureMap::iterator it = m_textures.find(fdFile.cFileName);
			if(it != m_textures.end())
			{
				glDeleteTextures(1, &it->second);
				m_textures.erase(it);
			}

			trace("[TEXTURES]" "creating texture...");

			// convert to a OpenGL RBG surface. 
			//SDL_Surface* openGLSurface = SDL_CreateRGBSurface(0, surface->w, surface->h, 24, 0xff000000, 0x00ff0000, 0x0000ff00,0);
			//SDL_BlitSurface(surface, 0, openGLSurface, 0); /* blit onto a purely RGB Surface*/

			// generate the testure.
			GLuint texture;
			glGenTextures( 1, &texture );
			glBindTexture( GL_TEXTURE_2D, texture );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels );
			//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, openGLSurface->w, openGLSurface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, openGLSurface->pixels );
			
			TextureEntry entry(fdFile.cFileName, texture);
			m_textures.insert(entry);

			SDL_FreeSurface(surface);
			//SDL_FreeSurface(openGLSurface);
		}
    }
    while(FindNextFile(hFind, &fdFile)); //Find the next file.

    trace("[TEXTURES]" "loaded (%d) textures.", m_textures.size());
	
	return true;
}

void Textures::set_channel_texture(int channel, const char* texture_name)
{
	set_channel_texture(channel, find_texture(texture_name));
}

void Textures::set_channel_texture(int channel, GLuint texture)
{
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0+channel);
	glBindTexture(GL_TEXTURE_2D, texture);
}

GLuint Textures::find_texture(const char* texture_name)
{
	// remove previous texture.
	TextureMap::iterator it = m_textures.find(texture_name);
	if(it == m_textures.end())
		return 0;

	return it->second;
}