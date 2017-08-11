#pragma once

#include "utils.h"

class ShaderProgram
{
public:
	
	ShaderProgram() {data=0;};
	ShaderProgram(string vertFile, string fragFile, bool debug);
	inline operator GLuint() {return data;}
	~ShaderProgram();
	
private:
	
	class Shader
	{
	public:
		Shader(string file, GLenum type, bool debug);
		inline operator GLuint() {return data;}
		~Shader();
		
	private:
		string getInfoLog();
		GLuint data;
	};
	
	string getInfoLog();
	
	GLuint data;
};

