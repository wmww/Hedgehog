#pragma once

#include "../main/util.h"
#include "Texture.h"

// this class takes some work to initialize and needs an OpenGL context to be bound when created
class RectRenderer
{
public:
	RectRenderer();
	RectRenderer(const RectRenderer& that) = delete; // you may not copy this class, use a shared_ptr if you must
	~RectRenderer();
	void draw(Texture texture, V2d pos, V2d size);
	
private:
	uint vertexArrayID = 0;
};
