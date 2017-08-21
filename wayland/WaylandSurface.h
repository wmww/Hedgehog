#pragma once

#include "../main/Surface2D.h"

class WaylandSurface: MessageLogger
{
public:
	WaylandSurface(wl_client * client, uint32_t id, VerboseToggle verboseToggle);
	
private:
	Surface2D surface2D;
	struct wl_resource * buffer = nullptr;
};

