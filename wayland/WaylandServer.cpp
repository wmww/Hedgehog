#include "../main/utils.h"
#include "../opengl/Texture.h"
#include "WaylandServer.h"
#include "WaylandSurface.h"
#include "WlSeat.h"
#include "WlRegion.h"
#include "WlShellSurface.h"
#include "XdgShellV6Surface.h"

#include "std_headers/wayland-server.h"
#include "std_headers/wayland-server-protocol.h"
#include "protocols/xdg-shell-unstable-v6.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../backend/Backend.h"

// change to toggle debug statements on and off
#define debug debug_on

namespace WaylandServer
{

// no clue what this means
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

//PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
	
wl_display * display = nullptr;
	
struct wl_event_loop *eventLoop = nullptr;
int eventLoopFileDescriptor = 0;

// callbacks and interfaces
struct wl_compositor_interface compositorInterface = {
	
	// create surface
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("compositor interface create surface callback called");
		WaylandSurface(client, id);
	},

	// create region
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("compositor interface create region called");
		WlRegion(client, id);
	}
};

void compositorBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("compositorBindCallback called");
	if (version != 1)
	{
		warning("wl_compositor_interface version " + to_string(version) + " instead of 1");
	}
	//assert(version == 1);
	
	wl_resource * resource = wl_resource_create(client, &wl_compositor_interface, 1, id);
	wl_resource_set_implementation(resource, &compositorInterface, nullptr, nullptr);
};

struct wl_shell_interface shellInterface = {
	// get shell surface
	+[](wl_client * client, wl_resource * resource, uint32_t id, wl_resource * surface) {
		
		debug("shell interface get shell surface called");
		
		WlShellSurface(client, id, WaylandSurface::getFrom(surface));
	}
};

void shellBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("shellBindCallback called");
	assert(version == 1);
	
	wl_resource * resource = wl_resource_create(client, &wl_shell_interface, 1, id);
	wl_resource_set_implementation(resource, &shellInterface, nullptr, nullptr);
};


struct zxdg_shell_v6_interface xdgShellV6Interface {
	// destroy (must not be done while there are surfaces still alive)
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		warning("zxdg_shell_v6_interface destroy called (not yet implemented)");
	},
	// create_positioner
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
	{
		warning("zxdg_shell_v6_interface create_positioner called (not yet implemented)");
		
	},
	// get_xdg_surface
	+[](struct wl_client *client,struct wl_resource *resource,uint32_t id, struct wl_resource *surface)
	{
		debug("zxdg_shell_v6_interface get_xdg_surface called");
		
		XdgShellV6Surface(client, id, WaylandSurface::getFrom(surface));
	},
	// pong
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t serial)
	{
		warning("zxdg_shell_v6_interface pong called (not yet implemented)");
	}
};

void xdgShellV6BindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("xdgShellV6BindCallback called");
	assert(version == 1);
	
	wl_resource * resource = wl_resource_create(client, &zxdg_shell_v6_interface, 1, id);
	wl_resource_set_implementation(resource, &xdgShellV6Interface, nullptr, nullptr);
	
	/*
	wl_resource * resource = wl_resource_create(client, &xdg_shell_interface, version, id);
	//wl_resource_set_implementation(resource, &xdgShellInterface, nullptr, nullptr);
	wl_resource_set_dispatcher(resource, +[]
		(const void *implementation, void *target, uint32_t opcode, const struct wl_message *message, union wl_argument *arguments)
		{
			struct wl_resource * resource =  (wl_resource *)target;
			if (opcode != 1) {
				wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT, "use_unstable_version must be called first");
				return 0;
			}

			if (arguments[0].i != XDG_SHELL_VERSION_CURRENT) {
				string msg =
					"incompatible xdg_shell versions, server: " +
					to_string(XDG_SHELL_VERSION_CURRENT) +
					", client: " +
					to_string(arguments[0].i);
				
				wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT, msg.c_str());
				return 0;
			}

			wl_resource_set_implementation(resource, &xdgShellInterface, nullptr, nullptr);
			return 1;
		},
		nullptr, nullptr, nullptr);
	*/
};

void seatBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("seatBindCallback called");
	assert(version == 1);
	WlSeat(client, id);
	//wl_resource * seat = wl_resource_create(client, &wl_seat_interface, 1, id);
	//wl_resource_set_implementation(seat, &seatInterface, nullptr, nullptr);
	//wl_seat_send_capabilities(seat, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
	//wl_seat_send_capabilities(seat, WL_SEAT_CAPABILITY_POINTER);
	//wl_seat_send_capabilities(seat, 0);
};

void setup()
{
	debug("starting Wayland server");
	
	// get function pointer
	//glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	
	display = wl_display_create();
	
	// automatically find a free socket and connect it to the display
	wl_display_add_socket_auto(display);
	
	// create global objects
	wl_global_create(display, &wl_compositor_interface, 3, nullptr, compositorBindCallback);
	wl_global_create(display, &wl_shell_interface, 1, nullptr, shellBindCallback);
	wl_global_create(display, &zxdg_shell_v6_interface, 1, nullptr, xdgShellV6BindCallback);
	wl_global_create(display, &wl_seat_interface, 1, nullptr, seatBindCallback);
	
	wl_display_init_shm(display);
	
	eventLoop = wl_display_get_event_loop(display);
	eventLoopFileDescriptor = wl_event_loop_get_fd(eventLoop);
	
	debug("Wayland server setup done");
}
	
void shutdown()
{
	debug("shutting down Wayland server");
	wl_display_destroy(display);
	display = nullptr;
}
	
void iteration()
{
	wl_event_loop_dispatch(eventLoop, 0);
	wl_display_flush_clients(display);
}

uint32_t lastSerialNum = 1;

uint32_t nextSerialNum()
{
	lastSerialNum++;
	return lastSerialNum;
}

}

