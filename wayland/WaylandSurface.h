#pragma once

#include "../main/Surface2D.h"
#include "../main/utils.h"
#include "WaylandServer.h"

#include <wayland-server-core.h>

class WaylandSurface
{
public:
	// constructors
	WaylandSurface() {}
	WaylandSurface(wl_client * client, uint32_t id);
	static WaylandSurface getFrom(wl_resource * resource); // to use this the resource must have been created by this class
	
	//static void makeXdgShellV6Surface(wl_client * client, uint32_t id, wl_resource * surface);
	
	// getters
	wl_resource * getSurfaceResource();
	Surface2D getSurface2D();
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

