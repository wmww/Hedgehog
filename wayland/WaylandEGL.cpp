#include "WaylandEGL.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

// change to toggle debug statements on and off
#define debug debug_on

namespace WaylandEGL
{

// pointers to functions that need to be retrieved dynamically
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = nullptr;
PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = nullptr;
PFNEGLUNBINDWAYLANDDISPLAYWL eglUnbindWaylandDisplayWL = nullptr;

EGLDisplay eglDisplay;

void setEglDisplay(void * eglDisplay)
{
	WaylandEGL::eglDisplay = eglDisplay;
}

void setup(wl_display * display)
{
	debug(FUNC + " called");
	// function pointers that need to be retrieved at run time. This is Cs sad, pathetic attempt at duck typing.
	eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	ASSERT(eglCreateImageKHR);
	eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	ASSERT(eglDestroyImageKHR);
	eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL)eglGetProcAddress("eglQueryWaylandBufferWL");
	ASSERT(eglQueryWaylandBufferWL);
	eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL)eglGetProcAddress("eglBindWaylandDisplayWL");
	ASSERT(eglBindWaylandDisplayWL);
	eglUnbindWaylandDisplayWL = (PFNEGLUNBINDWAYLANDDISPLAYWL)eglGetProcAddress("eglUnbindWaylandDisplayWL");
	ASSERT(eglUnbindWaylandDisplayWL);
	
	eglBindWaylandDisplayWL(eglDisplay, display);
}

}

