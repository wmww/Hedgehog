#pragma once

#include "../main/util.h"
#include <wayland-server-core.h>

const int wl_display_VERSION		= 1;
const int wl_callback_VERSION		= 1;
const int wl_compositor_VERSION		= 4;
const int wl_shm_pool_VERSION		= 1;
const int wl_shm_VERSION			= 1;
const int wl_buffer_VERSION			= 1;
const int wl_data_offer_VERSION		= 3;
const int wl_data_source_VERSION	= 3;
const int wl_data_device_VERSION	= 3;
const int wl_data_device_manager_VERSION = 3;
const int wl_shell_VERSION			= 1;
const int wl_shell_surface_VERSION	= 1;
//const int zzzzzzzzz _VERSION		= 1;
//const int zzzzzzzzz _VERSION		= 1;
//const int zzzzzzzzz _VERSION		= 1;
//const int zzzzzzzzz _VERSION		= 1;
//const int zzzzzzzzz _VERSION		= 1;

// these macros are convenient and allow for proper file/line num if assert fails
#define IMPL_ELSE(action) shared_ptr<Impl> impl = this->impl.lock(); ASSERT_ELSE(impl, action);
#define IMPL_FROM(resource) shared_ptr<Impl> impl = Resource(resource).get<Impl>(); if (!impl) { warning(FUNC + " called with invalid resource"); return; }

// an object that can be retrieved from a resource must inherit from Resource::Data
// always hold weak pointers to objects inheriting from Resource::Data

class Resource
{
public:
	
	class Data {}; // the only purpose of this is to inherit from
	
	Resource() {}
	//Resource(shared_ptr<Data> dataIn, wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct);
	void setup(shared_ptr<Data> dataIn, wl_client * client, uint32_t id, const wl_interface * interface, int version, const void * implStruct);
	explicit Resource(wl_resource * resourceIn);
	
	inline bool isNull() { return impl.expired(); };
	inline bool isValid() { return !isNull(); };
	
	shared_ptr<Data> getData();
	
	template<typename T>
	inline shared_ptr<T> get()
	{
		return std::static_pointer_cast<T>(getData());
	}
	
	wl_resource * getRaw();
	uint getVersion();
	
	void destroy();
	
private:
	
	struct Impl;
	weak_ptr<Impl> impl;
};
