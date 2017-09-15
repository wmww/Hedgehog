#include "Backend.h"
//#include "BackendImplBase.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/Xlib-xcb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <cstring>
#include <linux/input.h> // for BTN_LEFT and maybe other stuff
//#include <xkbcommon/xkbcommon.h> // for getting the keymap

// change to toggle debug statements on and off
#define debug debug_off

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct BackendGLX: Backend
{
	Display * xDisplay = nullptr;
	GLXContext glxContext;
	Window window;
	V2i dim;
	xkb_keymap * keymap = nullptr;
	
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
		
		XSetWMName(xDisplay, window, &windowName);
		
		ASSERT(window != 0);
		
		debug("mapping window...");
		
		XMapWindow(xDisplay, window);
	}
	
	void setupXKB()
	{
		xcb_connection_t * xcbConnection = XGetXCBConnection(xDisplay);
		xkb_context * xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		xkb_x11_setup_xkb_extension(
			xcbConnection,
			XKB_X11_MIN_MAJOR_XKB_VERSION,
			XKB_X11_MIN_MINOR_XKB_VERSION,
			XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
			nullptr, nullptr, nullptr, nullptr
			);
		int32_t keyboardDeviceId = xkb_x11_get_core_keyboard_device_id(xcbConnection);
		keymap = xkb_x11_keymap_new_from_device(xkbContext, xcbConnection, keyboardDeviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);
		//xkb_state * xkbState = xkb_x11_state_new_from_device(keymap, xcbConnection, keyboardDeviceId);
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
		while (XPending(xDisplay))
		{
			XNextEvent(xDisplay, &event);
			
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
	
	string getKeymap()
	{
		ASSERT_ELSE(keymap, return "");
		return xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
	}
	
	void * getXDisplay()
	{
		return xDisplay;
	}
};

unique_ptr<Backend> Backend::makeGLX(V2i dim)
{
	return make_unique<BackendGLX>(dim);
}

