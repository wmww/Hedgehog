#include "Backend.h"
#include "BackendImplBase.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <cstring>

// change to toggle debug statements on and off
#define debug debug_off

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

struct BackendGLX: Backend::ImplBase
{
	Display * display = nullptr;
	GLXContext ctx;
	Window win;
	
	BackendGLX(V2i dim)
	{
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
		
		XSetWindowAttributes windowAttribs;
		windowAttribs.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
		windowAttribs.border_pixel = 0;
		//windowAttribs.event_mask = StructureNotifyMask;
		windowAttribs.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;
		
		int x = 0;
		int y = 0;
		
		debug("creating window...");
		
		win = XCreateWindow(
			display,
			RootWindow(display, vi->screen), // parent
			x, y, dim.x, dim.y, // geometry
			0, // I think this is Z-depth
			vi->depth,
			InputOutput,
			vi->visual,
			CWBorderPixel|CWColormap|CWEventMask,
			&windowAttribs);
		
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
	
	~BackendGLX()
	{
		debug("cleaning up context...");
		XDestroyWindow(display, win);
		//ctx = glXGetCurrentContext();
		//glXMakeCurrent(display, 0, 0);
		glXDestroyContext(display, ctx);
	}
	
	void swapBuffer()
	{
		glXSwapBuffers(display, win);
	}
	
	void checkEvents()
	{	
		XEvent event;
		while (XPending(display))
		{
			XNextEvent(display, &event);
			
			if (event.type == MotionNotify)
			{
				if (auto interface = inputInterface.lock())
				{
					auto movement = V2d(event.xbutton.x, event.xbutton.y);
					interface->pointerMotion(movement);
				}
			}
		}
		
		/*
		XEvent event;
		while (XPending(display)) {
			XNextEvent (display, &event);
			if (event.type == ConfigureNotify) {
				callbacks.resize (event.xconfigure.width, event.xconfigure.height);
			}
			else if (event.type == Expose) {
				callbacks.draw ();
			}
			else if (event.type == MotionNotify) {
				callbacks.mouse_motion (event.xbutton.x, event.xbutton.y);
			}
			else if (event.type == ButtonPress) {
				if (event.xbutton.button == Button1)
					callbacks.mouse_button (BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
				else if (event.xbutton.button == Button2)
					callbacks.mouse_button (BTN_MIDDLE, WL_POINTER_BUTTON_STATE_PRESSED);
				else if (event.xbutton.button == Button3)
					callbacks.mouse_button (BTN_RIGHT, WL_POINTER_BUTTON_STATE_PRESSED);
			}
			else if (event.type == ButtonRelease) {
				if (event.xbutton.button == Button1)
					callbacks.mouse_button (BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
				else if (event.xbutton.button == Button2)
					callbacks.mouse_button (BTN_MIDDLE, WL_POINTER_BUTTON_STATE_RELEASED);
				else if (event.xbutton.button == Button3)
					callbacks.mouse_button (BTN_RIGHT, WL_POINTER_BUTTON_STATE_RELEASED);
			}
			else if (event.type == KeyPress) {
				callbacks.key (event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_PRESSED);
				xkb_state_update_key (state, event.xkey.keycode, XKB_KEY_DOWN);
				update_modifiers ();
			}
			else if (event.type == KeyRelease) {
				callbacks.key (event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_RELEASED);
				xkb_state_update_key (state, event.xkey.keycode, XKB_KEY_UP);
				update_modifiers ();
			}
			else if (event.type == FocusIn) {
				xkb_state_unref (state);
				state = xkb_x11_state_new_from_device (keymap, xcb_connection, keyboard_device_id);
				update_modifiers ();
			}
		}
		*/
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

