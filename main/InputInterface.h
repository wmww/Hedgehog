#pragma once

#include "utils.h"

class InputInterface
{
public:
	virtual void pointerMotion(V2d newPos) = 0;
};
