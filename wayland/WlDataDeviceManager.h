#pragma once

#include "WaylandServer.h"
#include "Resource.h"

const uint wl_data_device_manager_MAX_VERSION = 3;

class WlDataDeviceManager
{
public:
	WlDataDeviceManager() {}
	WlDataDeviceManager(wl_client * client, uint32_t id, uint version);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
};

