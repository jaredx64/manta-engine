#include <manta/collision.hpp>

#include <vendor/vendor.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool point_in_triangle( const double_v3 &point, const double_v3 &vert0,
	const double_v3 &vert1, const double_v3 &vert2 )
{
	const double_v3 v0v1 = vert1 - vert0;
	const double_v3 v0v2 = vert2 - vert0;
	const double_v3 v0p0 = point - vert0;

	const double dot00 = v0v1.dot( v0v1 );
	const double dot01 = v0v1.dot( v0v2 );
	const double dot11 = v0v2.dot( v0v2 );

	const double denom = dot00 * dot11 - dot01 * dot01;
	if( abs( denom ) < 1e-12 )
	{
		return ( point - vert0 ).length_sqr() < 1e-12 ||
			( point - vert1 ).length_sqr() < 1e-12 ||
			( point - vert2 ).length_sqr() < 1e-12;
	}

	const double dot02 = v0v1.dot( v0p0 );
	const double dot12 = v0v2.dot( v0p0 );
	const double invDenom = 1.0 / denom;

	const double u = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
	const double v = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;

	return ( u >= -1e-12 ) && ( v >= -1e-12 ) && ( u + v <= 1.0 + 1e-12 );
}


static double_v3 closest_point_on_triangle( const double_v3 &point, const double_v3 &v0,
	const double_v3 &v1, const double_v3 &v2 )
{
	const double_v3 v0v1 = v1 - v0;
	const double_v3 v0v2 = v2 - v0;
	const double_v3 v0p0 = point - v0;

	const double d1 = v0v1.dot( v0p0 );
	const double d2 = v0v2.dot( v0p0 );

	if( d1 <= 0.0 && d2 <= 0.0 ) { return v0; }

	const double_v3 bp = point - v1;
	const double d3 = v0v1.dot( bp );
	const double d4 = v0v2.dot( bp );

	if( d3 >= 0.0 && d4 <= d3 ) { return v1; }

	const double_v3 cp = point - v2;
	const double d5 = v0v1.dot( cp );
	const double d6 = v0v2.dot( cp );

	if( d6 >= 0.0 && d5 <= d6 ) { return v2; }

	const double vc = d1 * d4 - d3 * d2;
	if( vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0 )
	{
		const double v = d1 / ( d1 - d3 );
		return v0 + v0v1 * v;
	}

	const double vb = d5 * d2 - d1 * d6;
	if( vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0 )
	{
		const double w = d2 / ( d2 - d6 );
		return v0 + v0v2 * w;
	}

	const double va = d3 * d6 - d5 * d4;
	if( va <= 0.0 && ( d4 - d3 ) >= 0.0 && ( d5 - d6 ) >= 0.0 )
	{
		const double w = ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) );
		return v1 + ( v2 - v1 ) * w;
	}

	const double denom = 1.0 / ( va + vb + vc );
	const double v = vb * denom;
	const double w = vc * denom;
	return v0 + v0v1 * v + v0v2 * w;
}


