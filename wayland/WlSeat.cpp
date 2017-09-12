#include "WlSeat.h"
#include "Resource.h"

#include "std_headers/wayland-server-protocol.h"
#include <unordered_map>

// change to toggle debug statements on and off
#define debug debug_on

struct WlSeat::Impl: Resource::Data
{
	// instance data
	Resource seat;
	Resource pointer;
	Resource keyboard;
	Resource currentSurface;
	
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
		Resource(resource).destroy();
	}
};

const struct wl_keyboard_interface WlSeat::Impl::keyboardInterface {
	// release
	+[](wl_client * client, wl_resource *resource)
	{
		debug("wl_keyboard_interface.release called");
		Resource(resource).destroy();
	}
};

const struct wl_seat_interface WlSeat::Impl::seatInterface = {
	
	// get_pointer
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("wl_seat_interface.get_pointer called");
		
		IMPL_FROM(resource);
		ASSERT(impl->pointer.isNull());
		impl->pointer.setup(impl, client, id, &wl_pointer_interface, 1, &pointerInterface);
		//wl_resource * pointer = wl_resource_create(client, &wl_pointer_interface, 1, id);
		//wl_resource_set_implementation(pointer, &pointerInterface, nullptr, nullptr);
		//get_client(client)->pointer = pointer;
	},
	// get_keyboard
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("wl_seat_interface.get_keyboard called");
		
		IMPL_FROM(resource);
		ASSERT(impl->keyboard.isNull());
		impl->keyboard.setup(impl, client, id, &wl_keyboard_interface, 1, &keyboardInterface);
		//struct wl_resource *keyboard = wl_resource_create (client, &wl_keyboard_interface, 1, id);
		//wl_resource_set_implementation (keyboard, &keyboard_interface, NULL, NULL);
		//get_client(client)->keyboard = keyboard;
		//int fd, size;
		//backend_get_keymap (&fd, &size);
		//wl_keyboard_send_keymap (keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);
		////close (fd);
	},
	// get_touch
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		warning("wl_seat_interface.get_touch called (not yet implemented)");
	}
};

WlSeat::WlSeat(wl_client * client, uint32_t id)
{
	debug("creating WlSeat");
	auto iter = Impl::clientToImpl.find(client);
	if (iter != Impl::clientToImpl.end())
	{
		warning("single client made multiple seats");
	}
	auto impl = make_shared<Impl>();
	this->impl = impl;
	Impl::clientToImpl[client] = impl;
	impl->seat.setup(impl, client, id, &wl_seat_interface, 1, &Impl::seatInterface);
	wl_seat_send_capabilities(impl->seat.getRaw(), WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
}

void WlSeat::pointerMotion(V2d position, Resource surface)
{
	auto impl = getImplFromSurface(surface);
	
	ASSERT_ELSE(impl, return);
	ASSERT_ELSE(impl->pointer.isValid(), return);
	ASSERT_ELSE(surface.isValid(), return);
	
	if (impl->currentSurface.getRaw() != surface.getRaw())
	{
		impl->currentSurface = surface;
		wl_pointer_send_enter(
			impl->pointer.getRaw(),
			WaylandServer::nextSerialNum(),
			impl->currentSurface.getRaw(),
			wl_fixed_from_double(position.x),
			wl_fixed_from_double(position.y)
			);
	}
	
	wl_pointer_send_motion(
		impl->pointer.getRaw(),
		timeSinceStartMili(),
		wl_fixed_from_double(position.x),
		wl_fixed_from_double(position.y)
		);
}

void WlSeat::pointerLeave(Resource surface)
{
	auto impl = getImplFromSurface(surface);
	
	ASSERT_ELSE(impl, return);
	ASSERT_ELSE(impl->pointer.isValid(), return);
	ASSERT_ELSE(impl->currentSurface.isValid(), return);
	
	wl_pointer_send_leave(
		impl->pointer.getRaw(),
		WaylandServer::nextSerialNum(),
		impl->currentSurface.getRaw()
		);
}

void WlSeat::pointerClick(uint button, bool down, Resource surface)
{
	debug("mouse button '" + to_string(button) + "' " + (down ? "down" : "up"));
	
	auto impl = getImplFromSurface(surface);
	
	ASSERT_ELSE(impl, return);
	ASSERT_ELSE(impl->pointer.isValid(), return);
	ASSERT_ELSE(impl->currentSurface.isValid(), return);
	
	wl_pointer_send_button(
		impl->pointer.getRaw(),
		WaylandServer::nextSerialNum(),
		timeSinceStartMili(),
		button,
		down ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED
		);
}

void WlSeat::keyPress(uint key, bool down, Resource surface)
{
	debug("key '" + to_string(key) + "' " + (down ? "down" : "up"));
	
	auto impl = getImplFromSurface(surface);
	
	ASSERT_ELSE(impl, return);
	ASSERT_ELSE(impl->keyboard.isValid(), return);
	ASSERT_ELSE(impl->currentSurface.isValid(), return);
	
	wl_array wlArray;
	wl_array_init(&wlArray);
	
	wl_keyboard_send_enter(
		impl->keyboard.getRaw(),
		WaylandServer::nextSerialNum(),
		impl->currentSurface.getRaw(),
		&wlArray
		);
	
	wl_array_release(&wlArray);
	
	wl_keyboard_send_key(
		impl->keyboard.getRaw(),
		WaylandServer::nextSerialNum(),
		timeSinceStartMili(),
		key,
		down ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED
		);
	
	wl_keyboard_send_leave(
		impl->keyboard.getRaw(),
		WaylandServer::nextSerialNum(),
		impl->currentSurface.getRaw()
		);
}

WlSeat WlSeat::getFromClient(wl_client * client)
{
	ASSERT(client);
	auto iter = Impl::clientToImpl.find(client);
	ASSERT_ELSE(iter != Impl::clientToImpl.end(), return WlSeat());
	WlSeat seat;
	seat.impl = iter->second;
	return seat;
	
}

shared_ptr<WlSeat::Impl> WlSeat::getImplFromSurface(Resource surface)
{
	ASSERT_ELSE(surface.isValid(), return nullptr);
	ASSERT_ELSE(surface.getRaw(), return nullptr);
	wl_client * client = surface.getRaw()->client;
	auto impl = getFromClient(client).impl.lock();
	return impl;
}
