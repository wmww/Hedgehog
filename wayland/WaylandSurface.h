#pragma once

#include "../main/util.h"
#include "WaylandServer.h"
#include "../opengl/Texture.h"
#include "../scene/InputInterface.h"
#include "Resource.h"

class WaylandSurface
{
public:
	WaylandSurface() {}
	WaylandSurface(wl_client * client, uint32_t id);
	static WaylandSurface getFrom(Resource resource); // to use this the resource must have been created by this class
	
	static void runFrameCallbacks();
	
	weak_ptr<InputInterface> getInputInterface();
	Texture getTexture();
	
private:
	struct Impl;
	weak_ptr<Impl> impl; // can be null
};

