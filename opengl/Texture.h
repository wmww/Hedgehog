#pragma once

#include "../main/utils.h"

#include <EGL/egl.h>

class Texture
{
public:
	Texture(VerboseToggle verboseToggle);
	void loadFromImage(string filepath);
	void loadFromData(void * data, V2i dim);
	void loadFromEGLImage(EGLImage image, V2i dim);
	void draw();
	void clear();
	GLuint getTextureId();
	
private:
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

//const string textureShaderVertFile = "shader0.vert";
//const string textureShaderFragFile = "shader0.frag";
