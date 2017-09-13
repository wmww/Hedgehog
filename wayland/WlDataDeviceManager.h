#pragma once

#include "WaylandServer.h"
#include "Resource.h"

class WlDataDeviceManager
{
public:
	WlDataDeviceManager() {}
	WlDataDeviceManager(wl_client * client, uint32_t id);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

