// disables debug statements, must be before includes
#define NO_DEBUG

#include "WaylandSurface.h"
#include "WaylandServer.h"
#include "../backends/GLX/GLXContextManager.h"

#include <wayland-server-protocol.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <set>

struct WaylandSurface::Impl: public enable_shared_from_this<Impl>
{
	// instance data
	Surface2D surface2D;
	struct wl_resource * bufferResource = nullptr;
	struct wl_resource * surfaceResource = nullptr;
	
	// callbacks to be sent to libwayland
	static void deleteSurface(wl_resource * resource);
	static const struct wl_surface_interface surfaceInterface;
	
	// the sole responsibility of this set is to keep the objects alive as long as libwayland has raw pointers to them
	static std::set<shared_ptr<Impl>> surfaceImplSet;
	
	// pointers to functions that need to be retrieved dynamically
	// they will be fetched when the first instance of this class is created
	static PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL;
	static PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
	
	Impl()
	{
		setupIfFirstInstance(this);
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
std::set<shared_ptr<WaylandSurface::Impl>> WaylandSurface::Impl::surfaceImplSet;

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
		self.impl->bufferResource = buffer;
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
		
		// get the data we'll need
		EGLint texture_format;
		Display * display = GLXContextManagerBase::instance->getDisplay();
		struct wl_resource * buffer = self.impl->bufferResource;
		
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
			self.impl->surface2D.getTexture().loadFromEGLImage(image, V2i(width, height));
			eglDestroyImage(display, image);
		}
		else {
			// this is for sharing a memory buffer on the CPU
			struct wl_shm_buffer * shmBuffer = wl_shm_buffer_get(buffer);
			uint32_t width = wl_shm_buffer_get_width(shmBuffer);
			uint32_t height = wl_shm_buffer_get_height(shmBuffer);
			void * data = wl_shm_buffer_get_data(shmBuffer);
			self.impl->surface2D.getTexture().loadFromData(data, V2i(width, height));
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

void WaylandSurface::Impl::deleteSurface(wl_resource * resource)
{
	debug("delete surface callback called");
	auto self = getFrom(resource);
	auto iter = Impl::surfaceImplSet.find(self.impl);
	if (iter == Impl::surfaceImplSet.end())
	{
		warning("deleteSurface called but linked WaylandSurface::Impl is not in surfaceImplSet");
	}
	else
	{
		surfaceImplSet.erase(iter);
	}
};

WaylandSurface::WaylandSurface(wl_client * client, uint32_t id)
{
	impl = shared_ptr<Impl>(new Impl);
	impl->surfaceResource = wl_resource_create(client, &wl_surface_interface, 3, id);
	wl_resource_set_implementation(impl->surfaceResource, &Impl::surfaceInterface, &*impl, Impl::deleteSurface);
	Impl::surfaceImplSet.insert(impl);
}

WaylandSurface WaylandSurface::getFrom(wl_resource * resource)
{
	assert(string(wl_resource_get_class(resource)) == string("wl_surface"));
	Impl * impl = (Impl *)wl_resource_get_user_data(resource);
	return WaylandSurface(impl->shared_from_this());
}
