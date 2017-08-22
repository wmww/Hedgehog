// disables debug statements, must be before includes
#define NO_DEBUG

#include "GLXContextManager.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <cstring>

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct GLXContextManagerImpl: GLXContextManagerBase
{
	GLXContextManagerImpl(V2i dim, bool verboseIn)
	{
		verbose = verboseIn;
		
		debug("opening X display...");
		
		display = XOpenDisplay(0);
		
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
		
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
		GLXFBConfig *fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
		
		assert(fbc != nullptr);
		
		debug("getting XVisualInfo...");
		
		XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[0]);
	 
		XSetWindowAttributes swa;
		
		debug("creating colormap...");
		
		swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
		swa.border_pixel = 0;
		swa.event_mask = StructureNotifyMask;
		
		int x = 0;
		int y = 0;
		
		debug("creating window...");
		
		win = XCreateWindow(display, RootWindow(display, vi->screen), x, y, dim.x, dim.y, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa);
		
		string winName = "Hedgehog";
		
		// We use the XTextProperty structure to store the title.
        XTextProperty windowName;
        windowName.value    = (unsigned char *) winName.c_str();
        
        // XA_STRING is not defined, but its value appears to be 31
        //windowName.encoding = XA_STRING;
        windowName.encoding = 31;
        
        windowName.format   = 8;
        windowName.nitems   = strlen((char *) windowName.value);
		
		XSetWMName(display, win, &windowName);
		
		assert(win != 0);
		
		debug("mapping window...");
		
		XMapWindow(display, win);
	 
		// Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
		GLXContext ctx_old = glXCreateContext(display, vi, 0, GL_TRUE);
		glXCreateContextAttribsARB =  (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
		glXMakeCurrent(display, 0, 0);
		glXDestroyContext(display, ctx_old);
	 
		assert(glXCreateContextAttribsARB != nullptr);
	 
		static int context_attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
			None
		};
		
		debug("creating context...");
		
		ctx = glXCreateContextAttribsARB(display, fbc[0], NULL, true, context_attribs);
		assert(ctx != nullptr);
	 
		debug("Making context current");
		glXMakeCurrent(display, win, ctx);
	}
	
	~GLXContextManagerImpl()
	{
		debug("cleaning up context...");
		
		//ctx = glXGetCurrentContext();
		//glXMakeCurrent(display, 0, 0);
		glXDestroyContext(display, ctx);
	}
	
	Display * getDisplay()
	{
		return display;
	}
	
	void swapBuffer()
	{
		glXSwapBuffers(display, win);
	}
	
	Display * display = nullptr;
	GLXContext ctx;
	Window win;
	
	bool verbose = false;
};

GLXContextManager GLXContextManagerBase::instance = nullptr;

GLXContextManager GLXContextManagerBase::make(V2i dim, bool verbose)
{
	auto instance = GLXContextManager(new GLXContextManagerImpl(dim, verbose));
	GLXContextManagerBase::instance = instance;
	return instance;
}

