#pragma once

#include "../main/util.h"
#include "InputInterface.h"
#include "WindowInterface.h"

class Scene
{
public:
	void setup();
	void addWindow(weak_ptr<WindowInterface> window);
	weak_ptr<InputInterface> getInputInterface();
	void draw();
	
	static Scene instance;
private:
	struct Impl;
	shared_ptr<Impl> impl;
};

