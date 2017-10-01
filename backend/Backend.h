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
	
	enum Type {X11_GLX, X11_EGL, DRM};
	static void setup(Type type);
	
	static unique_ptr<Backend> instance;
	
protected:
	Backend();
	
	weak_ptr<InputInterface> inputInterface;
	
private:
	string keymapString;
};

