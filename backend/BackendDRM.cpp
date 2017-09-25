#include "Backend.h"
#include "../wayland/WaylandEGL.h"

// change to toggle debug statements on and off
#define debug debug_off

extern "C"
{

int drmSetup();
void drmSwapBuffers();
void * drmGetEglDisplay();
void * drmGetEglContext();

}

extern bool stop;
bool libinput_setup();
void libinput_destroy();
void libinput_check_events(InputInterface * interface);

struct BackendDRM: Backend
{
	BackendDRM()
	{
		int setupRet = drmSetup();
		ASSERT_FATAL(setupRet == 0);
		WaylandEGL::setEglVars(drmGetEglDisplay(), drmGetEglContext());
		ASSERT_FATAL(libinput_setup());
	}
	
	~BackendDRM()
	{
		warning("~BackendDRM not implemented");
		libinput_destroy();
	}
	
	void swapBuffer()
	{
		drmSwapBuffers();
	}
	
	void checkEvents()
	{
		if (auto input = inputInterface.lock())
		{
			libinput_check_events(&*input);
			if (stop)
				Backend::instance = nullptr;
		}
	}
};

unique_ptr<Backend> makeDRMBackend()
{
	return make_unique<BackendDRM>();
}

