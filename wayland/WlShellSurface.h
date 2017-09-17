#pragma once

#include "WlSurface.h"

class WlShellSurface
{
public:
	WlShellSurface(wl_client * client, uint32_t id, uint version, WlSurface surface);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

