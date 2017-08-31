#pragma once

#include "WaylandSurface.h"

class XdgShellV6Surface
{
public:
	XdgShellV6Surface(wl_client * client, uint32_t id, WaylandSurface surface);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

