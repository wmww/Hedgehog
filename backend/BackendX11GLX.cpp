#include "BackendX11Base.h"

#include <GL/glx.h>
#include <GL/gl.h>

// change to toggle debug statements on and off
#define debug debug_off

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct BackendX11GLX: BackendX11Base
{
	GLXContext glxContext;
	
	BackendX11GLX(V2i dim): BackendX11Base(dim)
	{
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
		
		debug("getting framebuffer config");
		int fbcount;
		GLXFBConfig * glxfb = glXChooseFBConfig(xDisplay, DefaultScreen(xDisplay), visual_attribs, &fbcount);
		ASSERT(glxfb != nullptr);
		
		debug("getting XVisualInfo");
		XVisualInfo * visual = glXGetVisualFromFBConfig(xDisplay, glxfb[0]);
		ASSERT(visual != nullptr);
		
		openWindow(visual, "Hedgehog");
		
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
		
		XFree(visual);
		XFree(glxfb);
		
	}
	
	~BackendX11GLX()
	{
		debug("cleaning up GLX context");
		glXDestroyContext(xDisplay, glxContext);
	}
	
	void swapBuffer()
	{
		glXSwapBuffers(xDisplay, window);
	}
};

unique_ptr<Backend> makeX11GLXBackend(V2i dim)
{
	return make_unique<BackendX11GLX>(dim);
}

