#include "BackendX11Base.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <cstring>
#include <X11/Xlib-xcb.h>

// change to toggle debug statements on and off
#define debug debug_off

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct BackendGLX: BackendX11Base
{
	GLXContext glxContext;
	
	BackendGLX(V2i dimIn)
	{
		dim = dimIn;
		
		debug("opening X display...");
		xDisplay = XOpenDisplay(0);
		
		debug("setting up XKB (keymap shit)");
		setupXKB();
		
		//const char *extensions = glXQueryExtensionsString(display, DefaultScreen(display));
		//cout << extensions << endl;
		
		static int visual_attribs[] =
		{
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			GLX_DOUBLEBUFFER, true,
			GLX_RED_SIZE, 1,
			GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1,
			None
		};
		
		debug("getting framebuffer config...");
		int fbcount;
		GLXFBConfig * glxfb = glXChooseFBConfig(xDisplay, DefaultScreen(xDisplay), visual_attribs, &fbcount);
		ASSERT(glxfb != nullptr);
		
		debug("getting XVisualInfo...");
		XVisualInfo * visual = glXGetVisualFromFBConfig(xDisplay, glxfb[0]);
		ASSERT(visual != nullptr);
		
		openWindow(visual);
		
		setupGLXContext(visual, glxfb);
		
		XFree(visual);
		XFree(glxfb);
		
		//debug("getting keymap with xkbcommon");
		//xkb_context * xkbContext;
		//xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		//ASSERT_ELSE(xkbContext != nullptr, return);
		
	}
	
	void setupGLXContext(XVisualInfo * visual, GLXFBConfig * glxfb)
	{
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
		
		// Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
		GLXContext oldContext = glXCreateContext(xDisplay, visual, 0, GL_TRUE);
		glXCreateContextAttribsARB =  (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
		glXMakeCurrent(xDisplay, 0, 0);
		glXDestroyContext(xDisplay, oldContext);
		ASSERT(glXCreateContextAttribsARB != nullptr);
	 
		static int context_attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
			None
		};
		
		debug("creating context...");
		
		glxContext = glXCreateContextAttribsARB(xDisplay, glxfb[0], NULL, true, context_attribs);
		ASSERT(glxContext != nullptr);
	 
		debug("Making context current");
		glXMakeCurrent(xDisplay, window, glxContext);
	}
	
	void openWindow(XVisualInfo * visual)
	{
		XSetWindowAttributes windowAttribs;
		windowAttribs.colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, visual->screen), visual->visual, AllocNone);
		windowAttribs.border_pixel = 0;
		//windowAttribs.event_mask = StructureNotifyMask;
		windowAttribs.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;
		
		int x = 0;
		int y = 0;
		
		debug("creating window...");
		
		window = XCreateWindow(
			xDisplay,
			RootWindow(xDisplay, visual->screen), // parent
			x, y, dim.x, dim.y, // geometry
			0, // I think this is Z-depth
			visual->depth,
			InputOutput,
			visual->visual,
			CWBorderPixel|CWColormap|CWEventMask,
			&windowAttribs);
		
		setWindowName("Hedgehog");
		
		ASSERT(window != 0);
		
		debug("mapping window...");
		
		XMapWindow(xDisplay, window);
	}
	
	~BackendGLX()
	{
		debug("cleaning up context...");
		XDestroyWindow(xDisplay, window);
		glXDestroyContext(xDisplay, glxContext);
	}
	
	void swapBuffer()
	{
		glXSwapBuffers(xDisplay, window);
	}
};

unique_ptr<Backend> Backend::makeGLX(V2i dim)
{
	return make_unique<BackendGLX>(dim);
}

