#include "WaylandSurface.h"
#include "WaylandServer.h"
#include "../backends/GLX/GLXContextManager.h"
#include "WaylandObject.h"

#include <wayland-server-protocol.h>
#include "../protocols/xdg-shell-unstable-v6.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

// change to toggle debug statements on and off
#define debug debug_off

struct WaylandSurface::Impl: public WaylandObject
{
	// instance data
	Surface2D surface2D;
	struct wl_resource * bufferResource = nullptr;
	struct wl_resource * surfaceResource = nullptr;
	
	// callbacks and interfaces to be sent to libwayland
	static void deleteSurface(wl_resource * resource);
	static const struct wl_surface_interface surfaceInterface;
	static const struct zxdg_surface_v6_interface xdgSurfaceV6Interface;
	
	// the sole responsibility of this set is to keep the objects alive as long as libwayland has raw pointers to them
	//static std::unordered_map<Impl *, shared_ptr<Impl>> surfaces;
	
	// pointers to functions that need to be retrieved dynamically
	// they will be fetched when the first instance of this class is created
	static PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL;
	static PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
	
	Impl()
	{
		setupIfFirstInstance(this);
	}
	
	~Impl()
	{
		debug("~Impl called");
	}
	
	static void firstInstanceSetup()
	{
		// function pointers that need to be retrieved at run time. This is Cs sad, pathetic attempt at duck typing.
		eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL)eglGetProcAddress("eglBindWaylandDisplayWL");
		eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL)eglGetProcAddress("eglQueryWaylandBufferWL");
	}
	
	/*
	// given a wayland resource, returns the associated Impl raw pointer
	// this should only be needed in a few places, WaylandSurface::getFrom() is usually safer
	static Impl * getRawPtrFrom(wl_resource * resource)
	{
		assert(string(wl_resource_get_class(resource)) == "wl_surface");
		Impl * implRawPtr = (Impl *)wl_resource_get_user_data(resource);
		return implRawPtr;
	}
	
	// returns the iterator for this objects position in surfaces
	auto getIterInSurfaces()
	{
		auto iter = surfaces.find(this);
		if (iter == surfaces.end())
		{
			warning("getIterInSurfaces called for Impl not in map");
		}
		return iter;
	}
	*/
};

PFNEGLBINDWAYLANDDISPLAYWL WaylandSurface::Impl::eglBindWaylandDisplayWL = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL WaylandSurface::Impl::eglQueryWaylandBufferWL = nullptr;
//std::unordered_map<WaylandSurface::Impl *, shared_ptr<WaylandSurface::Impl>> WaylandSurface::Impl::surfaces;

const struct wl_surface_interface WaylandSurface::Impl::surfaceInterface = {
	// surface destroy
	+[](wl_client * client, wl_resource * resource)
	{
		warning("surface interface surface destroy callback called (not yet implemented)");
	},
	// surface attach
	+[](wl_client * client, wl_resource * resource, wl_resource * buffer, int32_t x, int32_t y)
	{
		debug("surface interface surface attach callback called");
		auto impl = get<Impl>(resource);
		assert(impl);
		impl->bufferResource = buffer;
	},
	// surface damage
	+[](wl_client * client, wl_resource * resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		warning("surface interface surface damage callback called (not yet implemented)");
	},
	// surface frame
	+[](wl_client * client, wl_resource * resource, uint32_t callback)
	{
		warning("surface interface surface frame callback called (not yet implemented)");
		//struct surface *surface = wl_resource_get_user_data (resource);
		//surface->frame_callback = wl_resource_create (client, &wl_callback_interface, 1, callback);
	},
	// surface set opaque region
	+[](wl_client * client, wl_resource * resource, wl_resource * region)
	{
		warning("surface interface surface set opaque region callback called (not yet implemented)");
	},
	// surface set input region
	+[](wl_client * client, wl_resource * resource, wl_resource * region)
	{
		warning("surface interface surface set input region callback called (not yet implemented)");
	},
	// surface commit
	+[](wl_client * client, wl_resource * resource)
	{
		debug("surface interface surface commit callback called");
		
		auto impl = get<Impl>(resource);
		
		assert(impl);
		
		// get the data we'll need
		EGLint texture_format;
		Display * display = GLXContextManagerBase::instance->getDisplay();
		struct wl_resource * buffer = impl->bufferResource;
		
		// make sure this function pointer has been initialized
		assert(Impl::eglQueryWaylandBufferWL);
		
		// query the texture format of the buffer
		bool idkWhatThisVarMeans = Impl::eglQueryWaylandBufferWL(display, buffer, EGL_TEXTURE_FORMAT, &texture_format);
		
		if (idkWhatThisVarMeans) {
			// I think this is for using EGL to share a buffer directly on the GPU
			// this code path is currently untested
			EGLint width, height;
			Impl::eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &width);
			Impl::eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &height);
			EGLAttrib attribs = EGL_NONE;
			EGLImage image = eglCreateImage(display, EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL, buffer, &attribs);
			impl->surface2D.getTexture().loadFromEGLImage(image, V2i(width, height));
			eglDestroyImage(display, image);
		}
		else {
			// this is for sharing a memory buffer on the CPU
			struct wl_shm_buffer * shmBuffer = wl_shm_buffer_get(buffer);
			uint32_t width = wl_shm_buffer_get_width(shmBuffer);
			uint32_t height = wl_shm_buffer_get_height(shmBuffer);
			void * data = wl_shm_buffer_get_data(shmBuffer);
			impl->surface2D.getTexture().loadFromData(data, V2i(width, height));
		}
		wl_buffer_send_release(buffer);
	},
	// surface set buffer transform
	+[](wl_client * client, wl_resource * resource, int32_t transform)
	{
		warning("surface interface surface set buffer transform callback called (not yet implemented)");
	},
	// surface set buffer scale
	+[](wl_client * client, wl_resource * resource, int32_t scale)
	{
		warning("surface interface surface set buffer scale callback called (not yet implemented)");
	},
};

