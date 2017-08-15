#pragma once

#include "utils.h"
#include "ShaderProgram.h"

class TexRenderer: MessageLogger
{
public:
	//TexRenderer();
	//~TexRenderer();
	
	void setup(VerboseToggle verbose);
	void draw();
	
	float blurRds=0;
	
private:
	
	const string shaderVertPath = "shaders/shader0.vert";
	const string shaderFragPath = "shaders/shader0.frag";
	const string imagePath = "assets/hedgehog.jpg";
	
	void setupTexture();
	
	ShaderProgram shaderProgram;
	GLuint squareVAO;
	GLuint texture;
};
