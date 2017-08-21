#pragma once

#include "../main/Surface2D.h"

class WaylandSurface
{
public:
	static shared_ptr<WaylandSurface> make(wl_client * client, uint32_t id, VerboseToggle verboseToggle);
	static WaylandSurface getFrom(wl_resource * resource);
	
	static void firstInstanceSetup();
	
private:
	struct Impl;
	
	WaylandSurface(VerboseToggle verboseIn);
	WaylandSurface(shared_ptr<Impl> implIn) {impl = implIn;}
	
	// callbacks to be sent to libwayland
	static void deleteSurface(wl_resource * resource);
	static const struct wl_surface_interface surfaceInterface;
	
	// the sole responsibility of this set is to keep the objects alive as long as libwayland has raw pointers to them
	static std::set<shared_ptr<Impl>> surfaceImplSet;
	
	// pointers to functions that need to be retrieved dynamically
	// they will be fetched when the first instance of this class is created
	static PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL;
	static PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
	
	shared_ptr<Impl> impl = nullptr;
};

