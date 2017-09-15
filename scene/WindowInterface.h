#pragma once

#include "../main/util.h"
#include "InputInterface.h"
#include "../opengl/Texture.h"

class WindowInterface
{
public:
	virtual void setSize(V2i size) = 0;
	virtual weak_ptr<InputInterface> getInputInterface() = 0;
	
	Texture texture;
	
	// to implement:
	/*
	void pointerMotion(V2i newPos)
	{
		warning(FUNC + " not yet implemented");
	}
	
	weak_ptr<InputInterface> getInputInterface()
	{
		warning(FUNC + " not yet implemented");
	}
	*/
};

#include "Scene.h" // must be at bottom for include order
