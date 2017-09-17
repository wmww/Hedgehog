#include "WlShellSurface.h"
#include "../scene/WindowInterface.h"
#include "Resource.h"
#include "WlSeat.h"

#include <wayland-server-protocol.h>

// change to toggle debug statements on and off
#define debug debug_off

struct WlShellSurface::Impl: Resource::Data, WindowInterface
{
	// instance data
	WlSurface waylandSurface;
	wl_client * client;
	Resource resource;
	
	void setSize(V2i size)
	{
		warning(FUNC + " not yet implemented");
	}
	
	weak_ptr<InputInterface> getInputInterface()
	{
		return waylandSurface.getInputInterface();
	}
	
	// interface
	static const struct wl_shell_surface_interface wlShellSurfaceInterface;
};

const struct wl_shell_surface_interface WlShellSurface::Impl::wlShellSurfaceInterface = {
	// shell surface pong
	+[](struct wl_client *client, wl_resource * resource, uint32_t serial)
	{
		warning("shell surface interface pong callback called (not yet implemented)");
	},
	// shell surface move
	+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial)
	{
		warning("shell surface interface move callback called (not yet implemented)");
	},
	// shell surface resize
	+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial, uint32_t edges)
	{
		warning("shell surface interface resize callback called (not yet implemented)");
	},
	// shell surface set toplevel
	+[](wl_client * client, wl_resource * resource)
	{
		debug("shell surface interface set toplevel callback called");
	},
	// shell surface set transient
	+[](wl_client * client, wl_resource * resource, wl_resource * parent, int32_t x, int32_t y, uint32_t flags)
	{
		warning("shell surface interface set transient callback called (not yet implemented)");
	},
	// shell surface set fullscreen
	+[](wl_client * client, wl_resource * resource, uint32_t method, uint32_t framerate, wl_resource * output)
	{
		warning("shell surface interface set fullscreen callback called (not yet implemented)");
	},
	// shell surface set popup
	+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial, wl_resource * parent, int32_t x, int32_t y, uint32_t flags)
	{
		warning("shell surface interface set popup callback called (not yet implemented)");
	},
	// shell surface set maximized
	+[](wl_client * client, wl_resource * resource, wl_resource * output)
	{
		warning("shell surface interface set maximized callback called (not yet implemented)");
	},
	// shell surface set title
	+[](wl_client * client, wl_resource * resource, const char * title)
	{
		warning("shell surface interface set title callback called (not yet implemented)");
	},
	// shell surface set class
	+[](wl_client * client, wl_resource * resource, const char * class_)
	{
		warning("shell surface interface set class callback called (not yet implemented)");
	},
};

WlShellSurface::WlShellSurface(wl_client * client, uint32_t id, uint version, WlSurface surface)
{
	debug("creating WlShellSurface");
	auto impl = make_shared<Impl>();
	this->impl = impl;
	impl->waylandSurface = surface;
	impl->client = client;
	impl->texture = surface.getTexture();
	Scene::instance.addWindow(impl);
	impl->resource.setup(impl, client, id, &wl_shell_surface_interface, version, &Impl::wlShellSurfaceInterface);
}
