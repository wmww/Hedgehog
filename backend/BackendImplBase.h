#pragma once

#include "Backend.h"

struct Backend::ImplBase
{
	// instance data
	//weak_ptr<InputDelegate> inputDelegate;
	
	// virtual methods
	virtual void swapBuffer() = 0;
	virtual void checkEvents() = 0;
	virtual void * getXDisplay() = 0;
};

