#pragma once

#include "utils.h"

class Texture
{
public:
	Texture(VerboseToggle verboseToggle);
	void draw();
	GLuint getId();
	
private:
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

const string textureShaderVertFile = "shader0.vert";
const string textureShaderFragFile = "shader0.frag";
