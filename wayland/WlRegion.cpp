#include <wayland-server-protocol.h>
#include "WlRegion.h"
#include "Resource.h"

// change to toggle debug statements on and off
#define debug debug_off

struct Area
{
	virtual bool checkPoint(V2d pt) = 0;
};

struct EmptyArea: Area
{
	bool checkPoint(V2d pt)
	{
		return false;
	}
};

struct RectArea: Area
{
	V2d low;
	V2d high;
	
	RectArea(V2d low, V2d high)
	{
		this->low = low;
		this->high = high;
	}
	
	bool checkPoint(V2d pt)
	{
		return
			pt.x >= low.x &&
			pt.y >= low.y &&
			pt.x <= high.x &&
			pt.y <= high.y;
	}
};

struct UnionArea: Area
{
	unique_ptr<Area> a, b;
	
	UnionArea(unique_ptr<Area> a, unique_ptr<Area> b)
	{
		this->a = move(a);
		this->b = move(b);
	}
	
	bool checkPoint(V2d pt)
	{
		return a->checkPoint(pt) || b->checkPoint(pt);
	}
};

struct IntersectionArea: Area
{
	unique_ptr<Area> a, b;
	
	IntersectionArea(unique_ptr<Area> a, unique_ptr<Area> b)
	{
		this->a = move(a);
		this->b = move(b);
	}
	
	bool checkPoint(V2d pt)
	{
		return a->checkPoint(pt) && b->checkPoint(pt);
	}
};

struct InverseArea: Area
{
	unique_ptr<Area> a;
	
	InverseArea(unique_ptr<Area> a)
	{
		this->a = move(a);
	}
	
	bool checkPoint(V2d pt)
	{
		return !a->checkPoint(pt);
	}
};
struct WlRegion::Impl: Resource::Data
{
	// instance data
	unique_ptr<Area> data;
	
	//WaylandObject2<Impl> wlObj;
	Resource resource;
	
	// members
	Impl()
	{
		data = make_unique<EmptyArea>();
	}
	
	// interface
	static const struct wl_region_interface wlRegionInterface;
};

const struct wl_region_interface WlRegion::Impl::wlRegionInterface = {
	// destroy
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		debug("wl_region_interface::destroy called");
		Resource(resource).destroy();
	},
	// add
	+[](struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		debug("wl_region_interface::add called ("
			"x: " + to_string(x) + ", "
			"y: " + to_string(y) + ", "
			"width: " + to_string(width) + ", "
			"height: " + to_string(height) + ")");
		
		//GET_IMPL_FROM(resource);
		//auto impl = WaylandObject2<Impl>::get(resource);
		//assert(impl);
		IMPL_FROM(resource);
		if (
			x == INT32_MIN &&
			y == INT32_MIN &&
			width == INT32_MAX &&
			height == INT32_MAX
			)
		{
			impl->data = make_unique<InverseArea>(make_unique<EmptyArea>());
		}
		else
		{
			auto rect = make_unique<RectArea>(V2d(x, y), V2d(x + width, y + height));
			impl->data = make_unique<UnionArea>(move(impl->data), move(rect));
		}
	},
	// subtract
	+[](struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		debug("wl_region_interface::subtract called ("
			"x: " + to_string(x) + ", "
			"y: " + to_string(y) + ", "
			"width: " + to_string(width) + ", "
			"height: " + to_string(height) + ")");
		
		IMPL_FROM(resource);
		if (
			x == INT32_MIN &&
			y == INT32_MIN &&
			width == INT32_MAX &&
			height == INT32_MAX
			)
		{
			impl->data = make_unique<EmptyArea>();
		}
		else
		{
			auto rect = make_unique<RectArea>(V2d(x, y), V2d(x + width, y + height));
			impl->data = make_unique<IntersectionArea>(move(impl->data), make_unique<InverseArea>(move(rect)));
		}
	}
};

WlRegion::WlRegion(wl_client * client, uint32_t id, uint version)
{
	debug("creating WlRegion");
	auto impl = make_shared<Impl>();
	this->impl = impl;
	impl->resource.setup(impl, client, id, &wl_region_interface, version, &Impl::wlRegionInterface);
}
