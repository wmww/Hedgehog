#include <unistd.h>
#include <iostream>
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../opengl/Texture.h"
#include "../opengl/RectRenderer.h"
#include "../backend/Backend.h"
#include "../wayland/WaylandServer.h"
#include "../scene/Scene.h"

// change to toggle debug statements on and off
#define debug debug_off

unique_ptr<Backend> Backend::instance;

int main (int argc, char ** argv)
{
	debug("setting up backend");
	//auto backend = Backend::makeGLX(V2i(800, 800));
	Backend::setup(Backend::EGL);
	ASSERT_ELSE(Backend::instance, exit(1));
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glewInit();
	
	auto texture = Texture();
	texture.loadFromImage("assets/hedgehog.jpg");
	RectRenderer renderer;
	
	debug("setting up wayland server");
	
	WaylandServer::setup();
	
	Scene scene;
	scene.setup();
	Backend::instance->setInputInterface(scene.getInputInterface());
	
	//double startTime = timeSinceStart();
	//int ticks = 0;
	
	debug("starting main loop");
	while (Backend::instance)
	{
		renderer.draw(texture, V2d(0, 0), V2d(1, 1));
		WaylandServer::iteration();
		scene.draw();
		Backend::instance->swapBuffer();
		//sleepForSeconds(0.01667);
		Backend::instance->checkEvents();
		//ticks++;
		//double endTime = timeSinceStart();
		//debug("FPS: " + to_string(ticks / (endTime - startTime)));
	}
	
	debug("shutting down wayland server");
	WaylandServer::shutdown();
	
	std::cout << "exiting" << std::endl;
}
