#include "WaylandObject.h"
#include <wayland-server.h>

std::unordered_map<uint32_t, shared_ptr<WaylandObject>> WaylandObject::map;

void WaylandObject::WaylandObjectSetup(wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct)
{
	wl_resource * resource = wl_resource_create(client, interface, version, id);
	wl_resource_set_implementation(resource, implStruct, nullptr, deleteObj);
	map[id] = shared_from_this();
}

void WaylandObject::deleteObj(wl_resource * resource)
{
	assert(resource != nullptr);
	auto iter = map.find(resource->object.id);
	if (iter == map.end())
	{
		warning("deleteObj called for ID not in map");
	}
	else
	{
		map.erase(iter);
	}
}

shared_ptr<WaylandObject> WaylandObject::getWaylandObjectFrom(wl_resource * resource)
{
	assert(resource != nullptr);
	auto iter = map.find(resource->object.id);
	if (iter == map.end())
	{
		warning("getWaylandObjectFrom called for ID not in map");
		return shared_ptr<WaylandObject>(nullptr);
	}
	else
	{
		return iter->second;
	}
}
