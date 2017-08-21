#include "WaylandServer.h"

PFNEGLBINDWAYLANDDISPLAYWL WaylandSurface::eglBindWaylandDisplayWL = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL WaylandSurface::eglQueryWaylandBufferWL = nullptr;

struct WaylandSurface::Impl: MessageLogger, public enable_shared_from_this<WaylandSurface>
{
	Surface2D surface2D;
	struct wl_resource * bufferResource = nullptr;
	struct wl_resource * surfaceResource = nullptr;
};

const struct wl_surface_interface WaylandSurface::surfaceInterface = {
	// surface destroy
	+[](wl_client * client, wl_resource * resource)
	{
		getFrom(resource).impl->status("surface interface surface destroy callback called (not yet implemented)");
	},
	// surface attach
	+[](wl_client * client, wl_resource * resource, wl_resource * buffer, int32_t x, int32_t y)
	{
		getFrom(resource).impl->status("surface interface surface attach callback called");
		SurfaceData * surface = (SurfaceData *)wl_resource_get_user_data(resource);
		surface->buffer = buffer;
	},
	// surface damage
	+[](wl_client * client, wl_resource * resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		getFrom(resource).impl->status("surface interface surface damage callback called (not yet implemented)");
	},
	// surface frame
	+[](wl_client * client, wl_resource * resource, uint32_t callback)
	{
		getFrom(resource).impl->status("surface interface surface frame callback called (not yet implemented)");
		//struct surface *surface = wl_resource_get_user_data (resource);
		//surface->frame_callback = wl_resource_create (client, &wl_callback_interface, 1, callback);
	},
	// surface set opaque region
	+[](wl_client * client, wl_resource * resource, wl_resource * region)
	{
		getFrom(resource).impl->status("surface interface surface set opaque region callback called (not yet implemented)");
	},
	// surface set input region
	+[](wl_client * client, wl_resource * resource, wl_resource * region)
	{
		getFrom(resource).impl->status("surface interface surface set input region callback called (not yet implemented)");
	},
	// surface commit
	+[](wl_client * client, wl_resource * resource)
	{
		auto self = getFrom(resource);
		
		self.impl->status("surface interface surface commit callback called");
		
		// get the data we'll need
		EGLint texture_format;
		Display * display = GLXContextManagerBase::instance->getDisplay();
		struct wl_resource buffer = self.impl->buffer;
		
		// make sure this function pointer has been initialized
		assert(WaylandSurface::eglQueryWaylandBufferWL);
		
		// query the texture format of the buffer
		bool idkWhatThisVarMeans = WaylandSurface::eglQueryWaylandBufferWL(display, buffer, EGL_TEXTURE_FORMAT, &texture_format);
		
		if (idkWhatThisVarMeans) {
			// I think this is for using EGL to share a buffer directly on the GPU
			// this code path is currently untested
			EGLint width, height;
			WaylandSurface::eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &width);
			WaylandSurface::eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &height);
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
		getFrom(resource).impl->status("surface interface surface set buffer transform callback called (not yet implemented)");
	},
	// surface set buffer scale
	+[](wl_client * client, wl_resource * resource, int32_t scale)
	{
		getFrom(resource).impl->status("surface interface surface set buffer scale callback called (not yet implemented)");
	},
};

void WaylandSurface::deleteSurface(wl_resource * resource)
{
	auto self = getFrom(resource);
	self.impl->message("delete surface callback called");
	assert(surfaceImplSet.find(self.impl) != surfaceImplSet.end());
	surfaceImplSet.erase(self.impl);
};

void WaylandSurface::firstInstanceSetup()
{
	eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL)eglGetProcAddress("eglBindWaylandDisplayWL");
	eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL)eglGetProcAddress("eglQueryWaylandBufferWL");
}

WaylandSurface::WaylandSurface(VerboseToggle verboseToggle)
{
	setupIfFirstInstance(this);
	impl = shared_ptr<Impl>(new Impl);
	impl->verbose = verboseToggle;
	impl->tag = "WaylandSurface";
}

shared_ptr<WaylandSurface> WaylandSurface::make(wl_client * client, uint32_t id, VerboseToggle verboseToggle);
{
	obj = WaylandSurface(verboseToggle);
	obj.impl->surfaceResource = wl_resource_create(client, &wl_surface_interface, 3, id);
	wl_resource_set_implementation(obj.impl->surfaceResource, &surfaceInterface, &*obj.impl, deleteSurface);
	surfaceImplSet.insert(obj.impl);
	return obj;
}

static WaylandSurface WaylandSurface::getFrom(wl_resource * resource)
{
	assert(wl_resource_get_class(resource) == "wl_surface_interface");
	return WaylandSurface(((Impl *)wl_resource_get_user_data(resource))->shared_from_this());
}
