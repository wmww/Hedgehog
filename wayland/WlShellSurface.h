#pragma once

#include "WlSurface.h"

class WlShellSurface
{
public:
	WlShellSurface(wl_client * client, uint32_t id, WaylandSurface surface);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

