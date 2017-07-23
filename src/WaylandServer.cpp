#include "../h/WaylandServer.h"

#include <wayland-server.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

// no clue what this means
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

struct WaylandServerImpl: WaylandServerBase
{
	WaylandServerImpl(bool verboseIn)
	{
		verbose = verboseIn;
		
		if (verbose)
			cout << "starting Wayland server" << endl;
		
		// get function pointers
		eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL)eglGetProcAddress("eglBindWaylandDisplayWL");
		eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL)eglGetProcAddress("eglQueryWaylandBufferWL");
		glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
		
		wl_list_init(&clients);
		wl_list_init(&surfaces);
		
		display = wl_display_create();
		
		// automatically find a free socket and connect it to the display
		wl_display_add_socket_auto(display);
		
		if (verbose)
			cout << "Wayland server setup done" << endl;
	}
	
	~WaylandServerImpl()
	{
		if (verbose)
			cout << "shutting down Wayland server" << endl;
		wl_display_destroy(display);
		instance = nullptr;
	}
	
	void fail(string msg)
	{
		error = true;
		logError(msg);
	}
	
	static WaylandServer instance;
	
	bool error = false;
	bool verbose = false;
	
	// function pointers that need to be retrieved at run time. This is Cs sad, pathetic attempt at duck typing.
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
	PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = nullptr;
	PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = nullptr;
	
	wl_display * display = nullptr;
	
	wl_list clients;
	wl_list surfaces;
};

WaylandServer WaylandServerImpl::instance = nullptr;

WaylandServer WaylandServerBase::make(bool verbose)
{
	if (WaylandServerImpl::instance != nullptr)
	{
		logError("tried to create multiple WaylandServerImpl instances");
		return WaylandServerImpl::instance;
	}
	else
	{
		WaylandServerImpl::instance = WaylandServer(new WaylandServerImpl(verbose));
		return WaylandServerImpl::instance;
	}
}

