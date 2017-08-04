#include "../h/WaylandServer.h"

#include <wayland-server-protocol.h>
#include <wayland-server.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

// no clue what this means
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

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
		
		auto compositorBindCallback = +[](wl_client * client, void * data, uint32_t version, uint32_t id)
		{
			if (instance == nullptr)
			{
				logError("oh shit, there's no WaylandServerImpl instance!");
				return;
			}
			
			if (instance->verbose)
				cout << "compositorBindCallback called" << endl;
			
			struct wl_compositor_interface compositorInterface {
				
				// create surface
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					cout << "compositorCreateSurfaceCallback called" << endl;
				},
				
				// create region
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					cout << "compositorCreateRegionCallback called" << endl;
				}
			};
			
			wl_resource * resource = wl_resource_create(client, &wl_compositor_interface, 1, id);
			wl_resource_set_implementation(resource, &compositorInterface, nullptr, nullptr);
		};
		
		auto shellBindCallback = +[](wl_client * client, void * data, uint32_t version, uint32_t id)
		{
			if (instance == nullptr)
			{
				logError("oh shit, there's no WaylandServerImpl instance!");
				return;
			}
			
			if (instance->verbose)
				cout << "shellBindCallback called" << endl;
			
			struct wl_shell_interface shellInterface {
				// get shell surface
				+[](wl_client * client, wl_resource * resource, uint32_t id, wl_resource * surface) {
					
					cout << "shellInterface called" << endl;
					//struct wl_resource *shell_surface = wl_resource_create (client, &wl_shell_surface_interface, 1, id);
					//wl_resource_set_implementation (shell_surface, &shell_surface_interface, NULL, NULL);
				}
			};
			
			wl_resource * resource = wl_resource_create(client, &wl_shell_interface, 1, id);
			wl_resource_set_implementation(resource, &shellInterface, nullptr, nullptr);
		};
		
		auto seatBindCallback = +[](wl_client * client, void * data, uint32_t version, uint32_t id)
		{
			if (instance == nullptr)
			{
				logError("oh shit, there's no WaylandServerImpl instance!");
				return;
			}
			
			if (instance->verbose)
				cout << "seatBindCallback called" << endl;
			
			struct wl_seat_interface seatInterface
			{
				// get pointer
				+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
				{
					cout << "seat interface get pointer called" << endl;
					/*
					struct wl_resource *pointer = wl_resource_create (client, &wl_pointer_interface, 1, id);
					wl_resource_set_implementation (pointer, &pointer_interface, NULL, NULL);
					get_client(client)->pointer = pointer;
					*/
				},
				// get keyboard
				+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
				{
					cout << "seat interface get keyboard called" << endl;
					/*struct wl_resource *keyboard = wl_resource_create (client, &wl_keyboard_interface, 1, id);
					wl_resource_set_implementation (keyboard, &keyboard_interface, NULL, NULL);
					get_client(client)->keyboard = keyboard;
					int fd, size;
					backend_get_keymap (&fd, &size);
					wl_keyboard_send_keymap (keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);
					//close (fd);
					*/
				},
				// get touch
				+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
				{
					cout << "seat interface get touch called" << endl;
				}
			};
			
			wl_resource * seat = wl_resource_create(client, &wl_seat_interface, 1, id);
			wl_resource_set_implementation(seat, &seatInterface, nullptr, nullptr);
			wl_seat_send_capabilities(seat, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
		};
		
		// create global objects
		wl_global_create(display, &wl_compositor_interface, 3, nullptr, compositorBindCallback);
		wl_global_create(display, &wl_shell_interface, 1, nullptr, shellBindCallback);
		wl_global_create(display, &wl_seat_interface, 1, nullptr, seatBindCallback);
		
		wl_display_init_shm (display);
		
		eventLoop = wl_display_get_event_loop(display);
		eventLoopFileDescriptor = wl_event_loop_get_fd(eventLoop);
		
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
	
	void iteration()
	{
		wl_event_loop_dispatch(eventLoop, 0);
		wl_display_flush_clients(display);
		if (needsRedraw)
		{
			needsRedraw = false;
		}
		else {
			
		}
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
	
	struct wl_event_loop *eventLoop = nullptr;
	int eventLoopFileDescriptor = 0;
	
	bool needsRedraw = false;
};

WaylandServerImpl * WaylandServerImpl::instance = nullptr;

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

