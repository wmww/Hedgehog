#include "../h/utils.h"
#include "../h/Texture.h"
#include "../h/WaylandServer.h"

#include <wayland-server-protocol.h>
#include <wayland-server.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../h/GLXContextManager.h"

// no clue what this means
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

struct SurfaceData
{
	struct wl_resource * surface;
	//struct wl_resource *xdg_surface;
	struct wl_resource * buffer;
	Texture texture;
	//struct wl_resource *frame_callback;
	//int x, y;
	//struct texture texture_;
	//struct client *client;
};

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
					assertInstance();
					//struct surface *surface = calloc (1, sizeof(struct surface));
					instance->surfaceInterface = {
						// surface destroy
						+[](wl_client * client, wl_resource * resource)
						{
							message("surface interface surface destroy callback called (not yet implemented)");
						},
						// surface attach
						+[](wl_client * client, wl_resource * resource, wl_resource * buffer, int32_t x, int32_t y)
						{
							message("surface interface surface attach callback called");
							SurfaceData * surface = (SurfaceData *)wl_resource_get_user_data(resource);
							surface->buffer = buffer;
						},
						// surface damage
						+[](wl_client * client, wl_resource * resource, int32_t x, int32_t y, int32_t width, int32_t height)
						{
							message("surface interface surface damage callback called (not yet implemented)");
						},
						// surface frame
						+[](wl_client * client, wl_resource * resource, uint32_t callback)
						{
							message("surface interface surface frame callback called (not yet implemented)");
							//struct surface *surface = wl_resource_get_user_data (resource);
							//surface->frame_callback = wl_resource_create (client, &wl_callback_interface, 1, callback);
						},
						// surface set opaque region
						+[](wl_client * client, wl_resource * resource, wl_resource * region)
						{
							message("surface interface surface set opaque region callback called (not yet implemented)");
						},
						// surface set input region
						+[](wl_client * client, wl_resource * resource, wl_resource * region)
						{
							message("surface interface surface set input region callback called (not yet implemented)");
						},
						// surface commit
						+[](wl_client * client, wl_resource * resource)
						{
							message("surface interface surface commit callback called");
							
							assertInstance();
								
							SurfaceData * surfaceData = (SurfaceData *)wl_resource_get_user_data(resource);
							
							//struct surface *surface = wl_resource_get_user_data (resource);
							EGLint texture_format;
							auto display = GLXContextManagerBase::instance->getDisplay();
							auto buffer = surfaceData->buffer;
							
							// query the texture format of the buffer
							bool idkThisVarMeans = instance->eglQueryWaylandBufferWL(
									display,
									buffer,
									EGL_TEXTURE_FORMAT,
									&texture_format
								);
							
							if (idkThisVarMeans) {
								EGLint width, height;
								instance->eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &width);
								instance->eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &height);
								EGLAttrib attribs = EGL_NONE;
								EGLImage image = eglCreateImage(display, EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL, buffer, &attribs);
								//texture_delete(&surface->texture);
								surfaceData->texture.loadFromEGLImage(image, V2i(width, height));
								//texture_create_from_egl_image (&surface->texture, width, height, image);
								eglDestroyImage (display, image);
							}
							else {
								struct wl_shm_buffer * shmBuffer = wl_shm_buffer_get(buffer);
								uint32_t width = wl_shm_buffer_get_width(shmBuffer);
								uint32_t height = wl_shm_buffer_get_height(shmBuffer);
								void * data = wl_shm_buffer_get_data(shmBuffer);
								//texture_delete(&surface->texture);
								//texture_create(&surface->texture, width, height, data);
								surfaceData->texture.loadFromData(data, V2i(width, height));
							}
							wl_buffer_send_release (buffer);
							//redraw_needed = 1;
						},
						// surface set buffer transform
						+[](wl_client * client, wl_resource * resource, int32_t transform)
						{
							message("surface interface surface set buffer transform callback called (not yet implemented)");
						},
						// surface set buffer scale
						+[](wl_client * client, wl_resource * resource, int32_t scale)
						{
							message("surface interface surface set buffer scale callback called (not yet implemented)");
						},
					};
					
					auto deleteSurface = [](wl_resource * resource)
					{
						message("delete surface callback called (not yet implemented)");
						assertInstance();
						int i = 0;
						for (; i < (int)instance->surfaces.size(); i++)
						{
							if (instance->surfaces[i]->surface == resource)
								break;
						}
						if (i < (int)instance->surfaces.size())
						{
							instance->surfaces.erase(instance->surfaces.begin() + i);
						}
						
						/*
						struct surface *surface = wl_resource_get_user_data (resource);
						wl_list_remove (&surface->link);
						if (surface == active_surface) active_surface = NULL;
						if (surface == pointer_surface) pointer_surface = NULL;
						free (surface);
						redraw_needed = 1;
						*/
					};
					
					struct wl_resource * surface = wl_resource_create(client, &wl_surface_interface, 3, id);
					auto surfaceData = shared_ptr<SurfaceData>(new SurfaceData
						{
							(struct wl_resource *)surface,
							(struct wl_resource *)nullptr,
							Texture(VERBOSE_OFF),
						});
					message("adding surface!");
					instance->surfaces.push_back(surfaceData);
					wl_resource_set_implementation(surface, &instance->surfaceInterface, &*surfaceData, +deleteSurface);
					//surface->client = get_client (client);
					//wl_list_insert (&surfaces, &surface->link);
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
		//message("drawing " + to_string(surfaces.size()) + " textures...");
		for (auto i: surfaces)
		{
			i->texture.draw();
		}
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
	
	struct wl_event_loop *eventLoop = nullptr;
	int eventLoopFileDescriptor = 0;
	
	struct wl_compositor_interface compositorInterface;
	struct wl_shell_interface shellInterface;
	struct wl_seat_interface seatInterface;
	struct wl_pointer_interface pointerInterface;
	
	struct wl_surface_interface surfaceInterface;
	vector<shared_ptr<SurfaceData>> surfaces;
	
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

