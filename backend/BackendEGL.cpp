#include "BackendX11Base.h"
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

struct BackendEGL: BackendX11Base
{
	EGLDisplay eglDisplay;
	EGLContext windowContext;
	EGLSurface windowSurface;
	
	BackendEGL(V2i dim): BackendX11Base(dim)
	{
		eglDisplay = eglGetDisplay(xDisplay);
		eglInitialize(eglDisplay, nullptr, nullptr);
		
		// setup EGL
		EGLint eglAttribs[] = {
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_BIT,
			EGL_RED_SIZE,			1,
			EGL_GREEN_SIZE,			1,
			EGL_BLUE_SIZE,			1,
			EGL_NONE
			};
		EGLConfig config;
		EGLint configsCount;
		eglChooseConfig(eglDisplay, eglAttribs, &config, 1, &configsCount);
		EGLint visualId;
		eglGetConfigAttrib(eglDisplay, config, EGL_NATIVE_VISUAL_ID, &visualId);
		XVisualInfo visualTemplate;
		visualTemplate.visualid = visualId;
		int visualsCount;
		XVisualInfo * visual = XGetVisualInfo(xDisplay, VisualIDMask, &visualTemplate, &visualsCount);
		
		openWindow(visual, "Hedgehog");
		
		// EGL context and surface
		eglBindAPI(EGL_OPENGL_API);
		const EGLint moreAttribs[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
			EGL_CONTEXT_MINOR_VERSION_KHR, 3,
			EGL_NONE
			};
		windowContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, moreAttribs);
		ASSERT(windowContext != EGL_NO_CONTEXT);
		windowSurface = eglCreateWindowSurface(eglDisplay, config, window, nullptr);
		ASSERT(windowSurface != EGL_NO_SURFACE);
		eglMakeCurrent(eglDisplay, windowSurface, windowSurface, windowContext);
	}
	
	~BackendEGL()
	{
		warning("~BackendEGL not implemented");
	}
	
	void swapBuffer()
	{
		eglSwapBuffers(eglDisplay, windowSurface);
	}
};

unique_ptr<Backend> Backend::makeEGL(V2i dim)
{
	return make_unique<BackendEGL>(dim);
}

