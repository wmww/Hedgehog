#pragma once

#include "../main/utils.h"

namespace WaylandServer
{

void setup();

void shutdown();

void iteration();

uint32_t nextSerialNum();

};
