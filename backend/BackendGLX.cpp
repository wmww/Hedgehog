#include "Backend.h"
#include "BackendImplBase.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <cstring>
#include <linux/input.h> // for BTN_LEFT and maybe other stuff
//#include <xkbcommon/xkbcommon.h> // for getting the keymap

// change to toggle debug statements on and off
#define debug debug_off

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct BackendGLX: Backend::ImplBase
{
	Display * display = nullptr;
	GLXContext glxContext;
	Window window;
	V2i dim;
	
	BackendGLX(V2i dimIn)
	{
		dim = dimIn;
		
		debug("opening X display...");
		display = XOpenDisplay(0);
		
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
		GLXFBConfig * glxfb = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
		ASSERT(glxfb != nullptr);
		
		debug("getting XVisualInfo...");
		XVisualInfo * visual = glXGetVisualFromFBConfig(display, glxfb[0]);
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
		GLXContext ctx_old = glXCreateContext(display, visual, 0, GL_TRUE);
		glXCreateContextAttribsARB =  (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
		glXMakeCurrent(display, 0, 0);
		glXDestroyContext(display, ctx_old);
		ASSERT(glXCreateContextAttribsARB != nullptr);
	 
		static int context_attribs[] =
		{
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
			None
		};
		
		debug("creating context...");
		
		glxContext = glXCreateContextAttribsARB(display, glxfb[0], NULL, true, context_attribs);
		ASSERT(glxContext != nullptr);
	 
		debug("Making context current");
		glXMakeCurrent(display, window, glxContext);
	}
	
	void openWindow(XVisualInfo * visual)
	{
		XSetWindowAttributes windowAttribs;
		windowAttribs.colormap = XCreateColormap(display, RootWindow(display, visual->screen), visual->visual, AllocNone);
		windowAttribs.border_pixel = 0;
		//windowAttribs.event_mask = StructureNotifyMask;
		windowAttribs.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;
		
		int x = 0;
		int y = 0;
		
		debug("creating window...");
		
		window = XCreateWindow(
			display,
			RootWindow(display, visual->screen), // parent
			x, y, dim.x, dim.y, // geometry
			0, // I think this is Z-depth
			visual->depth,
			InputOutput,
			visual->visual,
			CWBorderPixel|CWColormap|CWEventMask,
			&windowAttribs);
		
		string winName = "Hedgehog";
		
		// We use the XTextProperty structure to store the title.
        XTextProperty windowName;
        windowName.value = (unsigned char *) winName.c_str();
        
        // XA_STRING is not defined, but its value appears to be 31
        // I should probably just find the right header to include, but hey, who doesn't love magic numbers?
        windowName.encoding = 31;
        // windowName.encoding = XA_STRING;
        
        windowName.format = 8;
        windowName.nitems = strlen((char *)windowName.value);
		
		XSetWMName(display, window, &windowName);
		
		ASSERT(window != 0);
		
		debug("mapping window...");
		
		XMapWindow(display, window);
	}
	
	~BackendGLX()
	{
		debug("cleaning up context...");
		XDestroyWindow(display, window);
		glXDestroyContext(display, glxContext);
	}
	
	void swapBuffer()
	{
		glXSwapBuffers(display, window);
	}
	
	static uint x11BtnToLinuxBtn(uint x11Btn)
	{
		switch (x11Btn)
		{
		case Button1:
			return BTN_LEFT;
		case Button2:
			return BTN_MIDDLE;
		case Button3:
			return BTN_RIGHT;
		default:
			warning("your mouse has a weird-ass button");
			return BTN_EXTRA;
		}
	}
	
	void checkEvents()
	{	
		XEvent event;
		while (XPending(display))
		{
			XNextEvent(display, &event);
			
			if (auto interface = inputInterface.lock())
			{
				if (event.type == MotionNotify)
				{
					auto movement = V2d((double)event.xbutton.x / dim.x, (double)event.xbutton.y / dim.y);
					interface->pointerMotion(movement);
				}
				else if (event.type == ButtonPress)
				{
					interface->pointerClick(x11BtnToLinuxBtn(event.xbutton.button), true);
				}
				else if (event.type == ButtonRelease)
				{
					interface->pointerClick(x11BtnToLinuxBtn(event.xbutton.button), false);
				}
				else if (event.type == KeyPress) {
					interface->keyPress(event.xkey.keycode - 8, true);
				}
				else if (event.type == KeyRelease) {
					interface->keyPress(event.xkey.keycode - 8, false);
				}
				/*
				else if (event.type == ConfigureNotify) {
					event.xconfigure.width and event.xconfigure.height are used;
				}
				else if (event.type == Expose) {
					// time to redraw?
				}
				else if (event.type == FocusIn) {
					// idk what these things do:s
					xkb_state_unref(state);
					state = xkb_x11_state_new_from_device(keymap, xcb_connection, keyboard_device_id);
					update_modifiers();
				}
				*/
			}
		}
	}
	
	void * getXDisplay()
	{
		return display;
	}
};

Backend Backend::makeGLX(V2i dim)
{
	shared_ptr<BackendGLX> impl = make_shared<BackendGLX>(dim);
	Backend backend = Backend(impl);
	return backend;
}

