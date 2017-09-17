#include "Texture.h"
#include "ShaderProgram.h"
#include <wayland-server-protocol.h>
#include <wayland-server.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext.h>
#include <SOIL/SOIL.h>

// change to toggle debug statements on and off
#define debug debug_off

typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;

struct Texture::Impl
{
	GLuint textureId = 0;
	V2i dim;
	
	Impl()
	{
		if (glEGLImageTargetTexture2DOES == nullptr)
		{
			glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
			ASSERT(glEGLImageTargetTexture2DOES != nullptr);
		}
		
		debug("setting up OpenGL texture");
		
		glGenTextures(1, &textureId);
		
		glBindTexture(GL_TEXTURE_2D, textureId);
		{
			// Set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glBindTexture(GL_TEXTURE_2D, false);
	}
	
	~Impl()
	{
		debug("deleting texture");
		glDeleteTextures(1, &textureId);
	}
};

void Texture::setupEmpty()
{
	if (!impl)
		impl = make_shared<Impl>();
}

void Texture::loadFromImage(string imagePath)
{
	debug("loading '" + imagePath + "' into texture...");
	if (!impl)
		impl = make_shared<Impl>();
	// Load and generate the texture
	int width, height;
	unsigned char* image = SOIL_load_image(imagePath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (!image)
	{
		warning(string() + "image loading error: " + SOIL_last_result());
	}
	debug("creating texture from image...");
	glBindTexture(GL_TEXTURE_2D, impl->textureId);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, false); // Unbind texture when done, so we won't accidentally mess up our texture.
	impl->dim = V2i(width, height);
	SOIL_free_image_data(image);
}

void Texture::loadFromData(void * data, V2i dim)
{
	if (!impl)
		impl = make_shared<Impl>();
	//impl->important("(" + to_string(dim.x) + ", " + to_string(dim.y) + ")");
	/*
	impl->important("loading from data, dim: " + to_string(dim));
	
	for (int i = 0; i < dim.x * dim.y; i += 600)
	{
		impl->important(
				"color: "
				+ to_string(((unsigned char *)data)[i * 4 + 0]) + ", "
				+ to_string(((unsigned char *)data)[i * 4 + 1]) + ", "
				+ to_string(((unsigned char *)data)[i * 4 + 2]) + ", "
				+ to_string(((unsigned char *)data)[i * 4 + 3])
			);
	}
	*/
	
	glBindTexture(GL_TEXTURE_2D, impl->textureId);
	{
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim.x, dim.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim.x, dim.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dim.x, dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, false);
	
	impl->dim = dim;
}

void Texture::loadFromEGLImage(void * image, V2i dim)
{
	if (!impl)
		impl = make_shared<Impl>();
	ASSERT_ELSE(glEGLImageTargetTexture2DOES, return);
	glBindTexture(GL_TEXTURE_2D, impl->textureId);
	{
		glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
	}
	glBindTexture(GL_TEXTURE_2D, false);
	
	impl->dim = dim;
}

void Texture::bind()
{
	ASSERT_ELSE(impl, return);
	glBindTexture(GL_TEXTURE_2D, impl->textureId);
}

void Texture::unbind()
{
	ASSERT(impl);
	glBindTexture(GL_TEXTURE_2D, false);
}

GLuint Texture::getTextureId()
{
	ASSERT_ELSE(impl, return 0);
	ASSERT(impl->textureId);
	return impl->textureId;
}
