#include <unistd.h>
#include <iostream>
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../opengl/Texture.h"
#include "../backends/GLX/GLXContextManager.h"
#include "../wayland/WaylandServer.h"
#include "Surface2D.h"

// change to toggle debug statements on and off
#define debug debug_off

int main (int argc, char ** argv)
{
	auto glx = GLXContextManagerBase::make(V2i(800, 800));
	
	glewInit();
	
	//auto renderer = TexRenderer();
	//renderer.setup(VERBOSE_ON);
	//auto renderer = TexRenderer(VERBOSE_ON);
	
	auto texture = Texture();
	
	texture.loadFromImage("assets/hedgehog.jpg");
	
	WaylandServer::setup();
	
	while (true)
	{
		texture.draw();
		WaylandServer::iteration();
		Surface2D::drawAll();
		glx->swapBuffer();
		sleepForSeconds(0.05);
	}
	
	WaylandServer::shutdown();
	
	/*
	glClearColor (0, 0.5, 1, 1);
	glClear (GL_COLOR_BUFFER_BIT);
	glXSwapBuffers (display, win);

	sleep(1);

	glClearColor (1, 0.5, 0, 1);
	glClear (GL_COLOR_BUFFER_BIT);
	glXSwapBuffers (display, win);

	sleep(1);
	*/
}