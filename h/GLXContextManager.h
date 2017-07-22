#pragma once

#include <GL/glx.h>
#include <GL/gl.h>

class GLXContextManager
{
	public:
	GLXContextManager(int width, int height);
	~GLXContextManager();
	
	void swapBuffer();
	
	//private:
	Display * display = nullptr;
	GLXContext ctx;
	Window win;
};

