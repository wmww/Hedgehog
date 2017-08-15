#include <unistd.h>
#include <iostream>
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../h/TexRenderer.h"
#include "../h/GLXContextManager.h"
#include "../h/WaylandServer.h"

int main (int argc, char ** argv)
{
	auto manager = GLXContextManagerBase::make(V2i(800, 800), VERBOSE_OFF);
	
	auto renderer = TexRenderer();
	renderer.setup(VERBOSE_ON);
	
	auto waylandServer = WaylandServerBase::make(VERBOSE_OFF);
	
	while (true)
	{
		waylandServer->iteration();
		renderer.draw();
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