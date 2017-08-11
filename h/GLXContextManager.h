#pragma once

#include "utils.h"
#include <GL/glx.h>

struct GLXContextManagerBase;

typedef shared_ptr<GLXContextManagerBase> GLXContextManager;

struct GLXContextManagerBase
{
	static GLXContextManager make(V2i dim, bool verbose = false);
	
	virtual Display * getDisplay() = 0;
	
	virtual void swapBuffer() = 0;
	
	static GLXContextManager instance;
};

