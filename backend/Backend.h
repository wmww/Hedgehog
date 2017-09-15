#pragma once

#include "../main/util.h"
#include "../scene/InputInterface.h"

class Backend
{
public:
	
	virtual void swapBuffer() = 0;
	virtual void checkEvents() = 0;
	virtual string getKeymap() = 0;
	virtual void * getXDisplay() = 0; // will be of type Display * or null
	
	void setInputInterface(weak_ptr<InputInterface> ptr) { inputInterface = ptr; }
	
	static void setup(V2i dim)
	{
		ASSERT_ELSE(instance == nullptr, return);
		instance = makeEGL(dim);
	}
	
	static unique_ptr<Backend> instance; // defined in main.cpp because there is no Backend.cpp
	
protected:
	
	static unique_ptr<Backend> makeGLX(V2i dim);
	static unique_ptr<Backend> makeEGL(V2i dim);
	
	weak_ptr<InputInterface> inputInterface;
};

