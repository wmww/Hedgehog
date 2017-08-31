#pragma once

#include "../main/Surface2D.h"
#include "../main/utils.h"
#include "WaylandServer.h"

#include <wayland-server-core.h>

class WaylandSurface
{
public:
	WaylandSurface(wl_client * client, uint32_t id);
	static void makeXdgShellV6Surface(wl_client * client, uint32_t id, wl_resource * surface);
	
private:
	WaylandSurface(wl_resource * resource); // to use this the resource must have been created by this class
	struct Impl;
	weak_ptr<Impl> impl;
};

