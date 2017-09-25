#include "BackendX11Base.h"

#include <linux/input.h> // for BTN_LEFT and maybe other stuff
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// change to toggle debug statements on and off
#define debug debug_off

uint x11BtnToLinuxBtn(uint x11Btn)
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

BackendX11Base::BackendX11Base(V2i dim)
{
	this->dim = dim;
	
	debug("opening X display");
	xDisplay = XOpenDisplay(0);
	
	debug("setting up XKB (keymap shit)");
}

BackendX11Base::~BackendX11Base()
{
	XCloseDisplay(xDisplay);
}

void BackendX11Base::setWindowName(string name)
{
	ASSERT(window);
	// We use the XTextProperty structure to store the title.
	XTextProperty windowName;
	windowName.value = (unsigned char *)name.c_str();
	
	// XA_STRING is not defined, but its value appears to be 31
	// I should probably just find the right header to include, but hey, who doesn't love magic numbers?
	windowName.encoding = 31;
	// windowName.encoding = XA_STRING;
	
	windowName.format = 8;
	windowName.nitems = name.size();
	
	XSetWMName(xDisplay, window, &windowName);
}

void BackendX11Base::openWindow(XVisualInfo * visual, string name)
{
	XSetWindowAttributes windowAttribs;
	windowAttribs.colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, visual->screen), visual->visual, AllocNone);
	windowAttribs.border_pixel = 0;
	windowAttribs.event_mask =
		ExposureMask			|
		KeyPressMask			|
		KeyReleaseMask			|
		ButtonPressMask			|
		ButtonReleaseMask		|
		PointerMotionMask		|
		EnterWindowMask			|
		LeaveWindowMask			|
		FocusChangeMask			|
		StructureNotifyMask		;
	
	int x = 0;
	int y = 0;
	
	debug("creating window");
	
	window = XCreateWindow(
		xDisplay,
		RootWindow(xDisplay, visual->screen), // parent
		x, y, dim.x, dim.y, // geometry
		0,
		visual->depth,
		InputOutput,
		visual->visual,
		CWBorderPixel|CWColormap|CWEventMask,
		&windowAttribs
	);
	
	Atom WM_DELETE_WINDOW = XInternAtom(xDisplay, "WM_DELETE_WINDOW", false); 
    XSetWMProtocols(xDisplay, window, &WM_DELETE_WINDOW, 1);
	
	setWindowName(name);
	
	ASSERT(window != 0);
	
	debug("mapping window");
	
	XMapWindow(xDisplay, window);
}

void BackendX11Base::checkEvents()
{
	auto interface = inputInterface.lock();
	if (!interface)
		return;
	
	while (XPending(xDisplay))
	{
		XEvent event;
		XNextEvent(xDisplay, &event);
		
		switch (event.type)
		{
			case MotionNotify: {
				// pointer motion
				auto movement = V2d((double)event.xbutton.x / dim.x, 1 - (double)event.xbutton.y / dim.y);
				interface->pointerMotion(movement);
			}	break;
				
			case ButtonPress:
				interface->pointerClick(x11BtnToLinuxBtn(event.xbutton.button), true);
				break;
				
			case ButtonRelease:
				interface->pointerClick(x11BtnToLinuxBtn(event.xbutton.button), false);
				break;
				
			case KeyPress:
				interface->keyPress(event.xkey.keycode - 8, true);
				break;
				
			case KeyRelease:
				interface->keyPress(event.xkey.keycode - 8, false);
				break;
				
			case ConfigureNotify:
				// window was resized
				dim = V2i(event.xconfigure.width, event.xconfigure.height);
				glViewport(0, 0, dim.x, dim.y);
				break;
			
			case DestroyNotify:
				// no need to use this, do stuff in the destructor instead
				break;
			
			case ClientMessage:
				// assume this is a window close event because that is the only atom we support
				debug("X window closed");
				XDestroyWindow(xDisplay, window);
				instance = nullptr;
				return; // 'this' will now be invalid
				
			default:
				// ignore other events
				break;
		}
	}
}


