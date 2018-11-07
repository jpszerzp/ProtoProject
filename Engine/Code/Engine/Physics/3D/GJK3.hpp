#pragma once

#include "Engine/Math/Vector3.hpp"

#include <set>

// We use GetPointSet from QH to generate hull point sets initially
// given point sets of two convex hull A and B, generate the set that represents the minkowski difference of them
std::set<Vector3> InitializeHullGJK(const std::set<Vector3>& hull_0, const std::set<Vector3>& hull_1);

// given minkowski difference of A and B, initialize the simplex set S with ONE point
void InitializeSimplexGJK();

// Initialize the support direction for this minkowski (difference) hull
void InitializeSupportDirectionGJK();

// given direction, generate support point of a single separate convex hull
Vector3 GenerateSupportSeparateGJK(const Vector3& direction, const std::set<Vector3>& hull);

// given direction, generate support point of a minkowski convex hull combined with info from two convex hulls
Vector3 GenerateSupportCombinedGJK(const Vector3& direction, const std::set<Vector3>& minkowski);

// given old support point (point from last round of computation), see if
// the support point candidate we just generated is more extremal compared to it
// this effectively decides if we abort the program
// if the candidate is NOT more extremal, it means that we cannot approach any closer to the origin
// on the minkowski convex hull, hence there is no chance for A and B to intersect, resulting in exiting the algorithm
bool IsSupportExtremalGJK(const Vector3& candidate, const Vector3& old_support);

// Support point is ALWAYS candidate for simplex, therefore we add it to the set.
// Note that we may need to reduce the simplex set later, 
// therefore it is better to differentiate from "Reduce" and call this "Increment"
void IncrementSimplexGJK(const Vector3& support, std::set<Vector3>& simplex);

// Update support direction and reassign support base point
// In initialization, support base point is just the point we first put into the simplex set
// and the support direction is simply the direction from that point to the origin
// See initialization functions above.
// Here we are way into iterations, so we need to find the "minimum norm" from simplex set to the origin,
// assign that vector as the new support direction, and the root of that norm 
// (which is just the closest point on simplex to the origin) as the base point of support
void GenerateSupportDirectionGJK(Vector3& direction);
void GenerateSupportBaseGJK(Vector3& base);

// Given the base point and simplex set, we need to cut the simplex to 
// a minimal one that is just enough to determine the base.
// This may or may NOT require removing points from simplex set.
// But it will NEVER add points to the set.
// Note that after this step, we are usually read to call GenerateSupportCombinedGJK
// to generate the new support candidate and proceed with the algorithm from there
void ReduceSimplexGJK(const Vector3& base, std::set<Vector3>& simplex);


// At this point, we wonder when do we stop the algorithm?
// In IsSupportExtremalGJK we have a chance to stop it when we are certain that A and B will NOT intersect.
// To save for the situation where the two DO intersect, note that the step of generating support point candidate
// will generate the origin itself at last iteration due to the way it is implemented.
// Therefore, given a simplex set with the origin, every time we generate a support base point for it, we should
// end up with the origin itself.
// Therefore a check on whether the support base is the origin will cover the case where the origin is inside the minkowski hull,
// which means the two shapes DO intersect. 
// After this check we just need to stop the program and extract intersection information we are interested in.
bool IsSupportBaseEqualOrigin(const Vector3& base);