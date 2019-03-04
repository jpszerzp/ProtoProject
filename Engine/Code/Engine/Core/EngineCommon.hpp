#pragma once

#include "Engine/Core/Profiler/ProfileLogScoped.hpp"
#include "Engine/Core/Util/StringUtils.hpp"

#define UNUSED(x) (void)(x);
#define PROFILE_LOG_SCOPED(tag) ProfileLogScoped var(tag);
#define PROFILE_LOG_SCOPED_FUNCTION() ProfileLogScoped var(__FUNCTION__);
#define PROFILE_LOG_SCOPED_PUSH() ProfilePush()
#define PROFILE_LOG_SCOPED_POP() ProfilePop()
#define PROFILE_ENABLED

class Blackboard;
class Clock;
class RCS;
class QuickHull;
class CollisionKeep;

extern Blackboard*		g_gameConfigBlackboard;
extern Clock*			g_masterClock;
extern RCS*				g_rcs;
extern bool				g_broadphase;
extern QuickHull*		g_hull;
extern CollisionKeep*	g_col_keep;

typedef unsigned int uint;