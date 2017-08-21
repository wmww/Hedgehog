#pragma once

#include "../main/Surface2D.h"
#include "../main/utils.h"

#include <wayland-server.h>

class WaylandSurface
{
public:
	static shared_ptr<WaylandSurface> make(wl_client * client, uint32_t id, VerboseToggle verboseToggle);
	static WaylandSurface getFrom(wl_resource * resource);
	
private:
	struct Impl;
	
	WaylandSurface(VerboseToggle verboseIn);
	WaylandSurface(shared_ptr<Impl> implIn) {impl = implIn;}
	
	shared_ptr<Impl> impl = nullptr;
};

