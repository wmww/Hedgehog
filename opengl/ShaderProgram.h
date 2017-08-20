#pragma once

#include "../main/utils.h"

class SingleShader
{
public:
	static SingleShader fromFile(string file, GLenum type, VerboseToggle verbose);
	static SingleShader fromCode(string code, GLenum type, VerboseToggle verbose);
	
	GLuint getShaderId();
	
private:
	SingleShader();
	
	string getInfoLog();
	static string typeToString(GLenum type);
	
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

class ShaderProgram
{
public:
	struct Impl;
	
	ShaderProgram(shared_ptr<Impl> implIn);
	
	static ShaderProgram fromFiles(string vertFile, string fragFile, VerboseToggle verbose);
	static ShaderProgram fromCode(string vertCode, string fragCode, VerboseToggle verbose);
	static ShaderProgram fromShaders(SingleShader vertShader, SingleShader fragShader, VerboseToggle verbose);
	
	void activete();
	void deactivate();
	
	GLuint getProgramId();
	
private:
	ShaderProgram();
	string getInfoLog();
	
	// this can be null and assertions should be done whenever assuming its not
	shared_ptr<Impl> impl = nullptr;
};

