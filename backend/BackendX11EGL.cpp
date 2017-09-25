#include "BackendX11Base.h"
#include "../wayland/WaylandEGL.h"

#include <EGL/eglext.h>

// change to toggle debug statements on and off
#define debug debug_off

struct BackendX11EGL: BackendX11Base
{
	EGLDisplay eglDisplay;
	EGLConfig config;
	EGLContext windowContext;
	EGLSurface windowSurface;
	bool valid = false;
	
	BackendX11EGL(V2i dim): BackendX11Base(dim)
	{
		// if returns before 'valid' is set to true, this object will be considered invalid
		ASSERT_ELSE(xDisplay, return);
		
		eglDisplay = eglGetDisplay(xDisplay);
		ASSERT_ELSE(eglDisplay != EGL_NO_DISPLAY, return);
		
		bool eglInitializeSuccess = eglInitialize(eglDisplay, nullptr, nullptr);
		ASSERT_ELSE(eglInitializeSuccess, return);
		
		// setup EGL
		EGLint eglAttribs[] = {
			EGL_RENDERABLE_TYPE,	EGL_OPENGL_BIT,
			EGL_RED_SIZE,			1,
			EGL_GREEN_SIZE,			1,
			EGL_BLUE_SIZE,			1,
			EGL_NONE
		};
		EGLint configsCount;
		bool eglConfigSuccess = eglChooseConfig(eglDisplay, eglAttribs, &config, 1, &configsCount);
		ASSERT_ELSE(eglConfigSuccess, return);
		EGLint visualId;
		eglGetConfigAttrib(eglDisplay, config, EGL_NATIVE_VISUAL_ID, &visualId);
		XVisualInfo visualTemplate;
		visualTemplate.visualid = visualId;
		int visualsCount;
		XVisualInfo * visual = XGetVisualInfo(xDisplay, VisualIDMask, &visualTemplate, &visualsCount);
		ASSERT_ELSE(visual, return);
		
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
		valid = true;
		
		WaylandEGL::setEglVars(eglDisplay, windowContext);
	}
	
	~BackendX11EGL()
	{
		warning("~BackendEGL not implemented");
	}
	
	void swapBuffer()
	{
		ASSERT_ELSE(valid, return);
		eglSwapBuffers(eglDisplay, windowSurface);
	}
};

unique_ptr<Backend> makeX11EGLBackend(V2i dim)
{
	return make_unique<BackendX11EGL>(dim);
}

