#include "WaylandServer.h"
#include <wayland-server-core.h>

#pragma once

// WaylandObjects manage their own memory. You should always hold weak pointers to them.

class WaylandObject: public std::enable_shared_from_this<WaylandObject>
{
public:
	
	void wlSetup(wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct);
	
	static shared_ptr<WaylandObject> getWaylandObject(wl_resource * resource);
	
	template<typename T>
	inline static shared_ptr<T> get(wl_resource * resource)
	{
		return std::static_pointer_cast<T>(getWaylandObject(resource));
	}
	
private:
};

