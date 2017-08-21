#pragma once

#include "utils.h"
#include "../opengl/Texture.h"

class Surface2D
{
public:
	Surface2D(VerboseToggle verboseToggle);
	Texture getTexture();
	void draw();
	
private:
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

