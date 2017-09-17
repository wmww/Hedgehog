#pragma once

#include "../main/util.h"
#include "../opengl/Texture.h"

#include <wayland-server.h>

namespace WaylandEGL
{

void setEglDisplay(void * eglDisplay);

void setup(wl_display * display);

void loadIntoTexture(wl_resource * buffer, Texture texture);

}
