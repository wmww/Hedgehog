#pragma once

#include "../main/util.h"

class ShaderProgram
{
public:
	
	ShaderProgram() {}
	
	void setupFromFiles(string vertFile, string fragFile);
	void setupFromCode(string vertCode, string fragCode);
	
	void bind();
	void unbind();
	
	inline bool isNull() { return impl == nullptr; };
	inline bool isValid() { return !isNull(); };
	
	// add more uniform functions as needed
	void uniformMatrix4fv(string name, GLfloat * data);
	
private:
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
	ShaderProgram(shared_ptr<Impl> impl) { this->impl = impl; }
};

