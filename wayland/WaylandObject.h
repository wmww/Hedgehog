#include "WaylandServer.h"
#include <wayland-server-core.h>
#include <unordered_map>

class WaylandObject: public std::enable_shared_from_this<WaylandObject>
{
public:
	
	static std::unordered_map<uint32_t, shared_ptr<WaylandObject>> map;
	
	void WaylandObjectSetup(wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct);
	
	static void deleteObj(wl_resource * resource);
	
	static shared_ptr<WaylandObject> getWaylandObjectFrom(wl_resource * resource);
	
	template<typename T>
	inline static shared_ptr<T> get(wl_resource * resource)
	{
		return std::static_pointer_cast<T>(getWaylandObjectFrom(resource));
	}
};

