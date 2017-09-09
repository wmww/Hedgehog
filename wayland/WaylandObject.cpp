#include "WaylandObject.h"
#include "std_headers/wayland-server.h"
#include <unordered_map>

// this is simply a way of storing a single shared_ptr to each WaylandObject so they are not deleted until they are removed from this map
// the advantage of this over manual memory management is that it is easy to check what objects have leaked and detect dangling pointers
// (if a map lookup finds nothing, the object has been deleted)
std::unordered_map<wl_resource *, shared_ptr<WaylandObject>> waylandObjectMap;

void destroyWaylandObject(wl_resource * resource)
{
	auto iter = waylandObjectMap.find(resource);
	if (iter == waylandObjectMap.end())
	{
		warning(FUNC + " called with resource with id not in map");
		return;
	}
	waylandObjectMap.erase(iter);
}

wl_resource * WaylandObject::wlObjMake(wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct)
{
	wl_resource * resource = wl_resource_create(client, interface, version, id);
	wl_resource_set_implementation(resource, implStruct, nullptr, destroyWaylandObject);
	assert(waylandObjectMap.find(resource) == waylandObjectMap.end());
	waylandObjectMap[resource] = shared_from_this();
	return resource;
}

shared_ptr<WaylandObject> WaylandObject::getWaylandObject(wl_resource * resource)
{
	assert(resource != nullptr);
	auto iter = waylandObjectMap.find(resource);
	if (iter == waylandObjectMap.end())
	{
		warning(string(__FUNCTION__) + " called on WaylandObject with id not in map");
		return shared_ptr<WaylandObject>(nullptr);
	}
	return iter->second;
}

void WaylandObject::wlObjDestroy(wl_resource * resource)
{
	wl_resource_destroy(resource);
	//*resource = nullptr;
}

