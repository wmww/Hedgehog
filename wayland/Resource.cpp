#include "Resource.h"
#include <unordered_map>

// change to toggle debug statements on and off
#define debug debug_off

// the only purpose of this is to be the user data of wl_resources created wit this class
// its only use is error checking
int wlResourceMadeWithResourceClass;

struct Resource::Impl
{
	wl_resource * resource = nullptr;
	uint resourceVersion = 0;
	shared_ptr<Data> data;
	
	static std::unordered_map<wl_resource *, shared_ptr<Impl>> map;
	
	static void destroyCallback(wl_resource * resourceRaw)
	{
		auto resource = Resource(resourceRaw);
		auto impl = resource.impl.lock();
		ASSERT_ELSE(impl, return);
		auto iter = map.find(impl->resource);
		ASSERT_ELSE(iter != Impl::map.end(), return);
		ASSERT_ELSE(&*iter->second == &*impl, return);
		impl->resource = nullptr; // this shouldn't be needed as the entire object should be deleted next line, but best to be safe
		impl->data = nullptr;
		Impl::map.erase(iter);
		debug("'" + string(wl_resource_get_class(resourceRaw)) + "' removed, there are now " + to_string(Impl::map.size()) + " resources");
		return; // 'this' will be invalid now, so don't do anything else!
	}
};

std::unordered_map<wl_resource *, shared_ptr<Resource::Impl>> Resource::Impl::map;

void Resource::setup(shared_ptr<Data> dataIn, wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct)
{
	// its fine to call this multiple times on a single instance
	auto impl = make_shared<Impl>();
	this->impl = impl;
	wl_resource * resource = wl_resource_create(client, interface, version, id);
	ASSERT(Impl::map.find(resource) == Impl::map.end());
	Impl::map[resource] = impl;
	debug("'" + string(wl_resource_get_class(resource)) + "' created, there are now " + to_string(Impl::map.size()) + " resources");
	wl_resource_set_implementation(resource, implStruct, &wlResourceMadeWithResourceClass, Impl::destroyCallback);
	impl->resource = resource;
	impl->resourceVersion = version;
	impl->data = dataIn;
}

Resource::Resource(wl_resource * resource)
{
	ASSERT_ELSE(resource, return);
	auto iter = Impl::map.find(resource);
	ASSERT_ELSE(wl_resource_get_user_data(resource) == &wlResourceMadeWithResourceClass, return);
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

uint Resource::getVersion()
{
	IMPL_ELSE(return 0);
	return impl->resourceVersion;
}

bool Resource::check(uint version)
{
	IMPL_ELSE(return false);
	return impl->resourceVersion >= version;
}

void Resource::destroy()
{
	IMPL_ELSE(return);
	ASSERT_ELSE(impl->resource, return);
	wl_resource_destroy(impl->resource);
}

