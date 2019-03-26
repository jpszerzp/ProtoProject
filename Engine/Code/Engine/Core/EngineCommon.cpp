#include "Engine/Core/EngineCommon.hpp"

Blackboard* g_gameConfigBlackboard = nullptr;
Clock* g_masterClock = nullptr;
RCS* g_rcs = nullptr;
bool g_broadphase = false;
QuickHull* g_hull = nullptr;
CollisionKeep* g_col_keep = nullptr;
ProfilerTree* g_my_tree = nullptr;
ProfilerTree* g_phys_tree = nullptr;