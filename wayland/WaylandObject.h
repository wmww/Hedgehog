#include "WaylandServer.h"
#include <wayland-server-core.h>

#pragma once

// these macros are convenient and allow for proper file/line num if assert fails
#define GET_IMPL shared_ptr<Impl> impl = this->impl.lock(); assert(impl);
#define GET_IMPL_ELSE shared_ptr<Impl> impl = this->impl.lock(); if (!impl)
#define GET_IMPL_FROM(resource) shared_ptr<Impl> impl = WaylandObject::get<Impl>(resource); assert(impl);

// WaylandObjects manage their own memory. You should always hold weak pointers to them.

class WaylandObject: public std::enable_shared_from_this<WaylandObject>
{
public:
	
	wl_resource * wlObjMake(wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct);
	
	static shared_ptr<WaylandObject> getWaylandObject(wl_resource * resource);
	
	template<typename T>
	inline static shared_ptr<T> get(wl_resource * resource)
	{
		return std::static_pointer_cast<T>(getWaylandObject(resource));
	}
	
	//wl_resource * getResource() {return resource;}
	
	static void wlObjDestroy(wl_resource * resource);
	
private:
};

