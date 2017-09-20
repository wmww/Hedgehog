#pragma once

#include "../main/util.h"
#include "../scene/InputInterface.h"

class Backend
{
public:
	
	virtual void swapBuffer() = 0;
	virtual void checkEvents() = 0;
	virtual string getKeymap() = 0;
	
	void setInputInterface(weak_ptr<InputInterface> ptr) { inputInterface = ptr; }
	
	static void setup(V2i dim)
	{
		ASSERT_ELSE(instance == nullptr, return);
		instance = makeDRM();
	}
	
	static unique_ptr<Backend> instance; // defined in main.cpp because there is no Backend.cpp
	
protected:
	
	static unique_ptr<Backend> makeGLX(V2i dim);
	static unique_ptr<Backend> makeEGL(V2i dim);
	static unique_ptr<Backend> makeDRM();
	
	weak_ptr<InputInterface> inputInterface;
};

