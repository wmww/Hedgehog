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
		
		message("starting Wayland server");
		
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
			assertInstance();
			
			message("compositorBindCallback called");
			
			instance->compositorInterface = {
				
				// create surface
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					message("compositorCreateSurfaceCallback called (not yet implemented)");
				},
				
				// create region
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					message("compositorCreateRegionCallback called (not yet implemented)");
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
					
					message("shellInterface called (not yet implemented)");
					//struct wl_resource *shell_surface = wl_resource_create (client, &wl_shell_surface_interface, 1, id);
					//wl_resource_set_implementation (shell_surface, &shell_surface_interface, NULL, NULL);
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
							message("set cursor called (not yet implemented)");
							//surface * surface = wl_resource_get_user_data(_surface);
							//cursor = surface;
						},
						
						// pointer release
						+[](wl_client * client, wl_resource *resource)
						{
							message("pointer release called (not yet implemented)");
						}
					};
					
					wl_resource * pointer = wl_resource_create(client, &wl_pointer_interface, 1, id);
					wl_resource_set_implementation(pointer, &instance->pointerInterface, nullptr, nullptr);
					//get_client(client)->pointer = pointer;
					message("seat interface get pointer over");
				},
				// get keyboard
				+[](wl_client * client, wl_resource * resource, uint32_t id)
				{
					message("seat interface get keyboard called (not yet implemented)");
					//struct wl_resource *keyboard = wl_resource_create (client, &wl_keyboard_interface, 1, id);
					//wl_resource_set_implementation (keyboard, &keyboard_interface, NULL, NULL);
		//			//get_client(client)->keyboard = keyboard;
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
		//if (needsRedraw)
		//{
		//	needsRedraw = false;
		//}
		//else {
		//	
		//}
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
		if (!instance)
		{
			cout << "error: wayland server: oh shit, WaylandServerImpl::instance is null!" << endl;
			exit(1);
		}
	}
	
	static WaylandServerImpl * instance;
	
	static bool verbose;
	
	// function pointers that need to be retrieved at run time. This is Cs sad, pathetic attempt at duck typing.
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
	PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = nullptr;
	PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = nullptr;
	
	wl_display * display = nullptr;
	
	wl_list clients;
	wl_list surfaces;
	
	struct wl_event_loop *eventLoop = nullptr;
	int eventLoopFileDescriptor = 0;
	
	struct wl_compositor_interface compositorInterface;
	struct wl_shell_interface shellInterface;
	struct wl_seat_interface seatInterface;
	struct wl_pointer_interface pointerInterface;
	
	bool needsRedraw = false;
};

WaylandServerImpl * WaylandServerImpl::instance = nullptr;
bool WaylandServerImpl::verbose = false;

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

