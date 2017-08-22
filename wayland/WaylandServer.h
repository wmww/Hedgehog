#pragma once

#include "../main/utils.h"

struct WaylandServerBase;

typedef shared_ptr<WaylandServerBase> WaylandServer;

struct WaylandServerBase: std::enable_shared_from_this<WaylandServerBase>
{
	static WaylandServer make();
	
	virtual void iteration() = 0;
};

