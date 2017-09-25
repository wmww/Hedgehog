#include "WlSeat.h"
#include "Resource.h"
#include "WlArray.h"
#include "WlSurface.h"
#include "../backend/Backend.h"
#include "../scene/Scene.h"

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
	
	.set_cursor = +[](wl_client * client, wl_resource * resource, uint32_t serial, wl_resource * _surface, int32_t hotspot_x, int32_t hotspot_y) {
		debug("wl_pointer.set_cursor called");
		IMPL_FROM(resource);
		if (_surface == nullptr)
		{
			Scene::instance.setCursor(Texture(), V2d(0, 0));
		}
		else
		{
			WlSurface surface = WlSurface::getFrom(Resource(_surface));
			Texture texture = surface.getTexture();
			ASSERT_ELSE(texture.isValid(), return);
			Scene::instance.setCursor(texture, V2d(0, 0));
		}
	},
	
	.release = +[](wl_client * client, wl_resource *resource) {
		debug("wl_pointer.release called");
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
	
	.get_pointer = +[](wl_client * client, wl_resource * resource, uint32_t id) {
		debug("wl_seat.get_pointer called");
		IMPL_FROM(resource);
		ASSERT(impl->pointer.isNull());
		impl->pointer.setup(
			impl,
			client,
			id,
			&wl_pointer_interface,
			wl_resource_get_version(resource),
			&pointerInterface
		);
	},
	.get_keyboard = +[](wl_client * client, wl_resource * resource, uint32_t id) {
		debug("wl_seat.get_keyboard called");
		IMPL_FROM(resource);
		ASSERT(impl->keyboard.isNull());
		impl->keyboard.setup(
			impl,
			client,
			id,
			&wl_keyboard_interface,
			wl_resource_get_version(resource),
			&keyboardInterface
		);
		Resource keyboard = impl->keyboard;
		if (keyboard.check(WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION))
		{
			wl_keyboard_send_repeat_info(keyboard.getRaw(), 0, 0);
		}
		if (keyboard.check(WL_KEYBOARD_KEYMAP_SINCE_VERSION))
		{
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
		}
	},
	.get_touch = +[](wl_client * client, wl_resource * resource, uint32_t id) {
		warning("wl_seat.get_touch not implemented");
	},
	.release = +[](wl_client * client, wl_resource * resource) {
		debug("wl_seat.release called");
		Resource(resource).destroy();
	}
};

WlSeat::WlSeat(wl_client * client, uint32_t id, uint version)
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
	ASSERT(version <= wl_seat_MAX_VERSION);
	impl->seat.setup(impl, client, id, &wl_seat_interface, version, &Impl::seatInterface);
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
	Resource pointer = impl->pointer;
	auto fixedPos = V2<wl_fixed_t>(wl_fixed_from_double(position.x), wl_fixed_from_double(position.y));
	if (impl->lastPointerSurfaceRaw != surface.getRaw())
	{
		if (pointer.check(WL_POINTER_ENTER_SINCE_VERSION))
		{
			impl->lastPointerSurfaceRaw = surface.getRaw();
			
			wl_pointer_send_enter(
				pointer.getRaw(),
				WaylandServer::nextSerialNum(),
				surface.getRaw(),
				fixedPos.x,
				fixedPos.y
			);
		}
	}
	else
	{
		if (pointer.check(WL_POINTER_MOTION_SINCE_VERSION))
		{
			wl_pointer_send_motion(
				pointer.getRaw(),
				timeSinceStartMili(),
				fixedPos.x,
				fixedPos.y
			);
		}
	}
	if (pointer.check(WL_POINTER_FRAME_SINCE_VERSION))
	{
		wl_pointer_send_frame(pointer.getRaw());
	}
	
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
	Resource pointer = impl->pointer;
	impl->lastPointerSurfaceRaw = nullptr;
	if (pointer.check(WL_POINTER_LEAVE_SINCE_VERSION))
	{
		wl_pointer_send_leave(
			pointer.getRaw(),
			WaylandServer::nextSerialNum(),
			surface.getRaw()
		);
	}
	if (pointer.check(WL_POINTER_FRAME_SINCE_VERSION))
	{
		wl_pointer_send_frame(pointer.getRaw());
	}
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
	Resource pointer = impl->pointer;
	
	if (pointer.check(WL_POINTER_BUTTON_SINCE_VERSION))
	{
		wl_pointer_send_button(
			pointer.getRaw(),
			WaylandServer::nextSerialNum(),
			timeSinceStartMili(),
			button,
			down ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED
		);
	}
	if (pointer.check(WL_POINTER_FRAME_SINCE_VERSION))
	{
		wl_pointer_send_frame(pointer.getRaw());
	}
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
	Resource keyboard = impl->keyboard;
	
	WlArray<uint> keysDownArray; // it will be empty for now
	
	if (keyboard.check(WL_KEYBOARD_ENTER_SINCE_VERSION))
	{
		wl_keyboard_send_enter(
			keyboard.getRaw(),
			WaylandServer::nextSerialNum(),
			surface.getRaw(),
			keysDownArray.getRaw()
		);
	}
	if (keyboard.check(WL_KEYBOARD_KEY_SINCE_VERSION))
	{
		wl_keyboard_send_key(
			keyboard.getRaw(),
			WaylandServer::nextSerialNum(),
			timeSinceStartMili(),
			key,
			down ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED
		);
	}
	if (keyboard.check(WL_KEYBOARD_LEAVE_SINCE_VERSION))
	{
		wl_keyboard_send_leave(
			keyboard.getRaw(),
			WaylandServer::nextSerialNum(),
			surface.getRaw()
		);
	}
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
