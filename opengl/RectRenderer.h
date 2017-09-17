#pragma once

#include "../main/util.h"
#include "Texture.h"

// this class takes some work to initialize and needs an OpenGL context to be bound when created
class RectRenderer
{
public:
	RectRenderer();
	RectRenderer(const RectRenderer& that) = delete;
	~RectRenderer();
	void draw(Texture texture);
	
private:
	uint vertexArrayID = 0;
};
