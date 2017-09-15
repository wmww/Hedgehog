#pragma once

#include "../main/util.h"

class InputInterface
{
public:
	virtual void pointerMotion(V2d newPos) = 0;
	virtual void pointerLeave() = 0;
	// button is to be a linux button such as BTN_LEFT as defined in <linux/input-event-codes.h>
	virtual void pointerClick(uint button, bool down) = 0;
	// key is to be some sort of keycode
	virtual void keyPress(uint key, bool down) = 0;
};
