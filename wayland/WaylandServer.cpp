#include "../main/utils.h"
#include "../opengl/Texture.h"
#include "WaylandServer.h"
#include "WaylandSurface.h"

#include <wayland-server-protocol.h>
#include <wayland-server.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../backends/GLX/GLXContextManager.h"

// no clue what this means
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

struct WaylandServerImpl: WaylandServerBase
{
	WaylandServerImpl(VerboseToggle verboseIn)
	{
		verbose = verboseIn;
		
		message("starting Wayland server");
		
		// get function pointers
		glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
		
		display = wl_display_create();
		
		// automatically find a free socket and connect it to the display
		wl_display_add_socket_auto(display);
		
		auto compositorBindCallback = +[](wl_client * client, void * data, uint32_t version, uint32_t id)
		{
			assertInstance();
			
			message("compositorBindCallback called");
			
			instance->compositorInterface = {
				
				// create surface
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					message("compositor interface create surface callback called");
					WaylandSurface(client, id);
				},
				
				// create region
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					message("compositor interface create region called (not yet implemented)");
				}
			};
			
			wl_resource * resource = wl_resource_create(client, &wl_compositor_interface, 1, id);
			wl_resource_set_implementation(resource, &instance->compositorInterface, nullptr, nullptr);
		};
		
		auto shellBindCallback = +[](wl_client * client, void * data, uint32_t version, uint32_t id)
		{
			assertInstance();
			
			message("shellBindCallback called");
			
			instance->shellInterface = {
				// get shell surface
				+[](wl_client * client, wl_resource * resource, uint32_t id, wl_resource * surface) {
					
					message("shell interface get shell surface called");
					
					struct wl_shell_surface_interface shellSurfaceInterface = {
						
						// shell surface pong
						+[](struct wl_client *client, wl_resource * resource, uint32_t serial)
						{
							message("shell surface interface pong callback called (not yet implemented)");
						},
						// shell surface move
						+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial)
						{
							message("shell surface interface move callback called (not yet implemented)");
						},
						// shell surface resize
						+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial, uint32_t edges)
						{
							message("shell surface interface resize callback called (not yet implemented)");
						},
						// shell surface set toplevel
						+[](wl_client * client, wl_resource * resource)
						{
							message("shell surface interface set toplevel callback called");
						},
						// shell surface set transient
						+[](wl_client * client, wl_resource * resource, wl_resource * parent, int32_t x, int32_t y, uint32_t flags)
						{
							message("shell surface interface set transient callback called (not yet implemented)");
						},
						// shell surface set fullscreen
						+[](wl_client * client, wl_resource * resource, uint32_t method, uint32_t framerate, wl_resource * output)
						{
							message("shell surface interface set fullscreen callback called (not yet implemented)");
						},
						// shell surface set popup
						+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial, wl_resource * parent, int32_t x, int32_t y, uint32_t flags)
						{
							message("shell surface interface set popup callback called (not yet implemented)");
						},
						// shell surface set maximized
						+[](wl_client * client, wl_resource * resource, wl_resource * output)
						{
							message("shell surface interface set maximized callback called (not yet implemented)");
						},
						// shell surface set title
						+[](wl_client * client, wl_resource * resource, const char * title)
						{
							message("shell surface interface set title callback called (not yet implemented)");
						},
						// shell surface set class
						+[](wl_client * client, wl_resource * resource, const char * class_)
						{
							message("shell surface interface set class callback called (not yet implemented)");
						},
					};
					
					wl_resource * shellSurface = wl_resource_create(client, &wl_shell_surface_interface, 1, id);
					wl_resource_set_implementation(shellSurface, &shellSurfaceInterface, nullptr, nullptr);
				}
			};
			
			wl_resource * resource = wl_resource_create(client, &wl_shell_interface, 1, id);
			wl_resource_set_implementation(resource, &instance->shellInterface, nullptr, nullptr);
		};
		
		auto seatBindCallback = +[](wl_client * client, void * data, uint32_t version, uint32_t id)
		{
			message("seatBindCallback called");
			
			assertInstance();
			
			instance->seatInterface = {
				// get pointer
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					message("seat interface get pointer called");
					
					assertInstance();
					
					instance->pointerInterface = {
						// set cursor
						+[](wl_client * client, wl_resource * resource, uint32_t serial, wl_resource * _surface, int32_t hotspot_x,
						int32_t hotspot_y)
						{
							message("pointer interface set cursor called (not yet implemented)");
							//surface * surface = wl_resource_get_user_data(_surface);
							//cursor = surface;
						},
						
						// pointer release
						+[](wl_client * client, wl_resource *resource)
						{
							message("pointer interface pointer release called (not yet implemented)");
						}
					};
					
					wl_resource * pointer = wl_resource_create(client, &wl_pointer_interface, 1, id);
					wl_resource_set_implementation(pointer, &instance->pointerInterface, nullptr, nullptr);
					//get_client(client)->pointer = pointer;
				},
				// get keyboard
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					message("seat interface get keyboard called (not yet implemented)");
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
					message("seat interface get touch called (not yet implemented)");
				}
			};
			
			//cout << "1" << endl;
			wl_resource * seat = wl_resource_create(client, &wl_seat_interface, 1, id);
			//cout << "2" << endl;
			wl_resource_set_implementation(seat, &instance->seatInterface, nullptr, nullptr);
			//wl_seat_send_capabilities(seat, WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD);
			wl_seat_send_capabilities(seat, 0);
		};
		
		// create global objects
		wl_global_create(display, &wl_compositor_interface, 3, nullptr, compositorBindCallback);
		wl_global_create(display, &wl_shell_interface, 1, nullptr, shellBindCallback);
		wl_global_create(display, &wl_seat_interface, 1, nullptr, seatBindCallback);
		
		wl_display_init_shm(display);
		
		eventLoop = wl_display_get_event_loop(display);
		eventLoopFileDescriptor = wl_event_loop_get_fd(eventLoop);
		
		message("Wayland server setup done");
	}
	
	~WaylandServerImpl()
	{
		message("shutting down Wayland server");
		wl_display_destroy(display);
		instance = nullptr;
	}
	
	void init();
	
	void iteration()
	{
		wl_event_loop_dispatch(eventLoop, 0);
		wl_display_flush_clients(display);
	}
	
	inline static void message(string msg)
	{
		if (verbose)
		{
			cout << "wayland server: " << msg << endl;
		}
	}
	
	inline static void assertInstance()
	{
		assert(instance != nullptr);
	}
	
	static WaylandServerImpl * instance;
	
	static VerboseToggle verbose;
	
	// function pointers that need to be retrieved at run time. This is Cs sad, pathetic attempt at duck typing.
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
	
	wl_display * display = nullptr;
	
	struct wl_event_loop *eventLoop = nullptr;
	int eventLoopFileDescriptor = 0;
	
	struct wl_compositor_interface compositorInterface;
	struct wl_shell_interface shellInterface;
	struct wl_seat_interface seatInterface;
	struct wl_pointer_interface pointerInterface;
};

WaylandServerImpl * WaylandServerImpl::instance = nullptr;
VerboseToggle WaylandServerImpl::verbose = VERBOSE_OFF;

WaylandServer WaylandServerBase::make(VerboseToggle verbose)
{
	if (WaylandServerImpl::instance != nullptr)
	{
		warning("tried to create multiple WaylandServerImpl instances");
		return WaylandServerImpl::instance->shared_from_this();
	}
	else
	{
		WaylandServerImpl::instance = new WaylandServerImpl(verbose);
		return WaylandServer(WaylandServerImpl::instance);
	}
}

