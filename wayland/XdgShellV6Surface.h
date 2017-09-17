#pragma once

#include "WlSurface.h"

class XdgShellV6Surface
{
public:
	XdgShellV6Surface(wl_client * client, uint32_t id, uint version, WlSurface surface);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

