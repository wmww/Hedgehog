#include "Backend.h"
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

struct BackendEGL: Backend
{
	Display * xDisplay = nullptr;
	xcb_connection_t * xcbConnection = nullptr;
	EGLDisplay eglDisplay;
	EGLContext windowContext;
	EGLSurface windowSurface;
	Window win;
	V2i dim;
	
	BackendEGL(V2i dimIn)
	{
		dim = dimIn;
		
		debug("opening X display");
		
		xDisplay = XOpenDisplay(nullptr);
		
		debug("getting XCB connection");
		
		xcbConnection = XGetXCBConnection(xDisplay);
		
		setupXKB();
		
		eglDisplay = eglGetDisplay(xDisplay);
		eglInitialize(eglDisplay, nullptr, nullptr);
		
		createWindow();
	}
	
	~BackendEGL()
	{
		debug("cleaning up context...");
		XDestroyWindow(xDisplay, win);
	}
	
	void setupXKB()
	{
		/*
		xkb_context * xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		xkb_x11_setup_xkb_extension (xcb_connection, XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION, 0, NULL, NULL, NULL, NULL);
		keyboard_device_id = xkb_x11_get_core_keyboard_device_id (xcb_connection);
		keymap = xkb_x11_keymap_new_from_device (context, xcb_connection, keyboard_device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
		state = xkb_x11_state_new_from_device (keymap, xcb_connection, keyboard_device_id);
		*/
	}
	
	void createWindow()
	{
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
		
		// create the window
		XSetWindowAttributes windowAttributes;
		windowAttributes.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;
		auto rootWindow = RootWindow(xDisplay,DefaultScreen(xDisplay));
		windowAttributes.colormap = XCreateColormap(xDisplay, rootWindow, visual->visual, AllocNone);
		int x = 0;
		int y = 0;
		win = XCreateWindow(
			xDisplay,
			rootWindow, // parent
			x, y, dim.x, dim.y, // geometry
			0, // either boarder width or Z-depth
			visual->depth,
			InputOutput,
			visual->visual, // visual
			CWEventMask|CWColormap, // attribute mask
			&windowAttributes
			);
		
		XFree(visual);
		XMapWindow(xDisplay, win);
		
		// EGL context and surface
		eglBindAPI(EGL_OPENGL_API);
		const EGLint moreAttribs[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
			EGL_CONTEXT_MINOR_VERSION_KHR, 3,
			EGL_NONE
			};
		windowContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, moreAttribs);
		ASSERT(windowContext != EGL_NO_CONTEXT);
		windowSurface = eglCreateWindowSurface(eglDisplay, config, win, nullptr);
		ASSERT(windowSurface != EGL_NO_SURFACE);
		eglMakeCurrent(eglDisplay, windowSurface, windowSurface, windowContext);
	}
	
	void swapBuffer()
	{
		eglSwapBuffers(eglDisplay, windowSurface);
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
					//xkb_state_update_key(state, event.xkey.keycode, XKB_KEY_DOWN);
					//update_modifiers ();
				}
				else if (event.type == KeyRelease) {
					interface->keyPress(event.xkey.keycode - 8, false);
					//xkb_state_update_key(state, event.xkey.keycode, XKB_KEY_UP);
					//update_modifiers ();
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
	
	string getKeymap()
	{
		warning("BackendEGL::getKeymap not yet implemented");
		return "";
	}
	
	void * getXDisplay()
	{
		return xDisplay;
	}
};

unique_ptr<Backend> Backend::makeEGL(V2i dim)
{
	return make_unique<BackendEGL>(dim);
}

