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
    auto manager = GLXContextManagerBase::make(V2i(600, 600), VERBOSE_OFF);
    
    auto renderer = TexRenderer();
    
    auto waylandServer = WaylandServerBase::make(VERBOSE_OFF);
    
    renderer.setup(VERBOSE_OFF);
    
    renderer.draw();
    
    manager->swapBuffer();
    
    sleep(4);
    
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