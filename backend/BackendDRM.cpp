#include "Backend.h"
#include "../wayland/WaylandEGL.h"
#include <wayland-server.h>
#include <X11/Xlib.h>
#include <linux/input.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <X11/Xlib-xcb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <poll.h>

// change to toggle debug statements on and off
#define debug debug_off

extern "C"
{

int setup_everything();
void swap_buffers();
void * getEglDisplay();
void * getEglContext();

}

struct BackendDRM: Backend
{
	//EGLDisplay eglDisplay;
	//EGLConfig config;
	//EGLContext windowContext;
	//EGLSurface windowSurface;
	
	BackendDRM()
	{
		int ret = setup_everything();
		ASSERT_ELSE(ret == 0, exit(1));
		WaylandEGL::setEglVars(getEglDisplay(), getEglContext());
	}
	
	~BackendDRM()
	{
		warning("~BackendDRM not implemented");
	}
	
	void swapBuffer()
	{
		swap_buffers();
	}
	
	void checkEvents()
	{
		//warning(FUNC + " not implemented");
	}
	
	string getKeymap()
	{
		warning(FUNC + " not implemented");
		return "";
	}
	
	void * getXDisplay()
	{
		warning(FUNC + " not implemented and shouldn't even exist");
		return nullptr;
	}
};

unique_ptr<Backend> Backend::makeDRM()
{
	return make_unique<BackendDRM>();
}

