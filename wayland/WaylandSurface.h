#pragma once

#include "../main/Surface2D.h"
#include "../main/utils.h"
#include "WaylandServer.h"
#include "../opengl/Texture.h"
#include "../main/InputInterface.h"
#include "Resource.h"

#include <wayland-server-core.h>

class WaylandSurface
{
public:
	// constructors
	WaylandSurface() {}
	WaylandSurface(wl_client * client, uint32_t id);
	static WaylandSurface getFrom(Resource resource); // to use this the resource must have been created by this class
	
	// doers
	static void runFrameCallbacks();
	
	// getters
	//wl_resource * getSurfaceResource();
	weak_ptr<InputInterface> getInputInterface();
	Texture getTexture();
	
private:
	struct Impl;
	weak_ptr<Impl> impl; // can be null
};

