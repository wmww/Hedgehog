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
	ASSERT_ELSE(impl, return);
	impl->swapBuffer();
}

void Backend::checkEvents()
{
	ASSERT_ELSE(impl, return);
	impl->checkEvents();
}

string Backend::getKeymap()
{
	assert(impl);
	return impl->getKeymap();
}

void Backend::setInputInterface(weak_ptr<InputInterface> inputInterface)
{
	ASSERT_ELSE(impl, return);
	impl->inputInterface = inputInterface;
}

void * Backend::getXDisplay()
{
	ASSERT_ELSE(impl, return nullptr);
	return impl->getXDisplay();
}

