#pragma once

#include "../main/util.h"
#include "WaylandServer.h"
#include "../opengl/Texture.h"
#include "../scene/InputInterface.h"
#include "Resource.h"

class WlSurface
{
public:
	WlSurface() {}
	WlSurface(wl_client * client, uint32_t id, uint version);
	static WlSurface getFrom(Resource resource); // to use this the resource must have been created by this class
	
	static void runFrameCallbacks();
	
	weak_ptr<InputInterface> getInputInterface();
	Texture getTexture();
	
private:
	struct Impl;
	weak_ptr<Impl> impl; // can be null
};

