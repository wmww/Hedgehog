#include "../h/WaylandServer.h"

struct WaylandServerImpl: WaylandServerBase
{
	WaylandServerImpl()
	{
		
	}
	
};

WaylandServer WaylandServerBase::make()
{
	return WaylandServer(new WaylandServerImpl());
}

