#pragma once

#include "../main/utils.h"
#include "../main/InputInterface.h"

class Backend
{
public:
	
	class ImplBase;
	
	Backend() {}
	Backend(shared_ptr<ImplBase> implIn);
	void swapBuffer();
	void checkEvents();
	void setInputInterface(weak_ptr<InputInterface> inputInterface);
	void * getXDisplay(); // will be of type Display or null
	
	static Backend makeGLX(V2i dim);
	static Backend makeEGL(V2i dim);
	
	static Backend makeDefault(V2i dim) { return makeGLX(dim); }
	
	static Backend instance;
	
private:
	shared_ptr<ImplBase> impl;
};

