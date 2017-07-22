#pragma once

#include "utils.h"

struct WaylandServerBase;

typedef shared_ptr<WaylandServerBase> WaylandServer;

struct WaylandServerBase
{
	static WaylandServer make();
};

