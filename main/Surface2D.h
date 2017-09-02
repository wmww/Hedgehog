#pragma once

#include "utils.h"
#include "../opengl/Texture.h"

class Surface2D
{
public:
	// constructors
	Surface2D() {}
	void setup();
	
	// use
	void draw();
	static void drawAll();
	
	// setters
	void setTexture(Texture texture);
	
	// getters
	Texture getTexture();
	
private:
	struct Impl;
	shared_ptr<Impl> impl;
	Surface2D(shared_ptr<Impl> impl) { this->impl = impl; }
};

