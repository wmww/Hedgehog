#include "WlSeat.h"
#include "WaylandObject.h"

#include "std_headers/wayland-server-protocol.h"

// change to toggle debug statements on and off
#define debug debug_off

struct WlSeat::Impl: WaylandObject
{
	// instance data
	wl_resource * seatResource = nullptr;
	wl_resource * pointerResource = nullptr;
	
	// interface
	static const struct wl_seat_interface seatInterface;
	static const struct wl_pointer_interface pointerInterface;
};

const struct wl_pointer_interface WlSeat::Impl::pointerInterface = {
	
	// set_cursor
	+[](wl_client * client, wl_resource * resource, uint32_t serial, wl_resource * _surface, int32_t hotspot_x, int32_t hotspot_y)
	{
		warning("wl_pointer_interface::set_cursor called (not yet implemented)");
		//surface * surface = wl_resource_get_user_data(_surface);
		//cursor = surface;
	},
	
	// pointer release
	+[](wl_client * client, wl_resource *resource)
	{
		warning("wl_pointer_interface::pointer_release called (not yet implemented)");
	}
};

const struct wl_seat_interface WlSeat::Impl::seatInterface = {
	
	// get pointer
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("wl_seat_interface::get_pointer called");
		
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
		warning("wl_seat_interface::get_keyboard called (not yet implemented)");
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
		warning("wl_seat_interface::get_touch called (not yet implemented)");
	}
};

WlSeat::WlSeat(wl_client * client, uint32_t id)
{
	debug("creating WlSeat");
	auto implShared = make_shared<Impl>();
	implShared->seatResource = implShared->wlObjMake(client, id, &wl_seat_interface, 1, &Impl::seatInterface);
	wl_seat_send_capabilities(implShared->seatResource, WL_SEAT_CAPABILITY_POINTER);
	impl = implShared;
}
