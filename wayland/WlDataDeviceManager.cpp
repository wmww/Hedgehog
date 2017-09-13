#include "WlDataDeviceManager.h"
#include "Resource.h"
#include "WlArray.h"
#include "std_headers/wayland-server-protocol.h"

// change to toggle debug statements on and off
#define debug debug_off

struct WlDataDeviceManager::Impl: Resource::Data
{
	// instance data
	Resource resource;
	
	// interface
	static const struct wl_data_device_manager_interface deviceManagerInterface;
};

const struct wl_data_device_manager_interface WlDataDeviceManager::Impl::deviceManagerInterface {
	// create_data_source
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
	{
		warning("wl_data_device_manager_interface.create_data_source called (not yet implemented)");
	},
	// get_data_device
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *seat)
	{
		warning("wl_data_device_manager_interface.get_data_device called (not yet implemented)");
	}
};

WlDataDeviceManager::WlDataDeviceManager(wl_client * client, uint32_t id)
{
	debug("creating WlDataDeviceManager");
	auto impl = make_shared<Impl>();
	this->impl = impl;
	impl->resource.setup(impl, client, id, &wl_data_device_manager_interface, 1, &Impl::deviceManagerInterface);
}
