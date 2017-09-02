#include "WaylandSurface.h"
#include "WaylandServer.h"
#include "../backends/GLX/GLXContextManager.h"
#include "WaylandObject.h"

#include <wayland-server-protocol.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

// change to toggle debug statements on and off
#define debug debug_off

struct WaylandSurface::Impl: public WaylandObject
{
	// instance data
	Texture texture;
	struct wl_resource * bufferResource = nullptr;
	//struct wl_resource * surfaceResource = nullptr;
	
	// interface
	static const struct wl_surface_interface surfaceInterface;
	
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
};

PFNEGLBINDWAYLANDDISPLAYWL WaylandSurface::Impl::eglBindWaylandDisplayWL = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL WaylandSurface::Impl::eglQueryWaylandBufferWL = nullptr;

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
			impl->texture.loadFromEGLImage(image, V2i(width, height));
			eglDestroyImage(display, image);
		}
		else {
			// this is for sharing a memory buffer on the CPU
			struct wl_shm_buffer * shmBuffer = wl_shm_buffer_get(buffer);
			uint32_t width = wl_shm_buffer_get_width(shmBuffer);
			uint32_t height = wl_shm_buffer_get_height(shmBuffer);
			void * data = wl_shm_buffer_get_data(shmBuffer);
			impl->texture.loadFromData(data, V2i(width, height));
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

WaylandSurface::WaylandSurface(wl_client * client, uint32_t id)
{
	debug("creating WaylandSurface");
	// important to use a temp var because impl is weak, so it would be immediately deleted
	// in wlSetup, a shared_ptr to the object is saved by WaylandObject, so it is safe to store in a weak_ptr after
	auto implShared = make_shared<Impl>();
	implShared->wlObjMake(client, id, &wl_surface_interface, 3, &Impl::surfaceInterface);
	impl = implShared;
}

WaylandSurface WaylandSurface::getFrom(wl_resource * resource)
{
	WaylandSurface out;
	out.impl = WaylandObject::get<Impl>(resource);
	return out;
}

/*
wl_resource * WaylandSurface::getSurfaceResource()
{
	GET_IMPL;
	return impl->getResource();
}
*/

Texture WaylandSurface::getTexture()
{
	GET_IMPL;
	return impl->texture;
}
