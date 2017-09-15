#include "WlSeat.h"
#include "Resource.h"
#include "WlArray.h"
#include "../backend/Backend.h"

#include <wayland-server-protocol.h>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>

// change to toggle debug statements on and off
#define debug debug_off

struct WlSeat::Impl: Resource::Data
{
	// instance data
	Resource seat;
	Resource pointer;
	Resource keyboard;
	// these are ONLY used to see if the last surface to receive input matches the current one
	wl_resource * lastPointerSurfaceRaw = nullptr;
	//wl_resource * lastKeyboardSurfaceRaw = nullptr;
	
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
		auto null_fd = open("/dev/null", O_RDONLY);
		wl_keyboard_send_keymap(
			impl->keyboard.getRaw(),
			WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP,
			null_fd,
			0
			);
		close(null_fd);
		//int fd, size;
		ASSERT_ELSE(Backend::instance, return);
		string keymapString = Backend::instance->getKeymap();
		size_t dataSize = keymapString.size() + 1;
		string xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");
		ASSERT_ELSE(!xdgRuntimeDir.empty(), return);
		int fd = open(xdgRuntimeDir.c_str(), O_TMPFILE|O_RDWR|O_EXCL, 0600);
		ftruncate(fd, dataSize);
		void * data = mmap(nullptr, dataSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		memcpy(data, keymapString.c_str(), dataSize);
		munmap(data, dataSize);
		wl_keyboard_send_keymap(impl->keyboard.getRaw(), WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, dataSize);
		close(fd);
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
	
	if (!impl || impl->pointer.isNull())
	{
		debug("client has not created the needed objects");
		return;
	}
	ASSERT_ELSE(surface.isValid(), return);
	
	if (impl->lastPointerSurfaceRaw != surface.getRaw())
	{
		impl->lastPointerSurfaceRaw = surface.getRaw();
		
		wl_pointer_send_enter(
			impl->pointer.getRaw(),
			WaylandServer::nextSerialNum(),
			surface.getRaw(),
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
	
	if (!impl || impl->pointer.isNull())
	{
		debug("client has not created the needed objects");
		return;
	}
	ASSERT_ELSE(impl->lastPointerSurfaceRaw != nullptr, return);
	ASSERT_ELSE(impl->lastPointerSurfaceRaw == surface.getRaw(), return);
	
	impl->lastPointerSurfaceRaw = nullptr;
	
	wl_pointer_send_leave(
		impl->pointer.getRaw(),
		WaylandServer::nextSerialNum(),
		surface.getRaw()
		);
}

void WlSeat::pointerClick(uint button, bool down, Resource surface)
{
	debug("mouse button '" + to_string(button) + "' " + (down ? "down" : "up"));
	
	auto impl = getImplFromSurface(surface);
	
	if (!impl || impl->pointer.isNull())
	{
		debug("client has not created the needed objects");
		return;
	}
	ASSERT_ELSE(impl->lastPointerSurfaceRaw != nullptr, return);
	ASSERT_ELSE(impl->lastPointerSurfaceRaw == surface.getRaw(), return);
	
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
	
	if (!impl || impl->keyboard.isNull())
	{
		debug("client has not created the needed objects");
		return;
	}
	ASSERT_ELSE(impl->lastPointerSurfaceRaw != nullptr, return);
	ASSERT_ELSE(impl->lastPointerSurfaceRaw == surface.getRaw(), return);
	
	WlArray<uint> keysDownArray;
	
	wl_keyboard_send_enter(
		impl->keyboard.getRaw(),
		WaylandServer::nextSerialNum(),
		surface.getRaw(),
		keysDownArray.getRaw()
		);
	
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
		surface.getRaw()
		);
}

WlSeat WlSeat::getFromClient(wl_client * client)
{
	ASSERT(client);
	auto iter = Impl::clientToImpl.find(client);
	if (iter == Impl::clientToImpl.end())
	{
		debug("requested client has not made a seat");
		return WlSeat();
	}
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
