#pragma once

#include "WaylandServer.h"
#include <wayland-server-core.h>

class WlSeat
{
public:
	WlSeat() {}
	WlSeat(wl_client * client, uint32_t id);
	static void pointerMotion(V2d position, wl_resource * surface);
	static void pointerLeave(wl_resource * surface); // call this before pointerMove on the new surface
	static void pointerClick(bool down, wl_resource * surface);
	static WlSeat getFromClient(wl_client * client);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
	static shared_ptr<Impl> getImplFromSurface(wl_resource * surface);
};

