#include "XdgShellV6Surface.h"
#include "WaylandObject.h"

#include <wayland-server-protocol.h>
#include "../protocols/xdg-shell-unstable-v6.h"

// change to toggle debug statements on and off
#define debug debug_on

struct XdgShellV6Surface::Impl: public WaylandObject
{
	// instance data
	WaylandSurface waylandSurface;
	Surface2D surface2D;
	
	// interface
	static const struct zxdg_surface_v6_interface xdgSurfaceV6Interface;
};


const struct zxdg_surface_v6_interface XdgShellV6Surface::Impl::xdgSurfaceV6Interface = {
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

XdgShellV6Surface::XdgShellV6Surface(wl_client * client, uint32_t id, WaylandSurface surface)
{
	debug("creating " + FUNC);
	// important to use a temp var because impl is weak, so it would be immediately deleted
	// in wlSetup, a shared_ptr to the object is saved by WaylandObject, so it is safe to store in a weak_ptr after
	auto implShared = make_shared<Impl>();
	implShared->waylandSurface = surface;
	implShared->surface2D.setTexture(surface.getTexture());
	impl = implShared;
	// sending 1 as the version number isn't a mistake. Idk why its called v6 but you send in 1, maybe always 1 until stable?
	implShared->wlSetup(client, id, &zxdg_surface_v6_interface, 1, &Impl::xdgSurfaceV6Interface);
}
