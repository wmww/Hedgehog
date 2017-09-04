#pragma once

#include "../main/utils.h"

class Backend
{
public:
	
	class ImplBase;
	
	class InputDelegate
	{
	public:
		virtual void pointerMotion(V2i newPos) = 0;
	};
	
	Backend() {}
	Backend(shared_ptr<ImplBase> implIn);
	void swapBuffer();
	void checkEvents();
	//void setInputDelegate(weak_ptr<InputDelegate> inputDelegate);
	void * getXDisplay(); // will be of type Display or null
	
	static Backend makeGLX(V2i dim);
	
	static Backend instance;
	
private:
	shared_ptr<ImplBase> impl;
};

