#pragma once

#include "Backend.h"

struct Backend::ImplBase
{
	// instance data
	weak_ptr<InputInterface> inputInterface;
	
	// virtual methods
	virtual void swapBuffer() = 0;
	virtual void checkEvents() = 0;
	virtual string getKeymap() = 0;
	virtual void * getXDisplay() = 0;
};

