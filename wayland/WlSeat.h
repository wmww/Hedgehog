#pragma once

#include "WaylandServer.h"
#include "Resource.h"

const uint wl_seat_MAX_VERSION = 6;

class WlSeat
{
public:
	WlSeat() {}
	WlSeat(wl_client * client, uint32_t id, uint version);
	static void pointerMotion(V2d position, Resource surface);
	static void pointerLeave(Resource surface); // call this before pointerMove on the new surface
	static void pointerClick(uint button, bool down, Resource surface);
	static void keyPress(uint key, bool down, Resource surface);
	static WlSeat getFromClient(wl_client * client);
	
private:
	struct Impl;
	weak_ptr<Impl> impl;
	static shared_ptr<Impl> getImplFromSurface(Resource surface);
};

