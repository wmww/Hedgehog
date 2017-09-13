#include <unistd.h>
#include <iostream>
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../opengl/Texture.h"
#include "../backend/Backend.h"
#include "../wayland/WaylandServer.h"
#include "Surface2D.h"
#include "../scene/Scene.h"

// change to toggle debug statements on and off
#define debug debug_off

int main (int argc, char ** argv)
{
	//auto backend = Backend::makeGLX(V2i(800, 800));
	auto backend = Backend::makeDefault(V2i(800, 800));
	
	glewInit();
	
	auto texture = Texture();
	texture.loadFromImage("assets/hedgehog.jpg");
	
	WaylandServer::setup();
	
	Scene scene;
	scene.setup();
	
	backend.setInputInterface(scene.getInputInterface());
	
	while (true)
	{
		backend.checkEvents();
		texture.draw();
		WaylandServer::iteration();
		scene.draw();
		backend.swapBuffer();
		sleepForSeconds(0.01667);
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