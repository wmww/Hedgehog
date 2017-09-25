#pragma once

#include <xkbcommon/xkbcommon.h> // for getting the keymap
#include <X11/Xutil.h>

#include "Backend.h"

class BackendX11Base: public Backend
{
protected:
	Display * xDisplay = nullptr;
	Window window;
	V2i dim; // window dimensions
	
	void openWindow(XVisualInfo * visual, string name);
	void setWindowName(string name);
	
public:
	BackendX11Base(V2i dim);
	~BackendX11Base();
	void checkEvents();
};
