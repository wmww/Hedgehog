#include "Resource.h"
#include <unordered_map>

struct Resource::Impl
{
	wl_resource * resource = nullptr;
	shared_ptr<Data> data;
	
	static std::unordered_map<wl_resource *, shared_ptr<Impl>> map;
	
	static void destroyWaylandResource(wl_resource * resource)
	{
		Resource(resource).destroy();
	}
};

std::unordered_map<wl_resource *, shared_ptr<Resource::Impl>> Resource::Impl::map;

Resource::Resource(shared_ptr<Data> dataIn, wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct)
{
	auto impl = make_shared<Impl>();
	this->impl = impl;
	wl_resource * resource = wl_resource_create(client, interface, version, id);
	wl_resource_set_implementation(resource, implStruct, nullptr, Impl::destroyWaylandResource);
	impl->resource = resource;
	impl->data = dataIn;
	ASSERT(Impl::map.find(resource) == Impl::map.end());
	Impl::map[resource] = impl;
}

Resource::Resource(wl_resource * resource)
{
	ASSERT_ELSE(resource, return);
	auto iter = Impl::map.find(resource);
	ASSERT_ELSE(iter != Impl::map.end(), return);
	ASSERT(iter->second->resource == resource);
	impl = iter->second;
}

shared_ptr<Resource::Data> Resource::getData()
{
	IMPL_ELSE(return nullptr)
	return impl->data;
}

wl_resource * Resource::getRaw()
{
	IMPL_ELSE(return nullptr);
	return impl->resource;
}

void Resource::destroy()
{
	IMPL_ELSE(return);
	auto iter = Impl::map.find(impl->resource);
	if (iter == Impl::map.end())
	{
		warning("Resource::destroy called with wl_resource * not in map");
		return;
	}
	if (iter->second)
		iter->second->resource = nullptr; // this shouldn't be needed as the entire object should be deleted next line, but best to be safe
	Impl::map.erase(iter);
}

