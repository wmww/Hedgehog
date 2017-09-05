#pragma once

#include "WaylandServer.h"
#include <wayland-server-core.h>

class WlSeat
{
public:
	WlSeat(wl_client * client, uint32_t id);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

