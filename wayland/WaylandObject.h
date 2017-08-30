#include "WaylandServer.h"
#include <wayland-server-core.h>
#include <unordered_map>

struct WlObjSetupData
{
	const wl_client * client;
	const wl_interface * interface;
	const int version;
	const uint32_t id;
	const void * implStruct;
};

template <typename T, const char * wlClassName>
class WaylandObject: public std::enable_shared_from_this<WaylandObject<T, wlClassName>>
{
public:
	
	static_assert(
        std::is_base_of<WaylandObject<T, wlClassName>, typename T::Impl>::value, 
        "WaylandObject can only be inherited from T::Impl"
    );
	
	static std::unordered_map<typename T::Impl *, shared_ptr<typename T::Impl> > map;
	
	WaylandObject(WlObjSetupData data)
	{
		wl_resource * resource = wl_resource_create(data.client, data.interface, data.version, data.id);
		wl_resource_set_implementation(resource, data.implStruct, this, deleteObj);
		map[this] = shared_ptr<WaylandObject<T, wlClassName>>(this);
	}
	
	static void deleteObj(wl_resource * resource)
	{
		auto rawPtr = getRawPtrFrom(resource);
		assert(rawPtr != nullptr);
		auto iter = map.find(rawPtr);
		if (iter == map.end())
		{
			warning("deleteObj called for Impl not in map");
		}
		else
		{
			map.erase(iter);
		}
	}
	
	static WaylandObject<T, wlClassName> * getRawPtrFrom(wl_resource * resource)
	{
		assert(string(wl_resource_get_class(resource)) == wlClassName);
		WaylandObject<T, wlClassName> * rawPtr = (WaylandObject<T, wlClassName> *)wl_resource_get_user_data(resource);
		return rawPtr;
	}
	
	static T getFrom(wl_resource * resource)
	{
		auto rawPtr = getRawPtrFrom(resource);
		assert(rawPtr != nullptr);
		auto iter = map.find(rawPtr);
		if (iter == map.end())
		{
			warning("getFrom called for Impl not in map");
		}
		else
		{
			return T(iter->second);
		}
		return T(nullptr);
	}
};

