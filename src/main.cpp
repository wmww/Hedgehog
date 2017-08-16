#include <unistd.h>
#include <iostream>
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../h/TexRenderer.h"
#include "../h/Texture.h"
#include "../h/GLXContextManager.h"
#include "../h/WaylandServer.h"

int main (int argc, char ** argv)
{
	auto manager = GLXContextManagerBase::make(V2i(800, 800), VERBOSE_OFF);
	
	glewInit();
	
	//auto renderer = TexRenderer();
	//renderer.setup(VERBOSE_ON);
	//auto renderer = TexRenderer(VERBOSE_ON);
	
	auto texture = Texture(VERBOSE_ON);
	
	texture.loadFromImage("assets/hedgehog.jpg");
	
	auto waylandServer = WaylandServerBase::make(VERBOSE_OFF);
	
	
	while (true)
	{
		waylandServer->iteration();
		texture.draw();
		manager->swapBuffer();
		sleepForSeconds(0.2);
	}
	
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