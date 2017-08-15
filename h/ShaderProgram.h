#pragma once

#include "utils.h"

class SingleShader
{
public:
	static SingleShader fromFile(string file, GLenum type, VerboseToggle verbose);
	static SingleShader fromCode(string code, GLenum type, VerboseToggle verbose);
	
	GLuint getShaderId();
	
private:
	ShaderProgram() {}
	
	struct Impl;
	shared_ptr<impl> impl = nullptr;
};

class ShaderProgram
{
public:
	
	static ShaderProgram fromFiles(string vertFile, string fragFile, VerboseToggle verbose);
	static ShaderProgram fromCode(string vertCode, string fragCode, VerboseToggle verbose);
	static ShaderProgram fromShaders(SingleShader vertShader, SingleShader fragShader, VerboseToggle verbose);
	
	GLuint getProgramId();
	
private:
	ShaderProgram() {}
	
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

