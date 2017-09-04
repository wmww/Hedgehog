#pragma once

#include "utils.h"

class InputInterface
{
public:
	virtual void pointerMotion(V2i newPos) = 0;
};
