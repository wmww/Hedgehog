#include "Backend.h"
#include "BackendImplBase.h"

// change to toggle debug statements on and off
#define debug debug_on

Backend Backend::instance;

Backend::Backend(shared_ptr<ImplBase> implIn)
{
	impl = implIn;
	if (instance.impl != nullptr && instance.impl != implIn)
	{
		warning("created multiple backends");
	}
	instance.impl = implIn;
}

void Backend::swapBuffer()
{
	assert(impl);
	impl->swapBuffer();
}

void Backend::checkEvents()
{
	assert(impl);
	impl->checkEvents();
}

/*
void Backend::setInputDelegate(weak_ptr<InputDelegate> inputDelegate)
{
	assert(impl);
	impl->inputDelegate = inputDelegate;
}
*/

void * Backend::getXDisplay()
{
	assert(impl);
	return impl->getXDisplay();
}

