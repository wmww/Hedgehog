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
	
private:
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
	ShaderProgram(shared_ptr<Impl> impl) { this->impl = impl; }
};

