#include "../main/utils.h"
#include "../opengl/Texture.h"
#include "WaylandServer.h"
#include "WaylandSurface.h"

#include <wayland-server-protocol.h>
#include <wayland-server.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../backends/GLX/GLXContextManager.h"

// change to toggle debug statements on and off
#define debug debug_off

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
		warning("compositor interface create region called (not yet implemented)");
	}
};

void compositorBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("compositorBindCallback called");
	
	wl_resource * resource = wl_resource_create(client, &wl_compositor_interface, 1, id);
	wl_resource_set_implementation(resource, &compositorInterface, nullptr, nullptr);
};

struct wl_shell_interface shellInterface = {
	// get shell surface
	+[](wl_client * client, wl_resource * resource, uint32_t id, wl_resource * surface) {
		
		debug("shell interface get shell surface called");
		
		WaylandSurface::makeWlShellSurface(client, id, surface);
	}
};

void shellBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("shellBindCallback called");
	
	wl_resource * resource = wl_resource_create(client, &wl_shell_interface, 1, id);
	wl_resource_set_implementation(resource, &shellInterface, nullptr, nullptr);
};

struct wl_pointer_interface pointerInterface = {
	
	// set cursor
	+[](wl_client * client, wl_resource * resource, uint32_t serial, wl_resource * _surface, int32_t hotspot_x, int32_t hotspot_y)
	{
		warning("pointer interface set cursor called (not yet implemented)");
		//surface * surface = wl_resource_get_user_data(_surface);
		//cursor = surface;
	},
	
	// pointer release
	+[](wl_client * client, wl_resource *resource)
	{
		debug("pointer interface pointer release called (not yet implemented)");
	}
};

struct wl_seat_interface seatInterface = {
	
	// get pointer
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("seat interface get pointer called");
		
		wl_resource * pointer = wl_resource_create(client, &wl_pointer_interface, 1, id);
		wl_resource_set_implementation(pointer, &pointerInterface, nullptr, nullptr);
		//get_client(client)->pointer = pointer;
	},
	// get keyboard
	+[](wl_client * client, wl_resource * resource, uint32_t id)
	{
		debug("seat interface get keyboard called (not yet implemented)");
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
		warning("seat interface get touch called (not yet implemented)");
	}
};

void seatBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("seatBindCallback called");
	
	
	//cout << "1" << endl;
	wl_resource * seat = wl_resource_create(client, &wl_seat_interface, 1, id);
	//cout << "2" << endl;
	wl_resource_set_implementation(seat, &seatInterface, nullptr, nullptr);
	//wl_seat_send_capabilities(seat, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
	wl_seat_send_capabilities(seat, 0);
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

}

