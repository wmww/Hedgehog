#pragma once

#include "utils.h"
#include "../opengl/Texture.h"

class Surface2D
{
public:
	Surface2D();
	Texture getTexture();
	void draw();
	
	static void drawAll();
	
private:
	struct Impl;
	shared_ptr<Impl> impl = nullptr;
};

