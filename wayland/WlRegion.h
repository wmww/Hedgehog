#pragma once

#include "WlSurface.h"

class WlRegion
{
public:
	WlRegion(wl_client * client, uint32_t id, uint version);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};
