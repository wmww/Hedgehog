#pragma once

#include "../main/util.h"
#include "../scene/InputInterface.h"

class Backend
{
public:
	
	virtual void swapBuffer() = 0;
	virtual void checkEvents() = 0;
	string getKeymap() { return keymapString; }
	
	void setInputInterface(weak_ptr<InputInterface> ptr) { inputInterface = ptr; }
	
	enum Type {GLX, EGL, DRM};
	
	static void setup(Type type)
	{
		ASSERT_ELSE(instance == nullptr, return);
		static const V2i defaultDim = V2i(800, 800);
		switch (type)
		{
		case GLX:
			instance = makeGLX(defaultDim);
			break;
		case EGL:
			instance = makeEGL(defaultDim);
			break;
		case DRM:
			instance = makeDRM();
			break;
		}
		ASSERT(instance);
	}
	
	static unique_ptr<Backend> instance;
	
protected:
	
	Backend();
	static unique_ptr<Backend> makeGLX(V2i dim);
	static unique_ptr<Backend> makeEGL(V2i dim);
	static unique_ptr<Backend> makeDRM();
	
	weak_ptr<InputInterface> inputInterface;
	
private:
	
	string keymapString;
};

