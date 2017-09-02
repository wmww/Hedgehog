#include "WaylandObject.h"
#include <wayland-server.h>
#include <unordered_map>

// this is simply a way of storing a single shared_ptr to each WaylandObject so they are not deleted until they are removed from this map
// the advantage of this over manual memory management is that it is easy to check what objects have leaked and detect dangling pointers
// (if a map lookup finds nothing, the object has been deleted)
std::unordered_map<WaylandObject *, shared_ptr<WaylandObject>> waylandObjectMap;

void destroyWaylandObject(wl_resource * resource)
{
	auto iter = waylandObjectMap.find((WaylandObject *)wl_resource_get_user_data(resource));
	if (iter == waylandObjectMap.end())
	{
		warning(string(__FUNCTION__) + " called with resource with id not in map");
		return;
	}
	waylandObjectMap.erase(iter);
}

void WaylandObject::wlSetup(wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct)
{
	resource = wl_resource_create(client, interface, version, id);
	wl_resource_set_implementation(resource, implStruct, this, destroyWaylandObject);
	assert(waylandObjectMap.find(this) == waylandObjectMap.end());
	waylandObjectMap[this] = shared_from_this();
}

shared_ptr<WaylandObject> WaylandObject::getWaylandObject(wl_resource * resource)
{
	assert(resource != nullptr);
	auto iter = waylandObjectMap.find((WaylandObject *)wl_resource_get_user_data(resource));
	if (iter == waylandObjectMap.end())
	{
		warning(string(__FUNCTION__) + " called on WaylandObject with id not in map");
		return shared_ptr<WaylandObject>(nullptr);
	}
	return iter->second;
}

void WaylandObject::destroy()
{
	wl_resource_destroy(resource);
}