bool solve_quadratic( double a, double b, double c, double max_t, double &t )
{
	if( abs( a ) < 1e-12 )
	{
		if( abs( b ) < 1e-12 ) { return false; }
		t = -c / b;
		return t >= 0.0 && t <= max_t;
	}

	const double discriminant = b * b - 4 * a * c;
	if( discriminant < 0 ) { return false; }

	const double discriminantSqrt = sqrt( discriminant );
	double t1 = ( -b - discriminantSqrt ) / ( 2.0 * a );
	double t2 = ( -b + discriminantSqrt ) / ( 2.0 * a );

	if( t1 > t2 )
	{
		const double tmp = t1;
		t1 = t2; t2 = tmp;
	}

	if( t1 >= 0.0 && t1 <= max_t )
	{
		t = t1;
		return true;
	}

	if( t2 >= 0 && t2 <= max_t )
	{
		t = t2;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CollisionResult sweep_sphere_vs_triangles( const double_v3 &center, const double radius,
	const double_v3 &displacement, const double_v3 *vertices, const usize count )
{
	const double radiusSqr = radius * radius;

	CollisionResult result;
	result.toi = 1.0;
	result.hit = false;

	// Early out if no displacement
	if( displacement.length_sqr() < 1e-16 )
	{
		// Check for static collision
		for( usize i = 0; i < count; i += 3 )
		{
			const double_v3 &v0 = vertices[i + 0];
			const double_v3 &v1 = vertices[i + 1];
			const double_v3 &v2 = vertices[i + 2];

			const double_v3 pointClosest = closest_point_on_triangle( center, v0, v1, v2 );
			const double distanceSqr = ( center - pointClosest ).length_sqr();

			if( distanceSqr <= radiusSqr + 1e-12 )
			{
				result.hit = true;
				result.toi = 0.0;
				result.normal = ( center - pointClosest ).normalize();
				result.point = pointClosest;
				return result;
			}
		}
		return result;
	}

	// Iterate through all triangles
	for( usize i = 0; i < count; i += 3 )
	{
		const double_v3 &v0 = vertices[i + 0];
		const double_v3 &v1 = vertices[i + 1];
		const double_v3 &v2 = vertices[i + 2];

		// Compute triangle normal
		const double_v3 edge1 = v1 - v0;
		const double_v3 edge2 = v2 - v0;
		const double_v3 normal = double_v3_cross( edge1, edge2 ).normalize();
#if 0
		const double areaSqr = normal.length_sqr();
		if( UNLIKELY( areaSqr < 1e-16 ) ) { continue; } // Degenerate triangle
#endif

		// Check if moving toward the triangle
		const double dotMovement = displacement.dot( normal );
		if( dotMovement > -1e-12 ) { continue; }

		// Calculate distance from sphere center to triangle plane
		const double distanceToTriangle = ( center - v0 ).dot( normal );
		const double distanceToTriangleMinusRadius = ( distanceToTriangle - radius );
		const double distanceToTriangleMinusRadiusSqr = distanceToTriangleMinusRadius * distanceToTriangleMinusRadius;
		if( distanceToTriangle > radius && distanceToTriangleMinusRadiusSqr > displacement.length_sqr() ) { continue; }

		// Calculate time of intersection with the expanded plane (considering sphere radius)
		const double t = clamp( ( radius - distanceToTriangle ) / dotMovement, 0.0, 1.0 );
		if( t >= result.toi ) { continue; }

		// Calculate sphere center at time t & closest point on triangle to that position
		const double_v3 centerAtTime = center + displacement * t;
		const double_v3 pointClosest = closest_point_on_triangle( centerAtTime, v0, v1, v2 );
		const double distanceSqr = ( centerAtTime - pointClosest ).length_sqr();

		// Check if sphere touches or intersects the triangle at time t
		if( distanceSqr <= radiusSqr + 1e-12 )
		{
			result.hit = true;
			result.toi = t;
			result.normal = ( centerAtTime - pointClosest ).normalize();
			result.point = pointClosest;
			if( t <= 1e-12 ) { return result; } // Early out if t = 0.0 collision
		}
	}

	return result;
}

// Capsule vs Triangles swept test
CollisionResult sweep_capsule_vs_triangles(
	const double_v3 &p0, const double_v3 &p1, double radius,
	const double_v3 &displacement,
	const double_v3 *vertices, usize count )
{

	CollisionResult result;
	result.toi = 1.0;

	// For capsules, we can reduce the problem to a sphere swept along the capsule axis
	double_v3 axis = p1 - p0;
	double axis_length = axis.length();
	double_v3 axis_dir = axis.normalize();

	// Iterate through all triangles
	for( usize i = 0; i < count; i += 3 )
	{
		const double_v3 &v0 = vertices[i + 0];
		const double_v3 &v1 = vertices[i + 1];
		const double_v3 &v2 = vertices[i + 2];

		// Compute triangle normal
		const double_v3 edge1 = v1 - v0;
		const double_v3 edge2 = v2 - v0;
		const double_v3 normal = double_v3_cross( edge1, edge2 ).normalize();

		// Find the closest point on the capsule segment to the triangle
		const double_v3 center = v0 + v1 + v2;
		const double_v3 tri_center = double_v3_divide( center, 3.0 );
		const double_v3 to_tri = tri_center - p0;
		const double proj = clamp( to_tri.dot( axis_dir ), 0.0, axis_length );
		const double_v3 capsule_point = p0 + axis_dir * proj;

		// Test sphere at this point against the triangle
		CollisionResult sphere_result = sweep_sphere_vs_triangles(
			capsule_point, radius, displacement, &vertices[i], 3 );

		if( sphere_result.hit && sphere_result.toi < result.toi )
		{
			result = sphere_result;
		}
	}

	return result;
}

// AABB vs Triangles swept test
CollisionResult sweep_aabb_vs_triangles(
	const double_v3 &min, const double_v3 &max,
	const double_v3 &displacement,
	const double_v3 *vertices, usize count )
{
	CollisionResult result;
	result.toi = 1.0;

	// For AABBs, we can use the separating axis theorem or approximate with sphere tests
	// at the corners. This is a simplified approach.
	const double_v3 center = ( min + max ) * 0.5;
	const double_v3 extents = ( max - min ) * 0.5;
	const double radius = extents.length();  // Sphere that encloses the AABB

	// Test with an enclosing sphere first
	CollisionResult sphere_result = sweep_sphere_vs_triangles(
		center, radius, displacement, vertices, count );

	if (sphere_result.hit) {
		// Refine the test by checking the AABB more accurately
		// This is a placeholder - a real implementation would use
		// the separating axis theorem for continuous collision detection
		result = sphere_result;

		// Adjust normal based on the actual AABB face that collided
		if( result.hit )
		{
			// Find the dominant axis of the normal
			const double_v3 abs_normal = double_v3 { abs( result.normal.x ),
				abs( result.normal.x ), abs( result.normal.z ) };

			if( abs_normal.x >= abs_normal.y && abs_normal.x >= abs_normal.z )
			{
				result.normal = double_v3 { ( result.normal.x > 0.0 ) ? 1.0 : -1.0, 0.0, 0.0 };
			}
			else if( abs_normal.y >= abs_normal.x && abs_normal.y >= abs_normal.z )
			{
				result.normal = double_v3 { 0, ( result.normal.y > 0.0 ) ? 1.0 : -1.0, 0.0 };
			}
			else
			{
				result.normal = double_v3 { 0.0, 0.0, ( result.normal.z > 0.0 ) ? 1.0 : -1.0 };
			}
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////