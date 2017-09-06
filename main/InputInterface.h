#pragma once

#include "utils.h"

class InputInterface
{
public:
	virtual void pointerMotion(V2d newPos) = 0;
	virtual void pointerLeave() = 0;
	virtual void pointerClick(bool down) = 0;
};
