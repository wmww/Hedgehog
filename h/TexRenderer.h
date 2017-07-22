#pragma once

#include "utils.h"
#include "ShaderProgram.h"

class TexRenderer
{
public:
	//TexRenderer();
	//~TexRenderer();
	
	void setup(int width, int height);
	void draw();
	
	float blurRds=0;
	
private:
	
	void setupTexture();
	
	ShaderProgram shaderProgram;
	GLuint squareVAO;
	GLuint texture;
};
