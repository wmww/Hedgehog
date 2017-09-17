#pragma once

#include "../main/util.h"
#include "../opengl/Texture.h"

#include <wayland-server.h>
#include <EGL/egl.h>

namespace WaylandEGL
{

void setEglVars(EGLDisplay eglDisplay, EGLContext eglContext);

void setup(wl_display * display);

void loadIntoTexture(wl_resource * buffer, Texture texture);

}
