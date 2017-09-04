#pragma once

#include "../main/Surface2D.h"
#include "../main/utils.h"
#include "WaylandServer.h"
#include "../opengl/Texture.h"

#include <wayland-server-core.h>

class WaylandSurface
{
public:
	// constructors
	WaylandSurface() {}
	WaylandSurface(wl_client * client, uint32_t id);
	static WaylandSurface getFrom(wl_resource * resource); // to use this the resource must have been created by this class
	
	// getters
	//wl_resource * getSurfaceResource();
	Texture getTexture();
	
private:
	struct Impl;
	weak_ptr<Impl> impl; // can be null
};

