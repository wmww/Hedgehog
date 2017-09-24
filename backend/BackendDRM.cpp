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

int drmSetup();
void drmSwapBuffers();
void * drmGetEglDisplay();
void * drmGetEglContext();

}

extern bool stop;
bool libinput_setup();
void libinput_destroy();
void libinput_check_events(InputInterface * interface);

struct BackendDRM: Backend
{
	//EGLDisplay eglDisplay;
	//EGLConfig config;
	//EGLContext windowContext;
	//EGLSurface windowSurface;
	
	BackendDRM()
	{
		int ret = drmSetup();
		ASSERT_ELSE(ret == 0, exit(1));
		WaylandEGL::setEglVars(drmGetEglDisplay(), drmGetEglContext());
		ASSERT_FATAL(libinput_setup());
	}
	
	~BackendDRM()
	{
		warning("~BackendDRM not implemented");
		libinput_destroy();
	}
	
	void swapBuffer()
	{
		drmSwapBuffers();
	}
	
	void checkEvents()
	{
		if (auto input = inputInterface.lock())
		{
			libinput_check_events(&*input);
			if (stop)
				Backend::instance = nullptr;
			//warning(FUNC + " not implemented");
		}
	}
	
	string getKeymap()
	{
		warning(FUNC + " not implemented");
		return "";
	}
};

unique_ptr<Backend> Backend::makeDRM()
{
	return make_unique<BackendDRM>();
}

