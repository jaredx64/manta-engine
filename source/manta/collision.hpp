#pragma once

#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CollisionResult
{
	CollisionResult() : hit { false }, toi { 1.0 } { }
	bool hit;
	double toi; // Time-of-impact in [0,1]
	double_v3 normal; // Surface normal at collision
	double_v3 point; // Hit point (optional)
};

extern CollisionResult sweep_sphere_vs_triangles(
	const double_v3 &center, double radius,
	const double_v3 &displacement,
	const double_v3 *vertices, usize count );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////