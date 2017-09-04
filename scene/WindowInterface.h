#pragma once

#include "../main/utils.h"
#include "../main/InputInterface.h"
#include "../opengl/Texture.h"

class WindowInterface: public InputInterface
{
public:
	virtual void setSize(V2i size) = 0;
	
	Texture texture;
	
	// to implement:
	/*
	void pointerMotion(V2i newPos)
	{
		warning(FUNC + " not yet implemented");
	}
	
	void setSize(V2i size)
	{
		warning(FUNC + " not yet implemented");
	}
	*/
};

#include "Scene.h" // must be at bottom for include order
