#include "../h/Texture.h"
#include "../h/ShaderProgram.h"

ShaderProgram shaderProgram;

struct Texture::Impl
{
	VerboseToggle verbose = VERBOSE_OFF;
	
	Impl()
	{
		setupIfFirstInstance(this);
	}
	
	static void firstInstanceSetup()
	{
		shaderProgram = ShaderProgram(textureShaderVertFile, textureShaderFragFile, VERBOSE_OFF);
	}
};

Texture::Texture(VerboseToggle verboseToggle)
{
	impl = shared_ptr<Impl>(new Impl);
	impl->verbose = verboseToggle;
}

void Texture::draw()
{
	
}

GLuint Texture::getId()
{
	
}
