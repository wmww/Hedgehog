#include "WlRegion.h"
#include "WaylandObject.h"

#include "std_headers/wayland-server-protocol.h"

// change to toggle debug statements on and off
#define debug debug_on

/*
struct RegionBase
{
	virtual bool checkPoint(V2d pt);
};

struct RegionRect: RegionBase
{
	V2d low;
	V2d high;
	
	bool checkPoint(V2d pt)
	{
		return
			pt.x >= low.x &&
			pt.y >= low.y &&
			pt.x <= high.x &&
			pt.y <= high.y;
	}
};
* */

struct WlRegion::Impl: WaylandObject
{
	// instance data
	
	// interface
	static const struct wl_region_interface wlRegionInterface;
};

const struct wl_region_interface WlRegion::Impl::wlRegionInterface = {
	// destroy
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		debug("wl_region_interface::destroy called");
		wlObjDestroy(resource);
	},
	// add
	+[](struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		warning("wl_region_interface::add called (not yet implemented)");
	},
	// subtract
	+[](struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		warning("wl_region_interface::subtract called (not yet implemented)");
	}
};

WlRegion::WlRegion(wl_client * client, uint32_t id)
{
	debug("creating WlRegion");
	auto implShared = make_shared<Impl>();
	implShared->wlObjMake(client, id, &wl_region_interface, 1, &Impl::wlRegionInterface);
	impl = implShared;
}
