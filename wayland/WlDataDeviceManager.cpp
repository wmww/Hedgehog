#include <wayland-server-protocol.h>
#include "WlDataDeviceManager.h"
#include "Resource.h"
#include "WlArray.h"

// change to toggle debug statements on and off
#define debug debug_off

struct WlDataDeviceManager::Impl: Resource::Data
{
	// instance data
	Resource dataDeviceManager;
	Resource dataDevice;
	
	// interface
	static const struct wl_data_device_manager_interface dataDeviceManagerInterface;
	static const struct wl_data_device_interface dataDeviceInterface;
};

const struct wl_data_device_manager_interface WlDataDeviceManager::Impl::dataDeviceManagerInterface {
	// create_data_source
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
	{
		warning("wl_data_device_manager_interface.create_data_source not yet implemented");
	},
	// get_data_device
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *seat)
	{
		debug("wl_data_device_manager_interface.get_data_device called");
		IMPL_FROM(resource);
		ASSERT(impl->dataDevice.isNull());
		impl->dataDevice.setup(impl, client, id, &wl_data_device_interface, 1, &Impl::dataDeviceInterface);
	}
};

const struct wl_data_device_interface WlDataDeviceManager::Impl::dataDeviceInterface {
	// start_drag
	+[](struct wl_client *client, struct wl_resource *resource, struct wl_resource *source, struct wl_resource *origin, struct wl_resource *icon, uint32_t serial)
	{
		warning("wl_data_device_interface.start_drag not yet implemented");
	},
	// set_selection
	+[](struct wl_client *client, struct wl_resource *resource, struct wl_resource *source, uint32_t serial)
	{
		warning("wl_data_device_interface.set_selection not yet implemented");
	},
	// release
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		debug("wl_data_device_interface.release called");
		Resource(resource).destroy();
	},
};

WlDataDeviceManager::WlDataDeviceManager(wl_client * client, uint32_t id, uint version)
{
	debug("creating WlDataDeviceManager");
	auto impl = make_shared<Impl>();
	this->impl = impl;
	ASSERT(version <= wl_data_device_manager_MAX_VERSION);
	impl->dataDeviceManager.setup(impl, client, id, &wl_data_device_manager_interface, version, &Impl::dataDeviceManagerInterface);
}
