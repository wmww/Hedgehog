#pragma once

#include "utils.h"

class SingleShader
{
public:
	static SingleShader fromFile(string file, GLenum type, VerboseToggle verbose);
	static SingleShader fromCode(string code, GLenum type, VerboseToggle verbose);
	
	GLuint getShaderId();
	
private:
	SingleShader();
	
	string getInfoLog();
	
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

class ShaderProgram
{
public:
	ShaderProgram();
	
	static ShaderProgram fromFiles(string vertFile, string fragFile, VerboseToggle verbose);
	static ShaderProgram fromCode(string vertCode, string fragCode, VerboseToggle verbose);
	static ShaderProgram fromShaders(SingleShader vertShader, SingleShader fragShader, VerboseToggle verbose);
	~ShaderProgram();
	
	void activete();
	void deactivate();
	
	GLuint getProgramId();
	
private:	
	string getInfoLog();
	
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

