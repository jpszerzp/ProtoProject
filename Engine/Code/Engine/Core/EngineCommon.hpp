#pragma once

#include "Engine/Core/Util/StringUtils.hpp"

#define UNUSED(x) (void)(x);

class Blackboard;
class Clock;

extern Blackboard*		g_config_blackboard;
extern Clock*			g_masterClock;

// type defs
typedef unsigned int uint;