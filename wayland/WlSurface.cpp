#include "WlSurface.h"
#include "WaylandServer.h"
#include "Resource.h"
#include "../backend/Backend.h"
#include "WlSeat.h"
#include "WaylandEGL.h"

#include <wayland-server-protocol.h>

// change to toggle debug statements on and off
#define debug debug_off

struct WlSurface::Impl: Resource::Data, InputInterface
{
	// instance data
	V2d dim;
	bool isDamaged = false;
	Texture texture;
	wl_resource * bufferResourceRaw;
	Resource surfaceResource;
	//struct wl_resource * surfaceResource = nullptr;
	
	// interface
	static const struct wl_surface_interface surfaceInterface;
	
	static vector<Resource> frameCallbacks;
	
	void pointerMotion(V2d normalizedPos)
	{
		ASSERT_ELSE(surfaceResource.isValid(), return);
		V2d newPos = V2d(normalizedPos.x * dim.x, (1 - normalizedPos.y) * dim.y);
		WlSeat::pointerMotion(newPos, surfaceResource);
	}
	
	void pointerLeave()
	{
		ASSERT_ELSE(surfaceResource.isValid(), return);
		WlSeat::pointerLeave(surfaceResource);
	}
	
	void pointerClick(uint button, bool down)
	{
		ASSERT_ELSE(surfaceResource.isValid(), return);
		WlSeat::pointerClick(button, down, surfaceResource);
	}
	
	void keyPress(uint key, bool down)
	{
		ASSERT_ELSE(surfaceResource.isValid(), return);
		WlSeat::keyPress(key, down, surfaceResource);
	}
};

vector<Resource> WlSurface::Impl::frameCallbacks;

const struct wl_surface_interface WlSurface::Impl::surfaceInterface = {
	.destroy = +[](wl_client * client, wl_resource * resource) {
		debug("wl_surface.destroy called");
		Resource(resource).destroy();
	},
	.attach = +[](wl_client * client, wl_resource * resource, wl_resource * buffer, int32_t x, int32_t y) {
		debug("wl_surface.attach called");
		IMPL_FROM(resource);
		impl->bufferResourceRaw = buffer;
	},
	.damage = +[](wl_client * client, wl_resource * resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		debug("wl_surface.damage called");
		// TODO: OPTIMIZATION: only repaint damaged region
		IMPL_FROM(resource);
		impl->isDamaged = true;
	},
	.frame = +[](wl_client * client, wl_resource * resource, uint32_t callback) {
		debug("wl_surface.frame called");
		// TODO: OPTIMIZATION: don't call the callback if the window is hidden (this may take some restructuring)
		Resource callbackResource;
		callbackResource.setup(nullptr, client, callback, &wl_callback_interface, 1, nullptr);
		frameCallbacks.push_back(callbackResource);
	},
	.set_opaque_region = +[](wl_client * client, wl_resource * resource, wl_resource * region) {
		debug("wl_surface.set_opaque_region called");
		// this is just for optimizing the redrawing of things behind this surface, fine to ignore for now
	},
	.set_input_region = +[](wl_client * client, wl_resource * resource, wl_resource * region) {
		warning("wl_surface.set_input_region not implemented");
	},
	.commit = +[](wl_client * client, wl_resource * resource) {
		debug("wl_surface.commit called");
		
		IMPL_FROM(resource);
		
		// get the data we'll need
		struct wl_resource * buffer = impl->bufferResourceRaw;
		if (buffer != nullptr && impl->isDamaged)
		{
			//EGLint texture_format;
			ASSERT_ELSE(Backend::instance, return);
			//Display * display = (Display *)Backend::instance->getXDisplay();
			//ASSERT_ELSE(display, return);
			
			// make sure this function pointer has been initialized
			//assert(Impl::eglQueryWaylandBufferWL);
			
			// query the texture format of the buffer
			//bool idkWhatThisVarMeans = Impl::eglQueryWaylandBufferWL(display, buffer, EGL_TEXTURE_FORMAT, &texture_format);
			
			V2i bufferDim;
			
			/*if (idkWhatThisVarMeans) {
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
			else*/
			// this is for sharing a memory buffer on the CPU
			debug("using SHM for CPU buffer sharing");
			struct wl_shm_buffer * shmBuffer = wl_shm_buffer_get(buffer);
			if (shmBuffer)
			{
				uint32_t width = wl_shm_buffer_get_width(shmBuffer);
				uint32_t height = wl_shm_buffer_get_height(shmBuffer);
				bufferDim = V2i(width, height);
				void * data = wl_shm_buffer_get_data(shmBuffer);
				impl->texture.loadFromData(data, bufferDim);
			}
			else
			{
				WaylandEGL::loadIntoTexture(buffer, impl->texture);
			}
			wl_buffer_send_release(buffer);
			impl->bufferResourceRaw = nullptr;
			impl->dim = V2d(bufferDim.x, bufferDim.y);
		}
	},
	.set_buffer_transform = +[](wl_client * client, wl_resource * resource, int32_t transform) {
		warning("wl_surface.set_buffer_transform not implemented");
	},
	.set_buffer_scale = +[](wl_client * client, wl_resource * resource, int32_t scale) {
		debug("wl_surface.set_buffer_scale called");
		if (scale != 1)
		{
			warning("scale is " + to_string(scale) + " which is not 1 and thus shouldn't be ignored");
		}
	},
	.damage_buffer = +[](struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		warning("wl_surface.damage_buffer not implemented");
	},
};

WlSurface::WlSurface(wl_client * client, uint32_t id, uint version)
{
	debug("creating WlSurface");
	auto impl = make_shared<Impl>();
	this->impl = impl;
	impl->surfaceResource.setup(impl, client, id, &wl_surface_interface, version, &Impl::surfaceInterface);
}

WlSurface WlSurface::getFrom(Resource resource)
{
	ASSERT(resource.isValid());
	WlSurface out;
	out.impl = resource.get<Impl>();
	return out;
}

void WlSurface::runFrameCallbacks()
{
	for (auto i: Impl::frameCallbacks)
	{
		ASSERT_THEN(i.isValid())
		{
			if (i.getVersion() >= WL_CALLBACK_DONE_SINCE_VERSION)
				wl_callback_send_done(i.getRaw(), timeSinceStartMili());
			i.destroy();
		}
	}
	Impl::frameCallbacks.clear();
}

Texture WlSurface::getTexture()
{
	IMPL_ELSE(return Texture());
	if (impl->texture.isNull())
		impl->texture.setupEmpty();
	return impl->texture;
}

weak_ptr<InputInterface> WlSurface::getInputInterface()
{
	return impl;
}
