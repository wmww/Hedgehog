#include "WaylandSurface.h"
#include "WaylandServer.h"
#include "../backends/GLX/GLXContextManager.h"

#include <wayland-server-protocol.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <unordered_map>

// change to toggle debug statements on and off
#define debug debug_off

struct WaylandSurface::Impl
{
	// instance data
	Surface2D surface2D;
	struct wl_resource * bufferResource = nullptr;
	struct wl_resource * surfaceResource = nullptr;
	
	// callbacks to be sent to libwayland
	static void deleteSurface(wl_resource * resource);
	static const struct wl_surface_interface surfaceInterface;
	static const struct wl_shell_surface_interface shellSurfaceInterface;
	
	// the sole responsibility of this set is to keep the objects alive as long as libwayland has raw pointers to them
	static std::unordered_map<Impl *, shared_ptr<Impl>> surfaces;
	
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
};

PFNEGLBINDWAYLANDDISPLAYWL WaylandSurface::Impl::eglBindWaylandDisplayWL = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL WaylandSurface::Impl::eglQueryWaylandBufferWL = nullptr;
std::unordered_map<WaylandSurface::Impl *, shared_ptr<WaylandSurface::Impl>> WaylandSurface::Impl::surfaces;

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
		auto self = getFrom(resource);
		auto impl = self.impl.lock();
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
		
		auto self = getFrom(resource);
		auto impl = self.impl.lock();
		
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

const struct wl_shell_surface_interface WaylandSurface::Impl::shellSurfaceInterface = {
	// shell surface pong
	+[](struct wl_client *client, wl_resource * resource, uint32_t serial)
	{
		warning("shell surface interface pong callback called (not yet implemented)");
	},
	// shell surface move
	+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial)
	{
		warning("shell surface interface move callback called (not yet implemented)");
	},
	// shell surface resize
	+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial, uint32_t edges)
	{
		warning("shell surface interface resize callback called (not yet implemented)");
	},
	// shell surface set toplevel
	+[](wl_client * client, wl_resource * resource)
	{
		debug("shell surface interface set toplevel callback called");
	},
	// shell surface set transient
	+[](wl_client * client, wl_resource * resource, wl_resource * parent, int32_t x, int32_t y, uint32_t flags)
	{
		warning("shell surface interface set transient callback called (not yet implemented)");
	},
	// shell surface set fullscreen
	+[](wl_client * client, wl_resource * resource, uint32_t method, uint32_t framerate, wl_resource * output)
	{
		warning("shell surface interface set fullscreen callback called (not yet implemented)");
	},
	// shell surface set popup
	+[](wl_client * client, wl_resource * resource, wl_resource * seat, uint32_t serial, wl_resource * parent, int32_t x, int32_t y, uint32_t flags)
	{
		warning("shell surface interface set popup callback called (not yet implemented)");
	},
	// shell surface set maximized
	+[](wl_client * client, wl_resource * resource, wl_resource * output)
	{
		warning("shell surface interface set maximized callback called (not yet implemented)");
	},
	// shell surface set title
	+[](wl_client * client, wl_resource * resource, const char * title)
	{
		warning("shell surface interface set title callback called (not yet implemented)");
	},
	// shell surface set class
	+[](wl_client * client, wl_resource * resource, const char * class_)
	{
		warning("shell surface interface set class callback called (not yet implemented)");
	},
};

void WaylandSurface::Impl::deleteSurface(wl_resource * resource)
{
	debug("delete surface callback called");
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
};

WaylandSurface::WaylandSurface(wl_client * client, uint32_t id)
{
	debug("creating WaylandSurface");
	auto implShared = make_shared<Impl>();
	impl = implShared;
	implShared->surfaceResource = wl_resource_create(client, &wl_surface_interface, 3, id);
	wl_resource_set_implementation(implShared->surfaceResource, &Impl::surfaceInterface, &*implShared, Impl::deleteSurface);
	Impl::surfaces[&*implShared] = implShared;
}

void WaylandSurface::makeWlShellSurface(wl_client * client, uint32_t id, wl_resource * surface)
{
	Impl * implRaw = Impl::getRawPtrFrom(surface);
	wl_resource * shellSurface = wl_resource_create(client, &wl_shell_surface_interface, 1, id);
	wl_resource_set_implementation(shellSurface, &Impl::shellSurfaceInterface, implRaw, nullptr);
}

WaylandSurface WaylandSurface::getFrom(wl_resource * resource)
{
	auto ptr = Impl::getRawPtrFrom(resource);
	assert(ptr != nullptr);
	auto iter = ptr->getIterInSurfaces();
	assert(iter != Impl::surfaces.end());
	return WaylandSurface(iter->second);
}
