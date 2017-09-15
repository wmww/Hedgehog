#pragma once

#include <xkbcommon/xkbcommon.h> // for getting the keymap
#include <X11/Xutil.h>

#include "Backend.h"

class BackendX11Base: public Backend
{
protected:
	Display * xDisplay = nullptr;
	Window window;
	xkb_keymap * keymap = nullptr;
	V2i dim;
	
	void setupXKB();
	void setWindowName(string name);
	
private:
	static uint x11BtnToLinuxBtn(uint x11Btn);
	
public:
	void checkEvents();
	string getKeymap();
	void * getXDisplay() { return xDisplay; }
};
