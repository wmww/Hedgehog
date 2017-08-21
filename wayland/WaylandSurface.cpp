#include "WaylandServer.h"

const struct wl_surface_interface surfaceInterface = {
	// surface destroy
	+[](wl_client * client, wl_resource * resource)
	{
		WaylandSurface::get(resource).message("surface interface surface destroy callback called (not yet implemented)");
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

void deleteSurface(wl_resource * resource)
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

WaylandSurface::WaylandSurface(wl_client * client, uint32_t id)
{
	verbose = verboseToggle;
	tag = "WaylandSurface";
	struct wl_resource * surfaceResource = wl_resource_create(client, &wl_surface_interface, 3, id);
	wl_resource_set_implementation(surfaceResource, &surfaceInterface, this, deleteSurface);
}
