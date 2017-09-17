#include "WaylandEGL.h"


#include <EGL/egl.h>
#include <EGL/eglext.h>

// change to toggle debug statements on and off
#define debug debug_off

namespace WaylandEGL
{

// pointers to functions that need to be retrieved dynamically
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = nullptr;
PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = nullptr;
PFNEGLUNBINDWAYLANDDISPLAYWL eglUnbindWaylandDisplayWL = nullptr;

EGLDisplay eglDisplay = nullptr;
EGLContext eglContext = nullptr;
bool isSetUp = false;

void setEglVars(EGLDisplay eglDisplay, EGLContext eglContext)
{
	ASSERT(!isSetUp);
	WaylandEGL::eglDisplay = eglDisplay;
	WaylandEGL::eglContext = eglContext;
}

void setup(wl_display * display)
{
	debug(FUNC + " called");
	ASSERT_ELSE(eglDisplay, return);
	ASSERT_ELSE(eglContext, return);
	ASSERT(!isSetUp);
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
	
	isSetUp = true;
}

void loadIntoTexture(wl_resource * buffer, Texture texture)
{
	debug(FUNC + " called");
	ASSERT_ELSE(isSetUp, return);
	debug("using EGL for GPU buffer sharing");
	EGLint width, height;
	eglQueryWaylandBufferWL(eglDisplay, buffer, EGL_WIDTH, &width);
	eglQueryWaylandBufferWL(eglDisplay, buffer, EGL_WIDTH, &height);
	// TODO: I think I need to query for the format
	auto bufferDim = V2i(width, height);
	EGLint attribs = EGL_NONE;
	EGLImageKHR eglImageKhr = eglCreateImageKHR(
		eglDisplay,
		eglContext,
		EGL_WAYLAND_BUFFER_WL,
		buffer,
		&attribs
	);
	texture.loadFromEGLImage((EGLImage)eglImageKhr, bufferDim);
}

}

