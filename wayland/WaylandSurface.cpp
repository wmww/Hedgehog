#include "WaylandSurface.h"
#include "WaylandServer.h"
#include "WaylandObject.h"
#include "../backend/Backend.h"
#include "WlSeat.h"

#include "std_headers/wayland-server-protocol.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

// change to toggle debug statements on and off
#define debug debug_off

struct WaylandSurface::Impl: WaylandObject, InputInterface
{
	struct FrameCallback
	{
		wl_resource * callbackResource = nullptr;
		
		void done()
		{
			assert(callbackResource);
			wl_callback_send_done(callbackResource, timeSinceStartMili());
			wl_resource_destroy(callbackResource);
			callbackResource = nullptr;
		}
	};
	
	// instance data
	V2d dim;
	bool isDamaged = false;
	Texture texture;
	wl_resource * bufferResource = nullptr;
	wl_resource * surfaceResource = nullptr;
	//struct wl_resource * surfaceResource = nullptr;
	
	// interface
	static const struct wl_surface_interface surfaceInterface;
	
	// pointers to functions that need to be retrieved dynamically
	// they will be fetched when the first instance of this class is created
	static PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL;
	static PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
	static vector<FrameCallback> frameCallbacks;
	
	Impl()
	{
		setupIfFirstInstance(this);
	}
	
	~Impl()
	{
		debug("~Impl called");
	}
	
	void pointerMotion(V2d normalizedPos)
	{
		assert(surfaceResource);
		V2d newPos = V2d(normalizedPos.x * dim.x, normalizedPos.y * dim.y);
		//warning(to_string(dim));
		WlSeat::pointerMotion(newPos, surfaceResource);
	}
	
	void pointerLeave()
	{
		assert(surfaceResource);
		WlSeat::pointerLeave(surfaceResource);
	}
	
	void pointerClick(bool down)
	{
		assert(surfaceResource);
		WlSeat::pointerClick(down, surfaceResource);
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
vector<WaylandSurface::Impl::FrameCallback> WaylandSurface::Impl::frameCallbacks;

const struct wl_surface_interface WaylandSurface::Impl::surfaceInterface = {
	// surface destroy
	+[](wl_client * client, wl_resource * resource)
	{
		debug("surface interface surface destroy callback called");
		wlObjDestroy(resource);
	},
	// surface attach
	+[](wl_client * client, wl_resource * resource, wl_resource * buffer, int32_t x, int32_t y)
	{
		debug("surface interface surface attach callback called");
		GET_IMPL_FROM(resource);
		impl->bufferResource = buffer;
	},
	// surface damage
	+[](wl_client * client, wl_resource * resource, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		debug("surface interface surface damage callback called");
		// TODO: OPTIMIZATION: only repaint damaged region
		GET_IMPL_FROM(resource);
		impl->isDamaged = true;
	},
	// surface frame
	+[](wl_client * client, wl_resource * resource, uint32_t callback)
	{
		debug("surface interface surface frame callback called");
		// TODO: OPTIMIZATION: don't call the callback if the window is hidden (this may take some restructuring)
		wl_resource * callbackResource = wl_resource_create(client, &wl_callback_interface, 1, callback);
		FrameCallback callbackStruct {callbackResource};
		frameCallbacks.push_back(callbackStruct);
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
		
		GET_IMPL_FROM(resource);
		
		// get the data we'll need
		struct wl_resource * buffer = impl->bufferResource;
		if (buffer != nullptr)
		{
			if (!impl->isDamaged)
			{
				warning("wl_surface_interface.commit called with new buffer but no damage. is this bad? idk.");
			}
			
			EGLint texture_format;
			Display * display = (Display *)Backend::instance.getXDisplay();
			
			assert(buffer != nullptr);
			
			// make sure this function pointer has been initialized
			assert(Impl::eglQueryWaylandBufferWL);
			
			// query the texture format of the buffer
			bool idkWhatThisVarMeans = Impl::eglQueryWaylandBufferWL(display, buffer, EGL_TEXTURE_FORMAT, &texture_format);
			
			V2i bufferDim;
			
			if (idkWhatThisVarMeans) {
				// I think this is for using EGL to share a buffer directly on the GPU
				// this code path is currently untested
				debug("using EGL for GPU buffer sharing");
				EGLint width, height;
				Impl::eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &width);
				Impl::eglQueryWaylandBufferWL(display, buffer, EGL_WIDTH, &height);
				bufferDim = V2i(width, height);
				EGLAttrib attribs = EGL_NONE;
				EGLImage image = eglCreateImage(display, EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL, buffer, &attribs);
				impl->texture.loadFromEGLImage(image, bufferDim);
				//eglDestroyImage(display, image);
			}
			else {
				// this is for sharing a memory buffer on the CPU
				debug("using SHM for CPU buffer sharing");
				struct wl_shm_buffer * shmBuffer = wl_shm_buffer_get(buffer);
				assert(shmBuffer != nullptr);
				uint32_t width = wl_shm_buffer_get_width(shmBuffer);
				uint32_t height = wl_shm_buffer_get_height(shmBuffer);
				bufferDim = V2i(width, height);
				void * data = wl_shm_buffer_get_data(shmBuffer);
				impl->texture.loadFromData(data, bufferDim);
			}
			wl_buffer_send_release(buffer);
			impl->bufferResource = nullptr;
			impl->dim = V2d(bufferDim.x, bufferDim.y);
		}
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
	implShared->surfaceResource = implShared->wlObjMake(client, id, &wl_surface_interface, 3, &Impl::surfaceInterface);
	impl = implShared;
}

WaylandSurface WaylandSurface::getFrom(wl_resource * resource)
{
	WaylandSurface out;
	out.impl = WaylandObject::get<Impl>(resource);
	return out;
}

void WaylandSurface::runFrameCallbacks()
{
	for (auto i: Impl::frameCallbacks)
	{
		i.done();
	}
	Impl::frameCallbacks.clear();
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

weak_ptr<InputInterface> WaylandSurface::getInputInterface()
{
	return impl;
}
