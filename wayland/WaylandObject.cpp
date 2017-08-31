#include "WaylandObject.h"
#include <wayland-server.h>

std::unordered_map<uint32_t, shared_ptr<WaylandObject>> waylandObjectMap;

void destroyWaylandObject(wl_resource * resource)
{
	uint32_t id = resource->object.id;
	auto iter = waylandObjectMap.find(resource->object.id);
	if (iter == waylandObjectMap.end())
	{
		warning(string(__FUNCTION__) + " called with resource with id not in map (id: " + to_string(id) + ")");
		return;
	}
	waylandObjectMap.erase(iter);
}

void WaylandObject::wlSetup(wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct)
{
	wl_resource * resource = wl_resource_create(client, interface, version, id);
	wl_resource_set_implementation(resource, implStruct, nullptr, destroyWaylandObject);
	id = id;
	waylandObjectMap[id] = shared_from_this();
}

shared_ptr<WaylandObject> WaylandObject::getWaylandObject(wl_resource * resource)
{
	assert(resource != nullptr);
	uint32_t id = resource->object.id;
	auto iter = waylandObjectMap.find(id);
	if (iter == waylandObjectMap.end())
	{
		warning(string(__FUNCTION__) + " called on WaylandObject with id not in map (id: " + to_string(id) + ")");
		return shared_ptr<WaylandObject>(nullptr);
	}
	return iter->second;
}
