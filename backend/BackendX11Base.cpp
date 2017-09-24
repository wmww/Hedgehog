#include "BackendX11Base.h"

#include <linux/input.h> // for BTN_LEFT and maybe other stuff
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <xkbcommon/xkbcommon-x11.h>


// change to toggle debug statements on and off
#define debug debug_off

//bool libinput_setup();
//void libinput_destroy();
//void libinput_check_events(InputInterface * interface);

BackendX11Base::BackendX11Base(V2i dim)
{
	this->dim = dim;
	
	debug("opening X display");
	xDisplay = XOpenDisplay(0);
	
	debug("setting up XKB (keymap shit)");
	setupXKB();
}

BackendX11Base::~BackendX11Base()
{
	XDestroyWindow(xDisplay, window);
}

void BackendX11Base::setWindowName(string name)
{
	ASSERT(window != 0);
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
	//ASSERT_FATAL(libinput_setup());
	XSetWindowAttributes windowAttribs;
	windowAttribs.colormap = XCreateColormap(xDisplay, RootWindow(xDisplay, visual->screen), visual->visual, AllocNone);
	windowAttribs.border_pixel = 0;
	//windowAttribs.event_mask = StructureNotifyMask;
	windowAttribs.event_mask
		= ExposureMask
		| KeyPressMask
		| KeyReleaseMask
		| ButtonPressMask
		| ButtonReleaseMask
		| PointerMotionMask
		| EnterWindowMask
		| LeaveWindowMask
		| FocusChangeMask
		| StructureNotifyMask
		;
	
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
	
	setWindowName(name);
	
	ASSERT(window != 0);
	
	debug("mapping window...");
	
	XMapWindow(xDisplay, window);
}

uint BackendX11Base::x11BtnToLinuxBtn(uint x11Btn)
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

void BackendX11Base::checkEvents()
{
	/*
	if (auto input = inputInterface.lock())
	{
		libinput_check_events(&*input);
		//warning(FUNC + " not implemented");
	}
	*/
	
	XEvent event;
	while (XPending(xDisplay))
	{
		XNextEvent(xDisplay, &event);
		
		if (auto interface = inputInterface.lock())
		{
			switch (event.type)
			{
			case MotionNotify: {
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
				dim = V2i(event.xconfigure.width, event.xconfigure.height);
				glViewport(0, 0, dim.x, dim.y);
				break;
			
			case DestroyNotify:
				instance = nullptr;
				break;
			
			default:
				// ignore
				break;
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

void BackendX11Base::setupXKB()
{
	/*
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
	keymap = xkb_x11_keymap_new_from_device(xkbContext, xcbConnection, keyboardDeviceId, XKB_KEYMAP_COMPILE_NO_FLAGS);\
	*/
	struct xkb_rule_names rules;
	rules.rules = getenv("XKB_DEFAULT_RULES");
	rules.model = getenv("XKB_DEFAULT_MODEL");
	rules.layout = getenv("XKB_DEFAULT_LAYOUT");
	rules.variant = getenv("XKB_DEFAULT_VARIANT");
	rules.options = getenv("XKB_DEFAULT_OPTIONS");
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	ASSERT_ELSE(context, return);
	keymap = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);
	//xkb_state * xkbState = xkb_x11_state_new_from_device(keymap, xcbConnection, keyboardDeviceId);
}

string BackendX11Base::getKeymap()
{
	ASSERT_ELSE(keymap, return "");
	return xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
}

