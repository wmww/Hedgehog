#include "../main/util.h"
#include "../opengl/Texture.h"
#include "WaylandServer.h"
#include "WlSurface.h"
#include "WlSeat.h"
#include "WlDataDeviceManager.h"
#include "WlRegion.h"
#include "WlShellSurface.h"
#include "XdgShellV6Surface.h"
#include "WaylandEGL.h"

#include <wayland-server.h>
#include <wayland-server-protocol.h>
#include "protocols/xdg-shell-unstable-v6.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../backend/Backend.h"

// change to toggle debug statements on and off
#define debug debug_off

const uint wl_compositor_MAX_VERSION = 4;
const uint wl_shell_MAX_VERSION = 1;
const uint zxdg_shell_v6_MAX_VERSION = 1;
const uint wl_output_MAX_VERSION = 3;

namespace WaylandServer
{

// no clue what this means
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

//PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
	
wl_display * display = nullptr;
	
struct wl_event_loop *eventLoop = nullptr;
int eventLoopFileDescriptor = 0;

// callbacks and interfaces
struct wl_compositor_interface compositorImpl = {
	
	.create_surface = +[](wl_client * client, wl_resource * resource, uint32_t id) {
		debug("wl_compositor.create_surface called");
		uint version = Resource(resource).getVersion();
		WlSurface(client, id, version);
	},
	.create_region = +[](wl_client * client, wl_resource * resource, uint32_t id) {
		debug("wl_compositor.create_region called");
		uint version = Resource(resource).getVersion();
		WlRegion(client, id, version);
	}
};

void compositorBind(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug(FUNC + " called");
	ASSERT(version <= wl_compositor_MAX_VERSION);
	Resource compositor;
	compositor.setup(nullptr, client, id, &wl_compositor_interface, version, &compositorImpl);
};

struct wl_shell_interface shellImpl = {
	.get_shell_surface = +[](wl_client * client, wl_resource * resource, uint32_t id, wl_resource * surface) {
		debug("wl_shell.get_shell_surface called");
		WlShellSurface(client, id, wl_resource_get_version(resource), WlSurface::getFrom(Resource(resource)));
	}
};

void shellBind(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug(FUNC + " called");
	ASSERT(version <= wl_shell_MAX_VERSION);
	Resource shell;
	shell.setup(nullptr, client, id, &wl_shell_interface, version, &shellImpl);
};


struct zxdg_shell_v6_interface xdgShellV6Impl {
	.destroy = +[](struct wl_client *client, struct wl_resource *resource) {
		debug("zxdg_shell_v6.destroy called");
		Resource(resource).destroy();
	},
	.create_positioner = +[](struct wl_client *client, struct wl_resource *resource, uint32_t id) {
		warning("zxdg_shell_v6.create_positioner not implemented");
	},
	.get_xdg_surface = +[](struct wl_client *client,struct wl_resource *resource, uint32_t id, struct wl_resource *surface) {
		debug("zxdg_shell_v6.get_xdg_surface called");
		XdgShellV6Surface(client, id, wl_resource_get_version(resource), WlSurface::getFrom(Resource(surface)));
	},
	.pong = +[](struct wl_client *client, struct wl_resource *resource, uint32_t serial)
	{
		debug("zxdg_shell_v6.pong called");
		// this is to test responsiveness, fine to ignore for now
	}
};

void xdgShellV6Bind(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug(FUNC + " called");
	ASSERT(version <= zxdg_shell_v6_MAX_VERSION);
	Resource shell;
	shell.setup(nullptr, client, id, &zxdg_shell_v6_interface, version, &xdgShellV6Impl);
};

void seatBind(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug(FUNC + " called");
	WlSeat(client, id, version);
};

void dataDeviceManagerBind(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug(FUNC + " called");
	WlDataDeviceManager(client, id, version);
}

const struct wl_output_interface outputInterface {
	.release = +[](struct wl_client *client, struct wl_resource *resource)
	{
		debug("wl_output.release called");
		Resource(resource).destroy();
	}
};

void outputBind(wl_client * client, void * data, uint32_t version, uint32_t id)
{
	debug("outputBindCallback called");
	ASSERT(version <= wl_output_MAX_VERSION);
	Resource output;
	output.setup(nullptr, client, id, &wl_output_interface, version, &outputInterface);
	if (version >= WL_OUTPUT_GEOMETRY_SINCE_VERSION)
		wl_output_send_geometry(output.getRaw(), 0, 0, 1600, 1024, WL_OUTPUT_SUBPIXEL_NONE, "", "", WL_OUTPUT_TRANSFORM_NORMAL);
	if (version >= WL_OUTPUT_SCALE_SINCE_VERSION)
		wl_output_send_scale(output.getRaw(), 1);
	if (version >= WL_OUTPUT_MODE_SINCE_VERSION)
		wl_output_send_mode(output.getRaw(), WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED, 800, 800, 60000);
	if (version >= WL_OUTPUT_DONE_SINCE_VERSION)
		wl_output_send_done(output.getRaw());
}

void setup()
{
	debug("starting Wayland server");
	
	// get function pointer
	//glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	
	display = wl_display_create();
	
	// automatically find a free socket and connect it to the display
	wl_display_add_socket_auto(display);
	
	// create global objects
	wl_global_create(display, &wl_compositor_interface, wl_compositor_MAX_VERSION, nullptr, compositorBind);
	wl_global_create(display, &wl_shell_interface, wl_shell_MAX_VERSION, nullptr, shellBind);
	wl_global_create(display, &zxdg_shell_v6_interface, zxdg_shell_v6_MAX_VERSION, nullptr, xdgShellV6Bind);
	wl_global_create(display, &wl_seat_interface, wl_seat_MAX_VERSION, nullptr, seatBind);
	wl_global_create(display, &wl_data_device_manager_interface, wl_data_device_manager_MAX_VERSION, nullptr, dataDeviceManagerBind);
	wl_global_create(display, &wl_output_interface, wl_output_MAX_VERSION, nullptr, outputBind);
	
	wl_display_init_shm(display);
	
	eventLoop = wl_display_get_event_loop(display);
	eventLoopFileDescriptor = wl_event_loop_get_fd(eventLoop);
	
	WaylandEGL::setup(display);
	
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
	WlSurface::runFrameCallbacks();
}

uint32_t lastSerialNum = 1;

uint32_t nextSerialNum()
{
	lastSerialNum++;
	return lastSerialNum;
}

}

