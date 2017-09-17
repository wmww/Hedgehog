#pragma once

#include "../main/util.h"
#include <wayland-server.h>

namespace WaylandEGL
{

void setEglDisplay(void * eglDisplay);

void setup(wl_display * display);

}
