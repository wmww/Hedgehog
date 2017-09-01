#pragma once

#include "../main/utils.h"

#include <EGL/egl.h>

// there is almost no cost to creating a texture, initialization is lazy and done on first load
class Texture
{
public:
	Texture();
	
	// load functions
	void loadFromImage(string filepath);
	void loadFromData(void * data, V2i dim);
	void loadFromEGLImage(EGLImage image, V2i dim);
	
	// use
	void draw();
	
	// getters
	GLuint getTextureId();
	bool isSetUp();
	
private:
	struct Impl;
	shared_ptr<Impl> impl; // never null after creation
};

//const string textureShaderVertFile = "shader0.vert";
//const string textureShaderFragFile = "shader0.frag";
