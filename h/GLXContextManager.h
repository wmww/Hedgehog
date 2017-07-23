#pragma once

#include "utils.h"

struct GLXContextManagerBase;

typedef shared_ptr<GLXContextManagerBase> GLXContextManager;

struct GLXContextManagerBase
{
	static GLXContextManager make(V2i dim, bool verbose = false);
	
	virtual void swapBuffer() = 0;
};

