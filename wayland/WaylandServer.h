#pragma once

#include "../main/util.h"

namespace WaylandServer
{

void setup();

void shutdown();

void iteration();

uint32_t nextSerialNum();

};