const struct zxdg_surface_v6_interface WaylandSurface::Impl::xdgSurfaceV6Interface = {
	// destroy
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		warning("zxdg_surface_v6_interface::destroy called (not yet implemented)");
	},
	// get_toplevel
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
	{
		warning("zxdg_surface_v6_interface::get_toplevel called (not yet implemented)");
	},
	//get_popup
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *parent, struct wl_resource *positioner)
	{
		warning("zxdg_surface_v6_interface::get_popup called (not yet implemented)");
	},
	/**
	 * set the new window geometry
	 *
	 * The window geometry of a surface is its "visible bounds" from
	 * the user's perspective. Client-side decorations often have
	 * invisible portions like drop-shadows which should be ignored for
	 * the purposes of aligning, placing and constraining windows.
	 *
	 * The window geometry is double buffered, and will be applied at
	 * the time wl_surface.commit of the corresponding wl_surface is
	 * called.
	 *
	 * Once the window geometry of the surface is set, it is not
	 * possible to unset it, and it will remain the same until
	 * set_window_geometry is called again, even if a new subsurface or
	 * buffer is attached.
	 *
	 * If never set, the value is the full bounds of the surface,
	 * including any subsurfaces. This updates dynamically on every
	 * commit. This unset is meant for extremely simple clients.
	 *
	 * The arguments are given in the surface-local coordinate space of
	 * the wl_surface associated with this xdg_surface.
	 *
	 * The width and height must be greater than zero. Setting an
	 * invalid size will raise an error. When applied, the effective
	 * window geometry will be the set window geometry clamped to the
	 * bounding rectangle of the combined geometry of the surface of
	 * the xdg_surface and the associated subsurfaces.
	 */
	// set_window_geometry
	+[](struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		warning("zxdg_surface_v6_interface::set_window_geometry called (not yet implemented)");
	},
	/**
	 * ack a configure event
	 *
	 * When a configure event is received, if a client commits the
	 * surface in response to the configure event, then the client must
	 * make an ack_configure request sometime before the commit
	 * request, passing along the serial of the configure event.
	 *
	 * For instance, for toplevel surfaces the compositor might use
	 * this information to move a surface to the top left only when the
	 * client has drawn itself for the maximized or fullscreen state.
	 *
	 * If the client receives multiple configure events before it can
	 * respond to one, it only has to ack the last configure event.
	 *
	 * A client is not required to commit immediately after sending an
	 * ack_configure request - it may even ack_configure several times
	 * before its next surface commit.
	 *
	 * A client may send multiple ack_configure requests before
	 * committing, but only the last request sent before a commit
	 * indicates which configure event the client really is responding
	 * to.
	 * @param serial the serial from the configure event
	 */
	// ack_configure
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t serial)
	{
		warning("zxdg_surface_v6_interface::ack_configure called (not yet implemented)");
	}
};

void WaylandSurface::Impl::deleteSurface(wl_resource * resource)
{
	warning("delete surface callback called (deprecated)");
	/*
	auto implPtr = Impl::getRawPtrFrom(resource);
	assert(implPtr != nullptr);
	auto iter = implPtr->getIterInSurfaces();
	if (iter != surfaces.end())
	{
		surfaces.erase(iter);
	}
	else
	{
		warning("not deleting wayland surface because it wasn't in the map");
	}
	*/
};

WaylandSurface::WaylandSurface(wl_client * client, uint32_t id)
{
	debug("creating WaylandSurface");
	auto implShared = make_shared<Impl>();
	impl = implShared;
	implShared->wlSetup(client, id, &wl_surface_interface, 3, &Impl::surfaceInterface);
	//implShared->surfaceResource = wl_resource_create(client, &wl_surface_interface, 3, id);
	//wl_resource_set_implementation(implShared->surfaceResource, &Impl::surfaceInterface, &*implShared, Impl::deleteSurface);
	//Impl::surfaces[&*implShared] = implShared;
}

WaylandSurface::WaylandSurface(wl_resource * resource)
{
	impl = WaylandObject::get<Impl>(resource);
}

/*
void WaylandSurface::makeWlShellSurface(wl_client * client, uint32_t id, wl_resource * surface)
{
	Impl * surfaceImplRaw = Impl::getRawPtrFrom(surface);
	wl_resource * shellSurface = wl_resource_create(client, &wl_shell_surface_interface, 1, id);
	wl_resource_set_implementation(shellSurface, &Impl::wlShellSurfaceInterface, surfaceImplRaw, nullptr);
}
*/

void WaylandSurface::makeXdgShellV6Surface(wl_client * client, uint32_t id, wl_resource * surface)
{
	warning("dead func");
	/*
	Impl * surfaceImplRaw = Impl::getRawPtrFrom(surface);
	wl_resource * xdgSurface = wl_resource_create(client, &zxdg_surface_v6_interface, 1, id);
	wl_resource_set_implementation(xdgSurface, &Impl::xdgSurfaceV6Interface, surfaceImplRaw, nullptr);
	*/
}
