#pragma once

#include "utils.h"

struct WaylandServerBase;

typedef shared_ptr<WaylandServerBase> WaylandServer;

struct WaylandServerBase: std::enable_shared_from_this<WaylandServerBase>
{
	static WaylandServer make(bool verbose);
	
	virtual void iteration() = 0;
};

