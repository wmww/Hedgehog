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
		
		if (verbose)
			cout << "opening X display..." << endl;
		
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
		
		if (verbose)
			cout << "getting framebuffer config..." << endl;
		
		int fbcount;
		GLXFBConfig *fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
		if (!fbc)
		{
			logError("Failed to retrieve a framebuffer config");
			exit(1);
		}
		
		if (verbose)
			cout << "getting XVisualInfo..." << endl;
		
		XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[0]);
	 
		XSetWindowAttributes swa;
		
		if (verbose)
			cout << "creating colormap..." << endl;
		
		swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
		swa.border_pixel = 0;
		swa.event_mask = StructureNotifyMask;
		
		int x = 0;
		int y = 0;
		
		if (verbose)
			cout << "creating window..." << endl;
		
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
		
		if (!win)
		{
			logError("failed to create window");
			exit(1);
		}
		
		if (verbose)
			cout << "mapping window..." << endl;
		
		XMapWindow(display, win);
	 
		// Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
		GLXContext ctx_old = glXCreateContext(display, vi, 0, GL_TRUE);
		glXCreateContextAttribsARB =  (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
		glXMakeCurrent(display, 0, 0);
		glXDestroyContext(display, ctx_old);
	 
		if (glXCreateContextAttribsARB == NULL)
		{
			logError("glXCreateContextAttribsARB entry point not found");
			exit(1);
		}
	 
		static int context_attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
			None
		};
		
		if (verbose)
			cout << "creating context..." << endl;
		
		ctx = glXCreateContextAttribsARB(display, fbc[0], NULL, true, context_attribs);
		if (!ctx)
		{
			logError("failed to create GL3 context");
			exit(1);
		}
	 
		std::cout << "Making context current" << std::endl;
		glXMakeCurrent(display, win, ctx);
	}
	
	~GLXContextManagerImpl()
	{
		if (verbose)
			cout << "cleaning up context..." << endl;
		
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
