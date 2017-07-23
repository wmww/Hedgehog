#include "../h/WaylandServer.h"

#include <wayland-server-protocol.h>
#include <wayland-server.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

// no clue what this means
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

// callbacks
void compositorBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id);

struct WaylandServerImpl: WaylandServerBase
{
	WaylandServerImpl(bool verboseIn)
	{
		verbose = verboseIn;
		
		if (verbose)
			cout << "starting Wayland server" << endl;
		
		// get function pointers
		eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL)eglGetProcAddress("eglBindWaylandDisplayWL");
		eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL)eglGetProcAddress("eglQueryWaylandBufferWL");
		glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
		
		wl_list_init(&clients);
		wl_list_init(&surfaces);
		
		display = wl_display_create();
		
		// automatically find a free socket and connect it to the display
		wl_display_add_socket_auto(display);
		
		// create global objects
		wl_global_create(display, &wl_compositor_interface, 3, nullptr, &compositorBindCallback);
		
		if (verbose)
			cout << "Wayland server setup done" << endl;
	}
	
	~WaylandServerImpl()
	{
		if (verbose)
			cout << "shutting down Wayland server" << endl;
		wl_display_destroy(display);
		instance = nullptr;
	}
	
	static void compositorBindCallback(wl_client * client, void * data, uint32_t version, uint32_t id)
	{
		if (instance == nullptr)
		{
			logError("oh shit, there's no WaylandServerImpl instance!");
			return;
		}
		
		if (instance->verbose)
			cout << "compositorBindCallback called" << endl;
		
		wl_resource * resource = wl_resource_create(client, &wl_compositor_interface, 1, id);
		wl_resource_set_implementation(resource, &compositorInterface, nullptr, nullptr);
	}
	
	static void compositorCreateSurfaceCallback(wl_client * client, wl_resource * resource, uint32_t id)
	{
		if (instance->verbose)
			cout << "compositorCreateSurfaceCallback called" << endl;
	}
	
	static void compositorCreateRegionCallback(wl_client * client, wl_resource * resource, uint32_t id)
	{
		if (instance->verbose)
			cout << "compositorCreateRegionCallback called" << endl;
	}
	
	void fail(string msg)
	{
		error = true;
		logError(msg);
	}
	
	static WaylandServerImpl * instance;
	
	bool error = false;
	bool verbose = false;
	
	// function pointers that need to be retrieved at run time. This is Cs sad, pathetic attempt at duck typing.
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
	PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = nullptr;
	PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = nullptr;
	
	wl_display * display = nullptr;
	
	wl_list clients;
	wl_list surfaces;
	
	static struct wl_compositor_interface compositorInterface;
};

WaylandServerImpl * WaylandServerImpl::instance = nullptr;

struct wl_compositor_interface WaylandServerImpl::compositorInterface
{
	&WaylandServerImpl::compositorCreateSurfaceCallback,
	&WaylandServerImpl::compositorCreateRegionCallback
};

WaylandServer WaylandServerBase::make(bool verbose)
{
	if (WaylandServerImpl::instance != nullptr)
	{
		logError("tried to create multiple WaylandServerImpl instances");
		return WaylandServerImpl::instance->shared_from_this();
	}
	else
	{
		WaylandServerImpl::instance = new WaylandServerImpl(verbose);
		return WaylandServer(WaylandServerImpl::instance);
	}
}

