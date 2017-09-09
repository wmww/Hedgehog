#include "WlSeat.h"
#include "WaylandObject.h"

#include "std_headers/wayland-server-protocol.h"
#include <unordered_map>

// change to toggle debug statements on and off
#define debug debug_off

struct WlSeat::Impl: WaylandObject
{
	// instance data
	wl_resource * seatResource = nullptr;
	wl_resource * pointerResource = nullptr;
	wl_resource * keyboardResource = nullptr;
	wl_resource * currentSurface = nullptr;
	
	// static
	static std::unordered_map<wl_client *, weak_ptr<Impl>> clientToImpl;
	
	// interface
	static const struct wl_pointer_interface pointerInterface;
	static const struct wl_keyboard_interface keyboardInterface;
	static const struct wl_seat_interface seatInterface;
};

std::unordered_map<wl_client *, weak_ptr<WlSeat::Impl>> WlSeat::Impl::clientToImpl;

const struct wl_pointer_interface WlSeat::Impl::pointerInterface = {
	
	// set_cursor
	+[](wl_client * client, wl_resource * resource, uint32_t serial, wl_resource * _surface, int32_t hotspot_x, int32_t hotspot_y)
	{
		warning("wl_pointer_interface.set_cursor called (not yet implemented)");
		//surface * surface = wl_resource_get_user_data(_surface);
		//cursor = surface;
	},
	
	// pointer release
	+[](wl_client * client, wl_resource *resource)
	{
		debug("wl_pointer_interface.release called");
		GET_IMPL_FROM(resource);
		assert(resource == impl->pointerResource);
		wlObjDestroy(resource);
		impl->pointerResource = nullptr;
	}
};

const struct wl_keyboard_interface WlSeat::Impl::keyboardInterface {
	// release
	+[](wl_client * client, wl_resource *resource)
	{
		debug("wl_keyboard_interface.release called");
		GET_IMPL_FROM(resource);
		assert(resource == impl->keyboardResource);
		wlObjDestroy(resource);
		impl->keyboardResource = nullptr;
	}
};

const struct wl_seat_interface WlSeat::Impl::seatInterface = {
	
	// get pointer
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("wl_seat_interface.get_pointer called");
		
		GET_IMPL_FROM(resource);
		assert(impl->pointerResource == nullptr);
		
		impl->pointerResource = impl->wlObjMake(client, id, &wl_pointer_interface, 1, &pointerInterface);
		//wl_resource * pointer = wl_resource_create(client, &wl_pointer_interface, 1, id);
		//wl_resource_set_implementation(pointer, &pointerInterface, nullptr, nullptr);
		//get_client(client)->pointer = pointer;
	},
	// get keyboard
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("wl_seat_interface.get_keyboard called");
		
		GET_IMPL_FROM(resource);
		assert(impl->pointerResource == nullptr);
		
		impl->keyboardResource = impl->wlObjMake(client, id, &wl_keyboard_interface, 1, &keyboardInterface);
		//struct wl_resource *keyboard = wl_resource_create (client, &wl_keyboard_interface, 1, id);
		//wl_resource_set_implementation (keyboard, &keyboard_interface, NULL, NULL);
		//get_client(client)->keyboard = keyboard;
		//int fd, size;
		//backend_get_keymap (&fd, &size);
		//wl_keyboard_send_keymap (keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);
		////close (fd);
	},
	// get touch
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		warning("wl_seat_interface.get_touch called (not yet implemented)");
	}
};

WlSeat::WlSeat(wl_client * client, uint32_t id)
{
	debug("creating WlSeat");
	assert(client);
	auto iter = Impl::clientToImpl.find(client);
	if (iter != Impl::clientToImpl.end())
	{
		fatal("single client made multiple seats");
	}
	auto implShared = make_shared<Impl>();
	Impl::clientToImpl[client] = implShared;
	implShared->seatResource = implShared->wlObjMake(client, id, &wl_seat_interface, 1, &Impl::seatInterface);
	wl_seat_send_capabilities(implShared->seatResource, WL_SEAT_CAPABILITY_POINTER);
	impl = implShared;
}

void WlSeat::pointerMotion(V2d position, wl_resource * surface)
{
	auto impl = getImplFromSurface(surface);
	
	if (!impl)
	{
		warning(FUNC + " called on deleted WaylandObject or something stupid like that");
		return;
	}
	if (!impl->pointerResource)
	{
		warning(FUNC + " called on seat with no pointer resource");
		return;
	}
	
	if (impl->currentSurface != surface)
	{
		impl->currentSurface = surface;
		wl_pointer_send_enter(
			impl->pointerResource,
			WaylandServer::nextSerialNum(),
			impl->currentSurface,
			wl_fixed_from_double(position.x),
			wl_fixed_from_double(position.y)
			);
	}
	
	wl_pointer_send_motion(
		impl->pointerResource,
		timeSinceStartMili(),
		wl_fixed_from_double(position.x),
		wl_fixed_from_double(position.y)
		);
}

void WlSeat::pointerLeave(wl_resource * surface)
{
	auto impl = getImplFromSurface(surface);
	
	if (!impl)
	{
		warning(FUNC + " called on deleted WaylandObject or something stupid like that");
		return;
	}
	if (!impl->pointerResource)
	{
		warning(FUNC + " called on seat with no pointer resource");
		return;
	}
	if (!impl->currentSurface)
	{
		warning(FUNC + " called on seat with no surface");
		return;
	}
	
	wl_pointer_send_leave(
		impl->pointerResource,
		WaylandServer::nextSerialNum(),
		impl->currentSurface
		);
}

void WlSeat::pointerClick(uint button, bool down, wl_resource * surface)
{
	auto impl = getImplFromSurface(surface);
	
	if (!impl)
	{
		warning(FUNC + " called on deleted WaylandObject or something stupid like that");
		return;
	}
	if (!impl->pointerResource)
	{
		warning(FUNC + " called on seat with no pointer resource");
		return;
	}
	if (!impl->currentSurface)
	{
		warning(FUNC + " called on seat with no surface");
		return;
	}
	
	warning(down ? "mouse down" : "mouse up");
	assert(surface);
	assert(string(wl_resource_get_class(surface)) == "wl_surface");
	assert(impl->pointerResource);
	assert(string(wl_resource_get_class(impl->pointerResource)) == "wl_pointer");
	
	wl_pointer_send_button(
		impl->pointerResource,
		WaylandServer::nextSerialNum(),
		timeSinceStartMili(),
		button,
		down ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED
		);
}

WlSeat WlSeat::getFromClient(wl_client * client)
{
	assert(client);
	auto iter = Impl::clientToImpl.find(client);
	if (iter == Impl::clientToImpl.end())
	{
		warning(FUNC + " called with client not in clientToImpl");
		return WlSeat();
	}
	else
	{
		WlSeat seat;
		seat.impl = iter->second;
		return seat;
	}
}

shared_ptr<WlSeat::Impl> WlSeat::getImplFromSurface(wl_resource * surface)
{
	assert(surface);
	wl_client * client = surface->client;
	assert(client);
	auto impl = getFromClient(client).impl.lock();
	return impl;
}
