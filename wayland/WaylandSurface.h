#pragma once

#include "../main/Surface2D.h"
#include "../main/utils.h"

#include <wayland-server.h>

class WaylandSurface
{
public:
	WaylandSurface(wl_client * client, uint32_t id);
	static WaylandSurface getFrom(wl_resource * resource);
	
private:
	struct Impl;
	
	WaylandSurface(shared_ptr<Impl> implIn) {impl = implIn;}
	
	shared_ptr<Impl> impl = nullptr;
};

