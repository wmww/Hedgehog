#include "XdgShellV6Surface.h"
#include "WaylandObject.h"
#include "../scene/WindowInterface.h"

#include "std_headers/wayland-server-protocol.h"
#include "protocols/xdg-shell-unstable-v6.h"

// change to toggle debug statements on and off
#define debug debug_on

struct XdgShellV6Surface::Impl: WaylandObject, WindowInterface
{
	// instance data
	WaylandSurface waylandSurface;
	//Surface2D surface2D;
	wl_resource * xdgSurfaceResource = nullptr;
	wl_resource * xdgToplevelResource = nullptr;
	
	void pointerMotion(V2d newPos)
	{
		warning(FUNC + " not yet implemented");
	}
	
	void setSize(V2i size)
	{
		warning(FUNC + " not yet implemented");
	}
	
	// interfaces
	static const struct zxdg_surface_v6_interface xdgSurfaceV6Interface;
	static const struct zxdg_toplevel_v6_interface xdgToplevelV6Interface;
};

const struct zxdg_surface_v6_interface XdgShellV6Surface::Impl::xdgSurfaceV6Interface = {
	// destroy
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		debug("zxdg_surface_v6_interface::destroy called");
		wlObjDestroy(resource);
	},
	// get_toplevel
	+[](struct wl_client *client, struct wl_resource *resource, uint32_t id)
	{
		debug("zxdg_surface_v6_interface::get_toplevel called");
		GET_IMPL_FROM(resource);
		impl->xdgToplevelResource = impl->wlObjMake(client, id, &zxdg_toplevel_v6_interface, 1, &xdgToplevelV6Interface);
		
		wl_array states;
		wl_array_init(&states);
		*((zxdg_toplevel_v6_state*)wl_array_add(&states, sizeof(zxdg_toplevel_v6_state))) = ZXDG_TOPLEVEL_V6_STATE_ACTIVATED;
		*((zxdg_toplevel_v6_state*)wl_array_add(&states, sizeof(zxdg_toplevel_v6_state))) = ZXDG_TOPLEVEL_V6_STATE_ACTIVATED;
		zxdg_toplevel_v6_send_configure(impl->xdgToplevelResource, 0, 0, &states);
		wl_array_release(&states);
		zxdg_surface_v6_send_configure(impl->xdgSurfaceResource, WaylandServer::nextSerialNum());
		
		debug("zxdg_surface_v6_interface::get_toplevel done");
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

const struct zxdg_toplevel_v6_interface XdgShellV6Surface::Impl::xdgToplevelV6Interface = {
	// destroy
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		debug("zxdg_toplevel_v6_interface::destroy called");
		wlObjDestroy(resource);
	},
	// set_parent
	+[](struct wl_client *client, struct wl_resource *resource, struct wl_resource *parent)
	{
		warning("zxdg_toplevel_v6_interface::set_parent called (not yet implemented)");
		// this is for popups n shit that are separate windows but are always on top of their parent
	},
	// set_title
	+[](struct wl_client *client, struct wl_resource *resource,const char *title)
	{
		debug("zxdg_toplevel_v6_interface::set_title called, ignoring");
		// safe to ignore until we need window titles
	},
	// set_app_id
	+[](struct wl_client *client, struct wl_resource *resource, const char *app_id)
	{
		debug("zxdg_toplevel_v6_interface::set_app_id called, ignoring");
		// fine to ignore this for now
		// may want to save this somewhere but its not really useful
		// unless you want to group windows of the same app or something
	},
	// show_window_menu
	+[](struct wl_client *client, struct wl_resource *resource, struct wl_resource *seat, uint32_t serial, int32_t x, int32_t y)
	{
		debug("zxdg_toplevel_v6_interface::show_window_menu called, ignoring");
		// this dumb ass message is for the compositor to pop up a window menu when client side decorations are right clicked on
		// fuck client side decorations, am I right?
	},
	// move
	+[](struct wl_client *client, struct wl_resource *resource, struct wl_resource *seat, uint32_t serial)
	{
		warning("zxdg_toplevel_v6_interface::move called (not yet implemented)");
		// this is that weird interactive move thing
		// best not to fuck with it for now
	},
	// resize
	+[](struct wl_client *client, struct wl_resource *resource, struct wl_resource *seat, uint32_t serial, uint32_t edges)
	{
		warning("zxdg_toplevel_v6_interface::resize called (not yet implemented)");
		// another interactive starter, same advice
	},
	// set_max_size
	+[](struct wl_client *client, struct wl_resource *resource, int32_t width, int32_t height)
	{
		debug("zxdg_toplevel_v6_interface::set_max_size called, ignoring");
		// it is documented that this can be safely ignored by the compositor
		// screw you client. I'll make you whatever size I damn well please and there is not a god damn thing you can do about it
	},
	// set_min_size
	+[](struct wl_client *client, struct wl_resource *resource, int32_t width, int32_t height)
	{
		debug("zxdg_toplevel_v6_interface::set_min_size called, ignoring");
		// this can also be safely ignored
	},
	// set_maximized
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		warning("zxdg_toplevel_v6_interface::set_maximized called (not yet implemented)");
		// proper implementation must put out configure event or some shit
	},
	// unset_maximized
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		warning("zxdg_toplevel_v6_interface::unset_maximized called (not yet implemented)");
		// also needs a configure event
	},
	// set_fullscreen
	+[](struct wl_client *client, struct wl_resource *resource, struct wl_resource *output)
	{
		warning("zxdg_toplevel_v6_interface::set_fullscreen called (not yet implemented)");
		// client wants to be fullscreen. what more do you want to know?
	},
	// unset_fullscreen
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		warning("zxdg_toplevel_v6_interface::unset_fullscreen called (not yet implemented)");
	},
	// set_minimized
	+[](struct wl_client *client, struct wl_resource *resource)
	{
		debug("zxdg_toplevel_v6_interface::set_minimized called, ignoring");
		// pretty sure this is safe to ignore
	},
};

XdgShellV6Surface::XdgShellV6Surface(wl_client * client, uint32_t id, WaylandSurface surface)
{
	debug("creating XdgShellV6Surface");
	// important to use a temp var because impl is weak, so it would be immediately deleted
	// in wlSetup, a shared_ptr to the object is saved by WaylandObject, so it is safe to store in a weak_ptr after
	auto implShared = make_shared<Impl>();
	//implShared->surface2D.setup();
	//implShared->surface2D.setTexture(surface.getTexture());
	implShared->texture = surface.getTexture();
	Scene::instance.addWindow(implShared);
	// sending 1 as the version number isn't a mistake. Idk why its called v6 but you send in 1, maybe always 1 until stable?
	implShared->xdgSurfaceResource = implShared->wlObjMake(client, id, &zxdg_surface_v6_interface, 1, &Impl::xdgSurfaceV6Interface);
	impl = implShared;
}
