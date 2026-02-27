#include <manta/3d.hpp>

#include <core/math.hpp>
#include <core/hashmap.hpp>

#include <manta/gfx.hpp>
#include <manta/draw.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EPSILON_FLOAT ( 1e-6f )
#define EPSILON_DOUBLE ( 1e-12 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Point

float_v2 point_world_to_screen( const float_v3 &pointWorld, const float_v2 &dimensionsScreen,
	const float_m44 &matrixView, const float_m44 &matrixProjection )
{
	const float_m44 matWorldToScreen = float_m44_multiply(
		float_m44_build_ndc( 1.0f, 1.0f ),
		float_m44_multiply( matrixProjection, matrixView ) );

	float_v4 world = { pointWorld.x, pointWorld.y, pointWorld.z, 1.0f };
	float_v4 screen = world.multiply( matWorldToScreen );

	if( screen.w <= 0.0f ) { return float_v2 { FLOAT_MIN, FLOAT_MIN }; }
	return screen.xy() * dimensionsScreen;
}


double_v2 point_world_to_screen( const double_v3 &pointWorld, const double_v2 &dimensionsScreen,
	const double_m44 &matrixView, const double_m44 &matrixProjection )
{
	const double_m44 matWorldToScreen = float_m44_multiply(
		double_m44_build_ndc( 1.0, 1.0 ),
		double_m44_multiply( matrixProjection, matrixView ) );

	double_v4 world = { pointWorld.x, pointWorld.y, pointWorld.z, 1.0 };
	double_v4 screen = world.multiply( matWorldToScreen );

	if( screen.w <= 0.0 ) { return double_v2 { FLOAT_MIN, FLOAT_MIN }; }
	return screen.xy() * dimensionsScreen;
}


float_v3 point_closest_on_line( const float_v3 &point, const float_v3 &v0, const float_v3 &v1 )
{
	const float_v3 ab = v1 - v0;
	const float_v3 ap = point - v0;

	const float denom = float_v3_dot( ab, ab );
	if( denom <= 0.0f ) { return v0; }

	float t = float_v3_dot( ap, ab ) / denom;
	if( t < 0.0f ) { t = 0.0f; }
	if( t > 1.0f ) { t = 1.0f; }

	return v0 + ab * t;
}


double_v3 point_closest_on_line( const double_v3 &point, const double_v3 &v0, const double_v3 &v1 )
{
	const double_v3 ab = v1 - v0;
	const double_v3 ap = point - v0;

	const double denom = double_v3_dot( ab, ab );
	if( denom <= 0.0 ) { return v0; }

	double t = double_v3_dot( ap, ab ) / denom;
	if( t < 0.0 ) { t = 0.0; }
	if( t > 1.0 ) { t = 1.0; }

	return v0 + ab * t;
}


float_v3 closest_point_on_ray( const float_v3 &point, const float_r3 &ray )
{
	const float_v3 op = point - ray.origin;

	const float denom = float_v3_dot( ray.vector, ray.vector );
	if( denom <= 0.0f ) { return ray.origin; }

	float t = float_v3_dot( op, ray.vector ) / denom;
	if( t < 0.0f ) { t = 0.0f; }

	return ray.origin + ray.vector * t;
}


double_v3 closest_point_on_ray( const double_v3 &point, const double_r3 &ray )
{
	const double_v3 op = point - ray.origin;

	const double denom = double_v3_dot( ray.vector, ray.vector );
	if( denom <= 0.0 ) { return ray.origin; }

	double t = double_v3_dot( op, ray.vector ) / denom;
	if( t < 0.0 ) { t = 0.0; }

	return ray.origin + ray.vector * t;
}


float_v3 closest_point_on_triangle( const float_v3 &point,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2 )
{
	// NOTE: Reference Christer Ericson's "Real-Time Collision Detection"

	float_v3 ab = v1 - v0;
	float_v3 ac = v2 - v0;
	float_v3 ap = point - v0;

	const float d1 = float_v3_dot( ab, ap );
	const float d2 = float_v3_dot( ac, ap );
	if( d1 <= 0.0f && d2 <= 0.0f ) { return v0; }

	const float_v3 bp = point - v1;
	const float d3 = float_v3_dot( ab, bp );
	const float d4 = float_v3_dot( ac, bp );
	if( d3 >= 0.0f && d4 <= d3 ) { return v1; }

	const float vc = d1 * d4 - d3 * d2;
	if( vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f )
	{
		const float v = d1 / ( d1 - d3 );
		return v0 + ab * v;
	}

	const float_v3 cp = point - v2;
	const float d5 = float_v3_dot( ab, cp );
	const float d6 = float_v3_dot( ac, cp );
	if( d6 >= 0.0f && d5 <= d6 ) { return v2; }

	const float vb = d5 * d2 - d1 * d6;
	if( vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f )
	{
		float w = d2 / ( d2 - d6 );
		return v0 + ac * w;
	}

	const float va = d3 * d6 - d5 * d4;
	if( va <= 0.0f && ( d4 - d3 ) >= 0.0f && ( d5 - d6 ) >= 0.0f )
	{
		float w = ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) );
		return v1 + ( v2 - v1 ) * w;
	}

	const float denom = 1.0f / ( va + vb + vc );
	const float v = vb * denom;
	const float w = vc * denom;
	return v0 + ab * v + ac * w;
}


double_v3 closest_point_on_triangle( const double_v3 &point,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2 )
{
	// NOTE: Reference Christer Ericson's "Real-Time Collision Detection"

	double_v3 ab = v1 - v0;
	double_v3 ac = v2 - v0;
	double_v3 ap = point - v0;

	const double d1 = double_v3_dot( ab, ap );
	const double d2 = double_v3_dot( ac, ap );
	if( d1 <= 0.0 && d2 <= 0.0 ) { return v0; }

	const double_v3 bp = point - v1;
	const double d3 = double_v3_dot( ab, bp );
	const double d4 = double_v3_dot( ac, bp );
	if( d3 >= 0.0 && d4 <= d3 ) { return v1; }

	const double vc = d1 * d4 - d3 * d2;
	if( vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0 )
	{
		const double v = d1 / ( d1 - d3 );
		return v0 + ab * v;
	}

	const double_v3 cp = point - v2;
	const double d5 = double_v3_dot( ab, cp );
	const double d6 = double_v3_dot( ac, cp );
	if( d6 >= 0.0 && d5 <= d6 ) { return v2; }

	const double vb = d5 * d2 - d1 * d6;
	if( vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0 )
	{
		double w = d2 / ( d2 - d6 );
		return v0 + ac * w;
	}

	const double va = d3 * d6 - d5 * d4;
	if( va <= 0.0 && ( d4 - d3 ) >= 0.0 && ( d5 - d6 ) >= 0.0 )
	{
		double w = ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) );
		return v1 + ( v2 - v1 ) * w;
	}

	const double denom = 1.0 / ( va + vb + vc );
	const double v = vb * denom;
	const double w = vc * denom;
	return v0 + ab * v + ac * w;
}


float_v3 point_closest_on_aabb( const float_v3 &point, const float_v3 &min, const float_v3 &max )
{
	float_v3 out;
	out.x = ( point.x < min.x ) ? min.x : ( point.x > max.x ? max.x : point.x );
	out.y = ( point.y < min.y ) ? min.y : ( point.y > max.y ? max.y : point.y );
	out.z = ( point.z < min.z ) ? min.z : ( point.z > max.z ? max.z : point.z );
	return out;
}


double_v3 point_closest_on_aabb( const double_v3 &point, const double_v3 &min, const double_v3 &max )
{
	double_v3 out;
	out.x = ( point.x < min.x ) ? min.x : ( point.x > max.x ? max.x : point.x );
	out.y = ( point.y < min.y ) ? min.y : ( point.y > max.y ? max.y : point.y );
	out.z = ( point.z < min.z ) ? min.z : ( point.z > max.z ? max.z : point.z );
	return out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ray

float_r3 ray_screen_point_to_world_perspective( const float_v2 &pointScreen, const float_v2 &dimensionsScreen,
	const float_v3 &pointEye, const float_m44 &matrixView, const float_m44 &matrixProjection )
{
	const float_m44 matrixProjectionInv = float_m44_inverse( matrixProjection );
	const float_m44 matrixViewInv = float_m44_inverse( matrixView );

	const float xNDC = ( 2.0f * pointScreen.x ) / dimensionsScreen.x - 1.0f;
	const float yNDC = 1.0f - ( 2.0f * pointScreen.y ) / dimensionsScreen.y;

	float_v4 pClip = float_v4 { xNDC, yNDC, -1.0f, 1.0f };
	float_v4 pView = pClip.multiply( matrixProjectionInv );
	pView /= pView.w;

	const float_v4 pWorld = pView.multiply( matrixViewInv );

	const float_r3 ray =
	{
		pointEye,
		float_v3 { pWorld.x - pointEye.x, pWorld.y - pointEye.y, pWorld.z - pointEye.z }.normalize()
	};

	return ray;
}


double_r3 ray_screen_point_to_world_perspective( const double_v2 &pointScreen, const double_v2 &dimensionsScreen,
	const double_v3 &pointEye, const double_m44 &matrixView, const double_m44 &matrixProjection )
{
	const double_m44 matrixProjectionInv = double_m44_inverse( matrixProjection );
	const double_m44 matrixViewInv = double_m44_inverse( matrixView );

	const double xNDC = ( 2.0 * pointScreen.x ) / dimensionsScreen.x - 1.0;
	const double yNDC = 1.0 - ( 2.0 * pointScreen.y ) / dimensionsScreen.y;

	double_v4 pClip = double_v4 { xNDC, yNDC, -1.0, 1.0 };
	double_v4 pView = pClip.multiply( matrixProjectionInv );
	pView /= pView.w;

	const double_v4 pWorld = pView.multiply( matrixViewInv );

	const double_r3 ray =
	{
		pointEye,
		double_v3 { pWorld.x - pointEye.x, pWorld.y - pointEye.y, pWorld.z - pointEye.z }.normalize()
	};

	return ray;
}


bool ray_intersects_aabb( const float_r3 &ray, const float_v3 &xyzMin, const float_v3 &xyzMax,
	float &hitDistance )
{
	const float_v3 rayVectorInv = float_v3
	{
		1.0f / ray.vector.x,
		1.0f / ray.vector.y,
		1.0f / ray.vector.z
	};

	const float t1 = ( xyzMin.x - ray.origin.x ) * rayVectorInv.x;
	const float t2 = ( xyzMax.x - ray.origin.x ) * rayVectorInv.x;
	const float t3 = ( xyzMin.y - ray.origin.y ) * rayVectorInv.y;
	const float t4 = ( xyzMax.y - ray.origin.y ) * rayVectorInv.y;
	const float t5 = ( xyzMin.z - ray.origin.z ) * rayVectorInv.z;
	const float t6 = ( xyzMax.z - ray.origin.z ) * rayVectorInv.z;

	const float tmin = max( max( min( t1, t2 ), min( t3, t4 ) ), min( t5, t6 ) );
	const float tmax = min( min( max( t1, t2 ), max( t3, t4 ) ), max( t5, t6 ) );

	if( tmax < 0.0f )
	{
		hitDistance = tmax;
		return false;
	}

	if( tmin > tmax )
	{
		hitDistance = tmax;
		return false;
	}

	hitDistance = tmin;
	return true;
}


bool ray_intersects_aabb( const double_r3 &ray, const double_v3 &xyzMin, const double_v3 &xyzMax,
	float &hitDistance )
{
	const double_v3 rayVectorInv = double_v3
	{
		1.0 / ray.vector.x,
		1.0 / ray.vector.y,
		1.0 / ray.vector.z
	};

	const double t1 = ( xyzMin.x - ray.origin.x ) * rayVectorInv.x;
	const double t2 = ( xyzMax.x - ray.origin.x ) * rayVectorInv.x;
	const double t3 = ( xyzMin.y - ray.origin.y ) * rayVectorInv.y;
	const double t4 = ( xyzMax.y - ray.origin.y ) * rayVectorInv.y;
	const double t5 = ( xyzMin.z - ray.origin.z ) * rayVectorInv.z;
	const double t6 = ( xyzMax.z - ray.origin.z ) * rayVectorInv.z;

	const double tmin = max( max( min( t1, t2 ), min( t3, t4 ) ), min( t5, t6 ) );
	const double tmax = min( min( max( t1, t2 ), max( t3, t4 ) ), max( t5, t6 ) );

	if( tmax < 0.0 )
	{
		hitDistance = tmax;
		return false;
	}

	if( tmin > tmax )
	{
		hitDistance = tmax;
		return false;
	}

	hitDistance = tmin;
	return true;
}


bool ray_intersect_triangle( const float_r3 &ray,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2, float_r3::hit &hit )
{
	float_v3 edge1 = float_v3 { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
	float_v3 edge2 = float_v3 { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

	float_v3 pvec = float_v3_cross( ray.vector, edge2 );
	float det = edge1.dot( pvec );
	if( det > -EPSILON_FLOAT && det < EPSILON_FLOAT ) { return false; }

	float invDet = 1.0f / det;
	float_v3 tvec = float_v3 { ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z };
	hit.baryU = tvec.dot( pvec ) * invDet;
	if( hit.baryU < 0.0f || hit.baryU > 1.0f ) { return false; }

	float_v3 qvec = tvec.cross( edge1 );
	hit.baryV = ray.vector.dot( qvec ) * invDet;
	if( hit.baryV < 0.0f || hit.baryU + hit.baryV > 1.0f ) { return false; }

	hit.distance = edge2.dot( qvec ) * invDet;
	return ( hit.distance > EPSILON_FLOAT );
}


bool ray_intersect_triangle( const double_r3 &ray,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2, double_r3::hit &hit )
{
	double_v3 edge1 = double_v3 { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
	double_v3 edge2 = double_v3 { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

	double_v3 pvec = double_v3_cross( ray.vector, edge2 );
	double det = edge1.dot( pvec );
	if( det > -EPSILON_DOUBLE && det < EPSILON_DOUBLE ) { return false; }

	double invDet = 1.0 / det;
	double_v3 tvec = double_v3 { ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z };
	hit.baryU = tvec.dot( pvec ) * invDet;
	if( hit.baryU < 0.0 || hit.baryU > 1.0 ) { return false; }

	double_v3 qvec = tvec.cross( edge1 );
	hit.baryV = ray.vector.dot( qvec ) * invDet;
	if( hit.baryV < 0.0 || hit.baryU + hit.baryV > 1.0 ) { return false; }

	hit.distance = edge2.dot( qvec ) * invDet;
	return ( hit.distance > EPSILON_DOUBLE );
}


bool ray_intersect_triangle_list( const float_r3 &ray,
	const float_v3 *verts, usize count, float_r3::hit &hit )
{
	hit.distance = FLOAT_MAX;
	float_r3::hit cur;

	for( usize i = 0; i + 2 < count; i += 3 )
	{
		if( ray_intersect_triangle( ray, verts[i + 0], verts[i + 1], verts[i + 2], cur ) &&
			cur.distance < hit.distance )
		{
			hit = cur;
			hit.triangleID = i / 3;
		}
	}

	return hit.distance < FLOAT_MAX;
}


bool ray_intersect_triangle_list( const double_r3 &ray,
	const double_v3 *verts, usize count, double_r3::hit &hit )
{
	hit.distance = FLOAT_MAX;
	double_r3::hit cur;

	for( usize i = 0; i + 2 < count; i += 3 )
	{
		if( ray_intersect_triangle( ray, verts[i + 0], verts[i + 1], verts[i + 2], cur ) &&
			cur.distance < hit.distance )
		{
			hit = cur;
			hit.triangleID = i / 3;
		}
	}

	return hit.distance < FLOAT_MAX;
}


bool ray_intersect_triangle_list( const float_r3 &ray, const float_m44 &matrix,
	const float_v3 *verts, usize count, float_r3::hit &hit )
{
	const float_m44 matrixInv = float_m44_inverse( matrix );
	const float_r3 rayLocal = float_r3
		{
			float_v4_multiply(
				float_v4 { ray.origin.x, ray.origin.y, ray.origin.z, 1.0f }, matrixInv ).xyz(),
			float_v4_multiply(
				float_v4 { ray.vector.x, ray.vector.y, ray.vector.z, 0.0f }, matrixInv ).xyz(),
		};

	hit.distance = FLOAT_MAX;
	float_r3::hit cur;

	for( usize i = 0; i + 2 < count; i += 3 )
	{
		if( ray_intersect_triangle( rayLocal, verts[i + 0], verts[i + 1], verts[i + 2], cur ) &&
			cur.distance < hit.distance )
		{
			hit = cur;
			hit.triangleID = i / 3;
		}
	}

	return hit.distance < FLOAT_MAX;
}


bool ray_intersect_triangle_list( const double_r3 &ray, const double_m44 &matrix,
	const double_v3 *verts, usize count, double_r3::hit &hit )
{
	const double_m44 matrixInv = double_m44_inverse( matrix );
	const double_r3 rayLocal = double_r3
		{
			double_v4_multiply(
				double_v4 { ray.origin.x, ray.origin.y, ray.origin.z, 1.0 }, matrixInv ).xyz(),
			double_v4_multiply(
				double_v4 { ray.vector.x, ray.vector.y, ray.vector.z, 0.0 }, matrixInv ).xyz(),
		};
	const double rayLocalScaleInv = 1.0 / double_v3_length( rayLocal.vector );

	hit.distance = FLOAT_MAX;
	double_r3::hit cur;

	for( usize i = 0; i + 2 < count; i += 3 )
	{
		if( ray_intersect_triangle( rayLocal, verts[i + 0], verts[i + 1], verts[i + 2], cur ) &&
			cur.distance < hit.distance )
		{
			hit = cur;
			hit.triangleID = i / 3;
		}
	}

	if( hit.distance < FLOAT_MAX )
	{
		hit.distance *= rayLocalScaleInv;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Triangle

float_v3 triangle_calculate_normal_3d( const float_v3 &v0, const float_v3 &v1, const float_v3 &v2 )
{
	return ( v1 - v0 ).cross( v2 - v0 ).normalize();
}


double_v3 triangle_calculate_normal_3d( const double_v3 &v0, const double_v3 &v1, const double_v3 &v2 )
{
	return ( v1 - v0 ).cross( v2 - v0 ).normalize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void aabb_sweep_util_project_triangle( const float_v3 &axis,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2, float &outMin, float &outMax )
{
	float p0 = float_v3_dot( v0, axis );
	float p1 = float_v3_dot( v1, axis );
	float p2 = float_v3_dot( v2, axis );
	outMin = fminf( p0, fminf( p1, p2 ) );
	outMax = fmaxf( p0, fmaxf( p1, p2 ) );
}


static void aabb_sweep_util_project_triangle( const double_v3 &axis,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2, double &outMin, double &outMax )
{
	double p0 = double_v3_dot( v0, axis );
	double p1 = double_v3_dot( v1, axis );
	double p2 = double_v3_dot( v2, axis );
	outMin = fmin( p0, fmin( p1, p2 ) );
	outMax = fmax( p0, fmax( p1, p2 ) );
}


static void aabb_sweep_util_project_swept_aabb( const float_v3 &axis,
	const float_v3 &center, const float_v3 &extents, float &outMin, float &outMax )
{
	float c = float_v3_dot( center, axis );
	float r = fabsf( axis.x ) * extents.x + fabsf( axis.y ) * extents.y + fabsf( axis.z ) * extents.z;
	outMin = c - r;
	outMax = c + r;
}


static void aabb_sweep_util_project_swept_aabb( const double_v3 &axis,
	const double_v3 &center, const double_v3 &extents, double &outMin, double &outMax )
{
	double c = double_v3_dot( center, axis );
	double r = fabs( axis.x ) * extents.x + fabs( axis.y ) * extents.y + fabs( axis.z ) * extents.z;
	outMin = c - r;
	outMax = c + r;
}


static bool aabb_sweep_util_sweep_intervals( float minA, float maxA, float minB, float maxB, float vel,
	float &tEnter, float &tExit )
{
	if( fabsf( vel ) < EPSILON_FLOAT )
	{
		if( maxA < minB || minA > maxB ) { return false; }
		tEnter = 0.0f;
		tExit = 1.0f;
		return true;
	}

	const float velInv = 1.0f / vel;
	float t0 = ( minB - maxA ) * velInv;
	float t1 = ( maxB - minA ) * velInv;

	if( t0 > t1 )
	{
		float tmp = t0;
		t0 = t1;
		t1 = tmp;
	}

	tEnter = t0;
	tExit = t1;
	return true;
}


static bool aabb_sweep_util_sweep_intervals( double minA, double maxA, double minB, double maxB, double vel,
	double &tEnter, double &tExit )
{
	if( fabs( vel ) < EPSILON_DOUBLE )
	{
		if( maxA < minB || minA > maxB ) { return false; }
		tEnter = 0.0;
		tExit = 1.0;
		return true;
	}

	const double velInv = 1.0 / vel;
	double t0 = ( minB - maxA ) * velInv;
	double t1 = ( maxB - minA ) * velInv;

	if( t0 > t1 )
	{
		double tmp = t0;
		t0 = t1;
		t1 = tmp;
	}

	tEnter = t0;
	tExit = t1;
	return true;
}


bool aabb_sweep_triangle( const float_v3 &aabbMin, const float_v3 &aabbMax,
	const float_v3 &position, const float_v3 &velocity,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2,
	AABBSweepHitFloat &hit )
{
	const float_v3 extents = float_v3
	{
		( aabbMax.x - aabbMin.x ) * 0.5f,
		( aabbMax.y - aabbMin.y ) * 0.5f,
		( aabbMax.z - aabbMin.z ) * 0.5f
	};

	const float_v3 center = float_v3
	{
		position.x + ( aabbMin.x + aabbMax.x ) * 0.5f,
		position.y + ( aabbMin.y + aabbMax.y ) * 0.5f,
		position.z + ( aabbMin.z + aabbMax.z ) * 0.5f
	};

	const float_v3 e0 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
	const float_v3 e1 = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
	const float_v3 e2 = { v0.x - v2.x, v0.y - v2.y, v0.z - v2.z };

	float tEnter = 0.0f;
	float tExit = 1.0f;
	float_v3 bestAxis = float_v3 { 0.0f, 0.0f, 1.0f };

	// Triangle normal
	const float_v3 triangleNormal = float_v3_cross( e0, e1 );
	if( float_v3_dot( triangleNormal, triangleNormal ) < EPSILON_FLOAT ) { return false; }

	const float_v3 axes[13] =
	{
		triangleNormal,
		float_v3 { 1.0f, 0.0f, 0.0f },
		float_v3 { 0.0f, 1.0f, 0.0f },
		float_v3 { 0.0f, 0.0f, 1.0f },
		float_v3_cross( float_v3 { 1.0f ,0.0f, 0.0f }, e0 ),
		float_v3_cross( float_v3 { 1.0f, 0.0f, 0.0f }, e1 ),
		float_v3_cross( float_v3 { 1.0f, 0.0f, 0.0f }, e2 ),
		float_v3_cross( float_v3 { 0.0f ,1.0f, 0.0f }, e0 ),
		float_v3_cross( float_v3 { 0.0f, 1.0f, 0.0f }, e1 ),
		float_v3_cross( float_v3 { 0.0f, 1.0f, 0.0f }, e2 ),
		float_v3_cross( float_v3 { 0.0f ,0.0f, 1.0f }, e0 ),
		float_v3_cross( float_v3 { 0.0f, 0.0f, 1.0f }, e1 ),
		float_v3_cross( float_v3 { 0.0f, 0.0f, 1.0f }, e2 )
	};

	for( int i = 0; i < 13; ++i )
	{
		float_v3 axis = axes[i];
		const float lenSqr = float_v3_dot( axis, axis );
		if( lenSqr < EPSILON_FLOAT ) { continue; }
		axis = float_v3_normalize( axis );

		float triMin, triMax;
		aabb_sweep_util_project_triangle( axis, v0, v1, v2, triMin, triMax );

		float boxMin, boxMax;
		aabb_sweep_util_project_swept_aabb( axis, center, extents, boxMin, boxMax );

		float vel = float_v3_dot( velocity, axis );

		float axisEnter, axisExit;
		if( !aabb_sweep_util_sweep_intervals( boxMin, boxMax, triMin, triMax, vel, axisEnter, axisExit ) )
		{
			return false;
		}

		if( axisEnter > tEnter )
		{
			tEnter = axisEnter;
			bestAxis = axis;
		}

		if( axisExit < tExit )
		{
			tExit = axisExit;
		}

		if( tEnter > tExit )
		{
			return false;
		}
	}

	if( tEnter < 0.0f ) { tEnter = 0.0f; }
	if( tEnter > 1.0f ) { return false; }

	hit.time = tEnter;
	hit.position = float_v3
	{
		position.x + velocity.x * tEnter,
		position.y + velocity.y * tEnter,
		position.z + velocity.z * tEnter
	};

	if( float_v3_dot( bestAxis, velocity ) > 0.0f )
	{
		bestAxis.x = -bestAxis.x;
		bestAxis.y = -bestAxis.y;
		bestAxis.z = -bestAxis.z;
	}

	hit.normal = bestAxis;
	return true;
}


bool aabb_sweep_triangle( const double_v3 &aabbMin, const double_v3 &aabbMax,
	const double_v3 &position, const double_v3 &velocity,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2,
	AABBSweepHitDouble &hit )
{
	const double_v3 extents = double_v3
	{
		( aabbMax.x - aabbMin.x ) * 0.5,
		( aabbMax.y - aabbMin.y ) * 0.5,
		( aabbMax.z - aabbMin.z ) * 0.5
	};

	const double_v3 center = double_v3
	{
		position.x + ( aabbMin.x + aabbMax.x ) * 0.5,
		position.y + ( aabbMin.y + aabbMax.y ) * 0.5,
		position.z + ( aabbMin.z + aabbMax.z ) * 0.5
	};

	const double_v3 e0 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
	const double_v3 e1 = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
	const double_v3 e2 = { v0.x - v2.x, v0.y - v2.y, v0.z - v2.z };

	double tEnter = 0.0;
	double tExit = 1.0;
	double_v3 bestAxis = double_v3 { 0.0, 0.0, 1.0 };

	// Triangle normal
	const double_v3 triangleNormal = double_v3_cross( e0, e1 );
	if( double_v3_dot( triangleNormal, triangleNormal ) < EPSILON_DOUBLE ) { return false; }

	const double_v3 axes[13] =
	{
		triangleNormal,
		double_v3 { 1.0, 0.0, 0.0 },
		double_v3 { 0.0, 1.0, 0.0 },
		double_v3 { 0.0, 0.0, 1.0 },
		double_v3_cross( double_v3 { 1.0 ,0.0, 0.0 }, e0 ),
		double_v3_cross( double_v3 { 1.0, 0.0, 0.0 }, e1 ),
		double_v3_cross( double_v3 { 1.0, 0.0, 0.0 }, e2 ),
		double_v3_cross( double_v3 { 0.0 ,1.0, 0.0 }, e0 ),
		double_v3_cross( double_v3 { 0.0, 1.0, 0.0 }, e1 ),
		double_v3_cross( double_v3 { 0.0, 1.0, 0.0 }, e2 ),
		double_v3_cross( double_v3 { 0.0 ,0.0, 1.0 }, e0 ),
		double_v3_cross( double_v3 { 0.0, 0.0, 1.0 }, e1 ),
		double_v3_cross( double_v3 { 0.0, 0.0, 1.0 }, e2 )
	};

	for( int i = 0; i < 13; ++i )
	{
		double_v3 axis = axes[i];
		const double lenSqr = double_v3_dot( axis, axis );
		if( lenSqr < EPSILON_DOUBLE ) { continue; }
		axis = double_v3_normalize( axis );

		double triMin, triMax;
		aabb_sweep_util_project_triangle( axis, v0, v1, v2, triMin, triMax );

		double boxMin, boxMax;
		aabb_sweep_util_project_swept_aabb( axis, center, extents, boxMin, boxMax );

		double vel = double_v3_dot( velocity, axis );

		double axisEnter, axisExit;
		if( !aabb_sweep_util_sweep_intervals( boxMin, boxMax, triMin, triMax, vel, axisEnter, axisExit ) )
		{
			return false;
		}

		if( axisEnter > tEnter )
		{
			tEnter = axisEnter;
			bestAxis = axis;
		}

		if( axisExit < tExit )
		{
			tExit = axisExit;
		}

		if( tEnter > tExit )
		{
			return false;
		}
	}

	if( tEnter < 0.0 ) { tEnter = 0.0; }
	if( tEnter > 1.0 ) { return false; }

	hit.time = tEnter;
	hit.position = double_v3
	{
		position.x + velocity.x * tEnter,
		position.y + velocity.y * tEnter,
		position.z + velocity.z * tEnter
	};

	if( double_v3_dot( bestAxis, velocity ) > 0.0 )
	{
		bestAxis.x = -bestAxis.x;
		bestAxis.y = -bestAxis.y;
		bestAxis.z = -bestAxis.z;
	}

	hit.normal = bestAxis;
	return true;
}


bool aabb_sweep_triangle_list( const float_v3 &aabbMin, const float_v3 &aabbMax,
	const float_v3 &position, const float_v3 &delta, const float_v3 *verts, usize count,
	AABBSweepHitFloat& hit )
{
	hit.time = FLOAT_MAX;
	AABBSweepHitFloat temp;

	Assert( count % 3 == 0 );
	for( usize i = 0; i + 2 < count; i += 3 )
	{
		if( aabb_sweep_triangle( aabbMin, aabbMax, position, delta, verts[i], verts[i + 1], verts[i + 2], temp ) )
		{
			if( temp.time < hit.time )
			{
				hit = temp;
				hit.triangleID = i / 3;
			}
		}
	}

	return hit.time < FLOAT_MAX;
}


bool aabb_sweep_triangle_list( const double_v3 &aabbMin, const double_v3 &aabbMax,
	const double_v3 &position, const double_v3 &delta, const double_v3 *verts, usize count,
	AABBSweepHitDouble& hit )
{
	hit.time = FLOAT_MAX;
	AABBSweepHitDouble temp;

	Assert( count % 3 == 0 );
	for( usize i = 0; i + 2 < count; i += 3 )
	{
		if( aabb_sweep_triangle( aabbMin, aabbMax, position, delta, verts[i], verts[i + 1], verts[i + 2], temp ) )
		{
			if( temp.time < hit.time )
			{
				hit = temp;
				hit.triangleID = i / 3;
			}
		}
	}

	return hit.time < FLOAT_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool sphere_sweep_util_point_in_triangle( const float_v3 &point, const float_v3 &vert0,
	const float_v3 &vert1, const float_v3 &vert2 )
{
	const float_v3 v0v1 = vert1 - vert0;
	const float_v3 v0v2 = vert2 - vert0;
	const float_v3 v0p0 = point - vert0;

	const float dot00 = v0v1.dot( v0v1 );
	const float dot01 = v0v1.dot( v0v2 );
	const float dot11 = v0v2.dot( v0v2 );

	const float denom = dot00 * dot11 - dot01 * dot01;

	if( fabs( denom ) < EPSILON_FLOAT )
	{
		return ( point - vert0 ).length_sqr() < EPSILON_FLOAT ||
			( point - vert1 ).length_sqr() < EPSILON_FLOAT ||
			( point - vert2 ).length_sqr() < EPSILON_FLOAT;
	}

	const float dot02 = v0v1.dot( v0p0 );
	const float dot12 = v0v2.dot( v0p0 );
	const float invDenom = 1.0f / denom;

	const float u = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
	const float v = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;

	return ( u >= -EPSILON_FLOAT ) && ( v >= -EPSILON_FLOAT ) && ( u + v <= 1.0f + EPSILON_FLOAT );
}


static bool sphere_sweep_util_point_in_triangle( const double_v3 &point, const double_v3 &vert0,
	const double_v3 &vert1, const double_v3 &vert2 )
{
	const double_v3 v0v1 = vert1 - vert0;
	const double_v3 v0v2 = vert2 - vert0;
	const double_v3 v0p0 = point - vert0;

	const double dot00 = v0v1.dot( v0v1 );
	const double dot01 = v0v1.dot( v0v2 );
	const double dot11 = v0v2.dot( v0v2 );

	const double denom = dot00 * dot11 - dot01 * dot01;

	if( fabs( denom ) < EPSILON_DOUBLE )
	{
		return ( point - vert0 ).length_sqr() < EPSILON_DOUBLE ||
			( point - vert1 ).length_sqr() < EPSILON_DOUBLE ||
			( point - vert2 ).length_sqr() < EPSILON_DOUBLE;
	}

	const double dot02 = v0v1.dot( v0p0 );
	const double dot12 = v0v2.dot( v0p0 );
	const double invDenom = 1.0 / denom;

	const double u = ( dot11 * dot02 - dot01 * dot12 ) * invDenom;
	const double v = ( dot00 * dot12 - dot01 * dot02 ) * invDenom;

	return ( u >= -EPSILON_DOUBLE ) && ( v >= -EPSILON_DOUBLE ) && ( u + v <= 1.0 + EPSILON_DOUBLE );
}


static float_v3 sphere_sweep_util_closest_point_on_triangle( const float_v3 &point, const float_v3 &v0,
	const float_v3 &v1, const float_v3 &v2 )
{
	const float_v3 v0v1 = v1 - v0;
	const float_v3 v0v2 = v2 - v0;
	const float_v3 v0p0 = point - v0;

	const float d1 = v0v1.dot( v0p0 );
	const float d2 = v0v2.dot( v0p0 );

	if( d1 <= 0.0f && d2 <= 0.0f ) { return v0; }

	const float_v3 bp = point - v1;
	const float d3 = v0v1.dot( bp );
	const float d4 = v0v2.dot( bp );

	if( d3 >= 0.0f && d4 <= d3 ) { return v1; }

	const float_v3 cp = point - v2;
	const float d5 = v0v1.dot( cp );
	const float d6 = v0v2.dot( cp );

	if( d6 >= 0.0f && d5 <= d6 ) { return v2; }

	const float vc = d1 * d4 - d3 * d2;
	if( vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f )
	{
		const float v = d1 / ( d1 - d3 );
		return v0 + v0v1 * v;
	}

	const float vb = d5 * d2 - d1 * d6;
	if( vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f )
	{
		const float w = d2 / ( d2 - d6 );
		return v0 + v0v2 * w;
	}

	const float va = d3 * d6 - d5 * d4;
	if( va <= 0.0f && ( d4 - d3 ) >= 0.0f && ( d5 - d6 ) >= 0.0f )
	{
		const float w = ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) );
		return v1 + ( v2 - v1 ) * w;
	}

	const float denom = 1.0f / ( va + vb + vc );
	const float v = vb * denom;
	const float w = vc * denom;
	return v0 + v0v1 * v + v0v2 * w;
}


static double_v3 sphere_sweep_util_closest_point_on_triangle( const double_v3 &point, const double_v3 &v0,
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


static bool sphere_sweep_util_solve_quadratic( float a, float b, float c, float max_t, float &t )
{
	if( fabsf( a ) < EPSILON_FLOAT )
	{
		if( fabsf( b ) < EPSILON_FLOAT ) { return false; }
		t = -c / b;
		return t >= 0.0f && t <= max_t;
	}

	const float discriminant = b * b - 4 * a * c;
	if( discriminant < 0.0f ) { return false; }

	const float discriminantSqrt = sqrtf( discriminant );
	float t1 = ( -b - discriminantSqrt ) / ( 2.0f * a );
	float t2 = ( -b + discriminantSqrt ) / ( 2.0f * a );

	if( t1 > t2 )
	{
		const float tmp = t1;
		t1 = t2; t2 = tmp;
	}

	if( t1 >= 0.0f && t1 <= max_t )
	{
		t = t1;
		return true;
	}

	if( t2 >= 0.0f && t2 <= max_t )
	{
		t = t2;
		return true;
	}

	return false;
}


static bool sphere_sweep_util_solve_quadratic( double a, double b, double c, double max_t, double &t )
{
	if( fabs( a ) < EPSILON_DOUBLE )
	{
		if( fabs( b ) < EPSILON_DOUBLE ) { return false; }
		t = -c / b;
		return t >= 0.0 && t <= max_t;
	}

	const double discriminant = b * b - 4 * a * c;
	if( discriminant < 0.0 ) { return false; }

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

	if( t2 >= 0.0 && t2 <= max_t )
	{
		t = t2;
		return true;
	}

	return false;
}


SphereSweepHitFloat sphere_sweep_triangle_list( const float_v3 &center, float radius,
	const float_v3 &velocity, const float_v3 *verts, usize count )
{
	const float radiusSqr = radius * radius;

	SphereSweepHitFloat result;
	result.time = 1.0f;
	result.hit = false;

	// Early out if no displacement
	if( velocity.length_sqr() < EPSILON_FLOAT )
	{
		// Check for static collision
		for( usize i = 0; i < count; i += 3 )
		{
			const float_v3 &v0 = verts[i + 0];
			const float_v3 &v1 = verts[i + 1];
			const float_v3 &v2 = verts[i + 2];

			const float_v3 pointClosest = sphere_sweep_util_closest_point_on_triangle( center, v0, v1, v2 );
			const float distanceSqr = ( center - pointClosest ).length_sqr();

			if( distanceSqr <= radiusSqr + EPSILON_FLOAT )
			{
				result.hit = true;
				result.time = 0.0f;
				result.position = pointClosest;
				result.normal = ( center - pointClosest ).normalize();
				return result;
			}
		}
		return result;
	}

	// Iterate through all triangles
	for( usize i = 0; i < count; i += 3 )
	{
		const float_v3 &v0 = verts[i + 0];
		const float_v3 &v1 = verts[i + 1];
		const float_v3 &v2 = verts[i + 2];

		// Compute triangle normal
		const float_v3 edge1 = v1 - v0;
		const float_v3 edge2 = v2 - v0;
		const float_v3 normal = float_v3_cross( edge1, edge2 ).normalize();
#if 0
		const float areaSqr = normal.length_sqr();
		if( UNLIKELY( areaSqr < EPSILON_FLOAT ) ) { continue; } // Degenerate triangle
#endif

		// Check if moving toward the triangle
		const float dotMovement = velocity.dot( normal );
		if( dotMovement > -EPSILON_FLOAT ) { continue; }

		// Calculate distance from sphere center to triangle plane
		const float distanceToTriangle = ( center - v0 ).dot( normal );
		const float distanceToTriangleMinusRadius = ( distanceToTriangle - radius );
		const float distanceToTriangleMinusRadiusSqr = distanceToTriangleMinusRadius * distanceToTriangleMinusRadius;
		if( distanceToTriangle > radius && distanceToTriangleMinusRadiusSqr > velocity.length_sqr() ) { continue; }

		// Calculate time of intersection with the expanded plane (considering sphere radius)
		const float t = clamp( ( radius - distanceToTriangle ) / dotMovement, 0.0f, 1.0f );
		if( t >= result.time ) { continue; }

		// Calculate sphere center at time t & closest point on triangle to that position
		const float_v3 centerAtTime = center + velocity * t;
		const float_v3 pointClosest = sphere_sweep_util_closest_point_on_triangle( centerAtTime, v0, v1, v2 );
		const float distanceSqr = ( centerAtTime - pointClosest ).length_sqr();

		// Check if sphere touches or intersects the triangle at time t
		if( distanceSqr <= radiusSqr + EPSILON_FLOAT )
		{
			result.hit = true;
			result.time = t;
			result.position = pointClosest;
			result.normal = ( centerAtTime - pointClosest ).normalize();
			if( t <= EPSILON_FLOAT ) { return result; } // Early out if t = 0.0 collision
		}
	}

	return result;
}


SphereSweepHitDouble sphere_sweep_triangle_list( const double_v3 &center, double radius,
	const double_v3 &velocity, const double_v3 *verts, usize count )
{
	const double radiusSqr = radius * radius;

	SphereSweepHitDouble result;
	result.time = 1.0;
	result.hit = false;

	// Early out if no displacement
	if( velocity.length_sqr() < EPSILON_DOUBLE )
	{
		// Check for static collision
		for( usize i = 0; i < count; i += 3 )
		{
			const double_v3 &v0 = verts[i + 0];
			const double_v3 &v1 = verts[i + 1];
			const double_v3 &v2 = verts[i + 2];

			const double_v3 pointClosest = sphere_sweep_util_closest_point_on_triangle( center, v0, v1, v2 );
			const double distanceSqr = ( center - pointClosest ).length_sqr();

			if( distanceSqr <= radiusSqr + EPSILON_DOUBLE )
			{
				result.hit = true;
				result.time = 0.0;
				result.position = pointClosest;
				result.normal = ( center - pointClosest ).normalize();
				return result;
			}
		}
		return result;
	}

	// Iterate through all triangles
	for( usize i = 0; i < count; i += 3 )
	{
		const double_v3 &v0 = verts[i + 0];
		const double_v3 &v1 = verts[i + 1];
		const double_v3 &v2 = verts[i + 2];

		// Compute triangle normal
		const double_v3 edge1 = v1 - v0;
		const double_v3 edge2 = v2 - v0;
		const double_v3 normal = double_v3_cross( edge1, edge2 ).normalize();
#if 0
		const double areaSqr = normal.length_sqr();
		if( UNLIKELY( areaSqr < EPSILON_DOUBLE ) ) { continue; } // Degenerate triangle
#endif

		// Check if moving toward the triangle
		const double dotMovement = velocity.dot( normal );
		if( dotMovement > -EPSILON_DOUBLE ) { continue; }

		// Calculate distance from sphere center to triangle plane
		const double distanceToTriangle = ( center - v0 ).dot( normal );
		const double distanceToTriangleMinusRadius = ( distanceToTriangle - radius );
		const double distanceToTriangleMinusRadiusSqr = distanceToTriangleMinusRadius * distanceToTriangleMinusRadius;
		if( distanceToTriangle > radius && distanceToTriangleMinusRadiusSqr > velocity.length_sqr() ) { continue; }

		// Calculate time of intersection with the expanded plane (considering sphere radius)
		const double t = clamp( ( radius - distanceToTriangle ) / dotMovement, 0.0, 1.0 );
		if( t >= result.time ) { continue; }

		// Calculate sphere center at time t & closest point on triangle to that position
		const double_v3 centerAtTime = center + velocity * t;
		const double_v3 pointClosest = sphere_sweep_util_closest_point_on_triangle( centerAtTime, v0, v1, v2 );
		const double distanceSqr = ( centerAtTime - pointClosest ).length_sqr();

		// Check if sphere touches or intersects the triangle at time t
		if( distanceSqr <= radiusSqr + EPSILON_DOUBLE )
		{
			result.hit = true;
			result.time = t;
			result.position = pointClosest;
			result.normal = ( centerAtTime - pointClosest ).normalize();
			if( t <= EPSILON_DOUBLE ) { return result; } // Early out if t = 0.0 collision
		}
	}

	return result;
}



u32 sphere_generate_geometry_icosahedron( u32 subdivisions, List<float_v3> *positions,
	List<float_v3> *normals, List<float_v2> *uvs, List<u32> *indices )
{
	const bool generatePositions = positions != nullptr;
	Assert( !generatePositions || positions->is_initialized() );
	const bool generateNormals = normals != nullptr;
	Assert( !generateNormals || normals->is_initialized() );
	const bool generateUVs = uvs != nullptr;
	Assert( !generateUVs || uvs->is_initialized() );
	const bool generateIndices = indices != nullptr;
	Assert( !generateIndices || indices->is_initialized() );

	if( UNLIKELY( !generatePositions && !generateNormals && !generateUVs && !generateIndices ) ) { return 0; }

	subdivisions = clamp( subdivisions, 0U, 8U );

	const float X = 0.525731112119133606f; // Golden ratio
	const float Z = 0.850650808352039932f; // Golden ratio

	const float basePositions[12][3] =
	{
		{ -X, 0, Z }, {  X, 0,  Z }, { -X,  0, -Z }, {  X,  0, -Z },
		{  0, Z, X }, {  0, Z, -X }, {  0, -Z,  X }, {  0, -Z, -X },
		{  Z, X, 0 }, { -Z, X,  0 }, {  Z, -X,  0 }, { -Z, -X,  0 }
	};

	const int baseIndices[20][3] =
	{
		{ 0x0, 0x4, 0x1 }, { 0x0, 0x9, 0x4 }, { 0x9, 0x5, 0x4 }, { 0x4, 0x5, 0x8 }, { 0x4, 0x8, 0x1 },
		{ 0x8, 0xA, 0x1 }, { 0x8, 0x3, 0xA }, { 0x5, 0x3, 0x8 }, { 0x5, 0x2, 0x3 }, { 0x2, 0x7, 0x3 },
		{ 0x7, 0xA, 0x3 }, { 0x7, 0x6, 0xA }, { 0x7, 0xB, 0x6 }, { 0xB, 0x0, 0x6 }, { 0x0, 0x1, 0x6 },
		{ 0x6, 0x1, 0xA }, { 0x9, 0x0, 0xB }, { 0x9, 0xB, 0x2 }, { 0x9, 0x2, 0x5 }, { 0x7, 0x2, 0xB }
	};

	// Helper: Compute UV
	auto compute_uv = [&]( const float_v3 &position ) -> float_v2
	{
		float_v2 uv;
		uv.x = ( atan2f( position.y, position.x ) + PI_F)  / ( 2.0f * PI_F );
		uv.y = acosf( position.z ) / PI_F;
		return uv;
	};

	// Helper: Approximate hashmap size
	auto hashmap_reserve = [&]( const u32 subdivisions ) -> u32
	{
		u32 reserve = 30;
		for( u32 i = 0; i < subdivisions; i++ ) { reserve = reserve * 2 + 20 * 3 * ( 1 << ( 2 * i ) ); }
		return reserve;
	};

	// Temporary data structures
	List<float_v3> _xyz;
	List<float_v3> *xyz = nullptr;
	if( generatePositions ) { xyz = positions; } else { _xyz.init(); xyz = &_xyz; }
	HashMap<SphereGeometry::EdgeKey, u32> midpoints;
	if( generateIndices ) { midpoints.init( hashmap_reserve( subdivisions ) ); }

	// Initial Icosahedron
	if( generateIndices )
	{
		// Vertices (Indexed)
		for( int i = 0; i < 12; i++ )
		{
			const float_v3 v0 = float_v3 { basePositions[i][0], basePositions[i][1],
				basePositions[i][2] }.normalize();

			xyz->add( v0 );
			if( generateNormals ) { normals->add( v0 ); }
			if( generateUVs ) { uvs->add( compute_uv( v0 ) ); }
		}

		// Indices
		for( int i = 0; i < 20; i++ )
		{
			indices->add( baseIndices[i][0] );
			indices->add( baseIndices[i][1] );
			indices->add( baseIndices[i][2] );
		}
	}
	else
	{
		// Vertices (Non-Indexed)
		for( int i = 0; i < 20; i++ )
		{
			const int *triangle = baseIndices[i];

			const float_v3 v0 = float_v3 { basePositions[triangle[0]][0], basePositions[triangle[0]][1],
				basePositions[triangle[0]][2] }.normalize();
			const float_v3 v1 = float_v3 { basePositions[triangle[1]][0], basePositions[triangle[1]][1],
				basePositions[triangle[1]][2] }.normalize();
			const float_v3 v2 = float_v3 { basePositions[triangle[2]][0], basePositions[triangle[2]][1],
				basePositions[triangle[2]][2] }.normalize();

			xyz->add( v0 );
			xyz->add( v1 );
			xyz->add( v2 );

			if( generateNormals )
			{
				normals->add( v0 );
				normals->add( v1 );
				normals->add( v2 );
			}

			if( generateUVs )
			{
				uvs->add( compute_uv( v0 ) );
				uvs->add( compute_uv( v1 ) );
				uvs->add( compute_uv( v2 ) );
			}
		}
	}

	// Subdivide Icosahedron
	for( u32 s = 0; s < subdivisions; s++ )
	{
		if( generateIndices )
		{
			List<u32> indicesSubdivided;
			indicesSubdivided.init( indices->count() * 4 );

			auto split_edge = [&]( const u32 i1, const u32 i2 ) -> u32
			{
				SphereGeometry::EdgeKey key { i1, i2 };
				if( midpoints.contains( key ) ) { return midpoints.get( key ); }

				const u32 index = static_cast<u32>( xyz->count() );
				midpoints.set( key, index );

				const float_v3 position = float_v3 { ( xyz->at( i1 ) + xyz->at( i2 ) ) * 0.5f }.normalize();
				xyz->add( position );
				if( generateNormals ) { normals->add( position ); }
				if( generateUVs ) { uvs->add( compute_uv( position ) ); }

				return index;
			};

			for( usize i = 0; i + 2 < indices->count(); i += 3 )
			{
				const u32 i1 = indices->at( i + 0 );
				const u32 i2 = indices->at( i + 1 );
				const u32 i3 = indices->at( i + 2 );
				const u32 a = split_edge( i1, i2 );
				const u32 b = split_edge( i2, i3 );
				const u32 c = split_edge( i3, i1 );
				indicesSubdivided.add( i1 );
				indicesSubdivided.add( a );
				indicesSubdivided.add( c );
				indicesSubdivided.add( i2 );
				indicesSubdivided.add( b );
				indicesSubdivided.add( a );
				indicesSubdivided.add( i3 );
				indicesSubdivided.add( c );
				indicesSubdivided.add( b );
				indicesSubdivided.add( a );
				indicesSubdivided.add( b );
				indicesSubdivided.add( c );
			}

			indices->move( static_cast<List<u32> &&>( indicesSubdivided ) );
		}
		else
		{
			List<float_v3> xyzSubdivided;
			List<float_v3> normalsSubdivided;
			List<float_v2> uvsSubdivided;
			xyzSubdivided.init( xyz->count() * 4 );
			if( generateNormals ) { normalsSubdivided.init( normals->count() * 4 ); }
			if( generateUVs ) { uvsSubdivided.init( uvs->count() * 4 ); }

			auto add_triangle = [&]( const float_v3 &p0, const float_v3 &p1, const float_v3 &p2 )
			{
				xyzSubdivided.add( p0 );
				xyzSubdivided.add( p1 );
				xyzSubdivided.add( p2 );

				if( generateNormals )
				{
					normalsSubdivided.add( p0 );
					normalsSubdivided.add( p1 );
					normalsSubdivided.add( p2 );
				}

				if( generateUVs )
				{
					uvsSubdivided.add( compute_uv( p0 ) );
					uvsSubdivided.add( compute_uv( p1 ) );
					uvsSubdivided.add( compute_uv( p2 ) );
				}
			};

			for( usize i = 0; i + 2 < xyz->count(); i += 3 )
			{
				const float_v3 &v0 = xyz->at( i + 0 );
				const float_v3 &v1 = xyz->at( i + 1 );
				const float_v3 &v2 = xyz->at( i + 2 );

				const u32 a = static_cast<u32>( xyzSubdivided.count() );
				const u32 b = a + 1;
				const u32 c = a + 2;

				const float_v3 m0 = float_v3 { ( v0 + v1 ) * 0.5f }.normalize();
				const float_v3 m1 = float_v3 { ( v1 + v2 ) * 0.5f }.normalize();
				const float_v3 m2 = float_v3 { ( v2 + v0 ) * 0.5f }.normalize();

				add_triangle( v0, m0, m2 );
				add_triangle( v1, m1, m0 );
				add_triangle( v2, m2, m1 );
				add_triangle( m0, m1, m2 );
			}

			xyz->move( static_cast<List<float_v3> &&>( xyzSubdivided ) );
			if( generateNormals ) { normals->move( static_cast<List<float_v3> &&>( normalsSubdivided ) ); }
			if( generateUVs ) { uvs->move( static_cast<List<float_v2> &&>( uvsSubdivided ) ); }
		}
	}

	// Free temporary data structures
	const u32 vertexCount = xyz->count();
	if( !generatePositions ) { _xyz.free(); }
	if( generateIndices ) { midpoints.free(); }

	return vertexCount;
}


u32 sphere_generate_geometry_latlon( u32 resolution, List<float_v3> *positions,
	List<float_v3> *normals, List<float_v2> *uvs, List<u32> *indices )
{
	const bool generatePositions = positions != nullptr;
	Assert( !generatePositions || positions->is_initialized() );
	const bool generateNormals = normals != nullptr;
	Assert( !generateNormals || normals->is_initialized() );
	const bool generateUVs = uvs != nullptr;
	Assert( !generateUVs || uvs->is_initialized() );
	const bool generateIndices = indices != nullptr;
	Assert( !generateIndices || indices->is_initialized() );

	if( UNLIKELY( !generatePositions && !generateNormals && !generateUVs && !generateIndices ) ) { return 0; }

	resolution = max( resolution, 3u );
	const u32 lonCount = resolution * 2;
	const u32 latCount = resolution;
	const float lonCountInv = 1.0f / lonCount;
	const float latCountInv = 1.0f / latCount;

	// Temporary data structures
	List<float_v3> _xyz;
	List<float_v3> *xyz = nullptr;
	if( generatePositions ) { xyz = positions; } else { _xyz.init(); xyz = &_xyz; }

	if( generateIndices )
	{
		const u32 vertexCount = ( latCount + 1 ) * ( lonCount + 1 );
		xyz->reserve( vertexCount );
		if( generateNormals ) { normals->reserve( vertexCount ); }
		if( generateUVs ) { uvs->reserve( vertexCount ); }

		// Generate vertices
		for( u32 lat = 0; lat <= latCount; ++lat )
		{
			// Latitude angle from 0 to PI
			const float latAngle = lat * latCountInv * PI_F;
			const float z = cosf( latAngle ); // Z ranges from 1 (north) to -1 (south)
			const float radius = sinf( latAngle );

			for( u32 lon = 0; lon <= lonCount; ++lon )
			{
				// Longitude angle from 0 to 2 * PI
				const float lonAngle = lon * lonCountInv * 2.0f * PI_F;
				const float x = radius * cosf( lonAngle );
				const float y = radius * sinf( lonAngle );

				const float_v3 position = float_v3 { x, y, z };
				xyz->add( position );

				if( generateNormals )
				{
					normals->add( float_v3_normalize( position ) );
				}

				if( generateUVs )
				{
					uvs->add( float_v2 { lon * lonCountInv, lat * latCountInv } );
				}
			}
		}

		const u32 indexCount = latCount * lonCount * 6;
		indices->reserve( indexCount );

		// Generate indices
		for( u32 lat = 0; lat < latCount; ++lat )
		{
			for( u32 lon = 0; lon < lonCount; ++lon )
			{
				const u32 first = lat * ( lonCount + 1 ) + lon;
				const u32 second = ( lat + 1 ) * ( lonCount + 1 ) + lon;

				// Two triangles per quad
				indices->add( first );
				indices->add( second );
				indices->add( first + 1 );

				indices->add( first + 1 );
				indices->add( second );
				indices->add( second + 1 );
			}
		}
	}
	else
	{
		const u32 vertexCount = latCount * lonCount * 6;
		xyz->reserve( vertexCount );
		if( generateNormals ) { normals->reserve( vertexCount ); }
		if( generateUVs ) { uvs->reserve( vertexCount ); }

		// Generate vertices directly as triangles
		for( u32 lat = 0; lat < latCount; ++lat )
		{
			// Latitude angles
			const float latAngle0 = lat * latCountInv * PI_F;
			const float lonAngle1 = ( lat + 1 ) * latCountInv * PI_F;

			const float z0 = cosf( latAngle0 );
			const float z1 = cosf( lonAngle1 );

			const float radius0 = sinf( latAngle0 );
			const float radius1 = sinf( lonAngle1 );

			for( u32 lon = 0; lon < lonCount; ++lon )
			{
				// Longitude angles
				const u32 nextLon = ( lon + 1 ) % lonCount;
				const float lonAngle0 = lon * lonCountInv * 2.0f * PI_F;
				const float lonAngle1 = nextLon * lonCountInv * 2.0f * PI_F;

				const float_v3 v0 = float_v3 { radius0 * cosf( lonAngle0 ), radius0 * sinf( lonAngle0 ), z0 };
				const float_v3 v1 = float_v3 { radius1 * cosf( lonAngle0 ), radius1 * sinf( lonAngle0 ), z1 };
				const float_v3 v2 = float_v3 { radius0 * cosf( lonAngle1 ), radius0 * sinf( lonAngle1 ), z0 };
				const float_v3 v3 = float_v3 { radius1 * cosf( lonAngle1 ), radius1 * sinf( lonAngle1 ), z1 };

				float_v2 uv0 = float_v2 { lon * lonCountInv, lat * latCountInv };
				float_v2 uv1 = float_v2 { lon * lonCountInv, ( lat + 1 ) * latCountInv };
				float_v2 uv2 = float_v2 { nextLon * lonCountInv, lat * latCountInv };
				float_v2 uv3 = float_v2 { nextLon * lonCountInv, ( lat + 1 ) * latCountInv };

				// UV.x seam: when nextLon is 0, uv.x must be 1.0 (voids backwards interpolation)
				if( nextLon == 0 )
				{
					uv2.x = 1.0f;
					uv3.x = 1.0f;
				}

				// First triangle
				xyz->add( v0 );
				xyz->add( v1 );
				xyz->add( v2 );

				if( generateNormals )
				{
					normals->add( float_v3_normalize( v0 ) );
					normals->add( float_v3_normalize( v1 ) );
					normals->add( float_v3_normalize( v2 ) );
				}

				if( generateUVs )
				{
					uvs->add( uv0 );
					uvs->add( uv1 );
					uvs->add( uv2 );
				}

				// Second triangle
				xyz->add( v2 );
				xyz->add( v1 );
				xyz->add( v3 );

				if( generateNormals )
				{
					normals->add( float_v3_normalize( v2 ) );
					normals->add( float_v3_normalize( v1 ) );
					normals->add( float_v3_normalize( v3 ) );
				}

				if( generateUVs )
				{
					uvs->add( uv2 );
					uvs->add( uv1 );
					uvs->add( uv3 );
				}
			}
		}
	}

	// Free temporary data structures
	const u32 vertexCount = xyz->count();
	if( !generatePositions ) { _xyz.free(); }

	return vertexCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrustumFloat frustum_build( const float_m44 &matrixMVP )
{
	FrustumFloat frustum;

	auto normalize_plane = []( float a, float b, float c, float d ) -> FrustumFloat::Plane
	{
		float len = sqrtf( a * a + b * b + c * c );
		if( len == 0.0f ) { len = 1.0f; }
		const float lenInv = 1.0f / len;
		return FrustumFloat::Plane { float_v3 { a * lenInv, b * lenInv, c * lenInv }, d * lenInv };
	};

	// Extract planes (row-major)
	const float_m44 &m = matrixMVP;
	frustum.planes[FrustumPlane_Left] = normalize_plane( m[3] + m[0], m[7] + m[4], m[0xB] + m[8], m[0xF] + m[0xC] );
	frustum.planes[FrustumPlane_Right] = normalize_plane( m[3] - m[0], m[7] - m[4], m[0xB] - m[8], m[0xF] - m[0xC] );
	frustum.planes[FrustumPlane_Bottom] = normalize_plane( m[3] + m[1], m[7] + m[5], m[0xB] + m[9], m[0xF] + m[0xD] );
	frustum.planes[FrustumPlane_Top] = normalize_plane( m[3] - m[1], m[7] - m[5], m[0xB] - m[9], m[0xF] - m[0xD] );
	frustum.planes[FrustumPlane_Near] = normalize_plane( m[3] + m[2], m[7] + m[6], m[0xB] + m[0xA], m[0xF] + m[0xE] );
	frustum.planes[FrustumPlane_Far] = normalize_plane( m[3] - m[2], m[7] - m[6], m[0xB] - m[0xA], m[0xF] - m[0xE] );

	auto intersect_planes = []( const FrustumFloat::Plane &p1,
		const FrustumFloat::Plane &p2,
		const FrustumFloat::Plane &p3 ) -> float_v3
	{
		const float_v3 n1 = p1.normal;
		const float_v3 n2 = p2.normal;
		const float_v3 n3 = p3.normal;

		const float_v3 n2xn3 = { n2.y * n3.z - n2.z * n3.y, n2.z * n3.x - n2.x * n3.z, n2.x * n3.y - n2.y * n3.x };

		const float denom = n1.x * n2xn3.x + n1.y * n2xn3.y + n1.z * n2xn3.z;
		if( fabsf( denom ) < EPSILON_FLOAT ) { return float_v3 { 0.0f, 0.0f, 0.0f }; }
		const float denomInv = 1.0f / denom;

		const float_v3 term1 = { -p1.distance * n2xn3.x, -p1.distance * n2xn3.y, -p1.distance * n2xn3.z };
		const float_v3 n3xn1 = { n3.y*n1.z - n3.z*n1.y, n3.z*n1.x - n3.x*n1.z, n3.x*n1.y - n3.y*n1.x };
		const float_v3 term2 = { -p2.distance * n3xn1.x, -p2.distance * n3xn1.y, -p2.distance * n3xn1.z };
		const float_v3 n1xn2 = { n1.y * n2.z - n1.z * n2.y, n1.z * n2.x - n1.x * n2.z, n1.x * n2.y - n1.y * n2.x };
		const float_v3 term3 = { -p3.distance * n1xn2.x, -p3.distance * n1xn2.y, -p3.distance * n1xn2.z };

		return float_v3
		{
			( term1.x + term2.x + term3.x ) * denomInv,
			( term1.y + term2.y + term3.y ) * denomInv,
			( term1.z + term2.z + term3.z ) * denomInv
		};
	};

	frustum.corners[FrustumCorner_NearTopLeft] = intersect_planes(frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_NearTopRight] = intersect_planes(frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Right] );
	frustum.corners[FrustumCorner_NearBottomLeft] = intersect_planes( frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_NearBottomRight] = intersect_planes( frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Right] );

	frustum.corners[FrustumCorner_FarTopLeft] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_FarTopRight] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Right] );
	frustum.corners[FrustumCorner_FarBottomLeft] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_FarBottomRight] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Right] );

	return frustum;
}


FrustumDouble frustum_build( const double_m44 &matrixMVP )
{
	FrustumDouble frustum;

	auto normalize_plane = []( double a, double b, double c, double d ) -> FrustumDouble::Plane
	{
		double len = sqrt( a * a + b * b + c * c );
		if( len == 0.0 ) { len = 1.0; }
		const double lenInv = 1.0 / len;
		return FrustumDouble::Plane { double_v3 { a * lenInv, b * lenInv, c * lenInv }, d * lenInv };
	};

	// Extract planes (row-major)
	const double_m44 &m = matrixMVP;
	frustum.planes[FrustumPlane_Left] = normalize_plane( m[3] + m[0], m[7] + m[4], m[0xB] + m[8], m[0xF] + m[0xC] );
	frustum.planes[FrustumPlane_Right] = normalize_plane( m[3] - m[0], m[7] - m[4], m[0xB] - m[8], m[0xF] - m[0xC] );
	frustum.planes[FrustumPlane_Bottom] = normalize_plane( m[3] + m[1], m[7] + m[5], m[0xB] + m[9], m[0xF] + m[0xD] );
	frustum.planes[FrustumPlane_Top] = normalize_plane( m[3] - m[1], m[7] - m[5], m[0xB] - m[9], m[0xF] - m[0xD] );
	frustum.planes[FrustumPlane_Near] = normalize_plane( m[3] + m[2], m[7] + m[6], m[0xB] + m[0xA], m[0xF] + m[0xE] );
	frustum.planes[FrustumPlane_Far] = normalize_plane( m[3] - m[2], m[7] - m[6], m[0xB] - m[0xA], m[0xF] - m[0xE] );

	auto intersect_planes = []( const FrustumDouble::Plane &p1,
		const FrustumDouble::Plane &p2,
		const FrustumDouble::Plane &p3 ) -> double_v3
	{
		const double_v3 n1 = p1.normal;
		const double_v3 n2 = p2.normal;
		const double_v3 n3 = p3.normal;

		const double_v3 n2xn3 = { n2.y * n3.z - n2.z * n3.y, n2.z * n3.x - n2.x * n3.z, n2.x * n3.y - n2.y * n3.x };

		const double denom = n1.x * n2xn3.x + n1.y * n2xn3.y + n1.z * n2xn3.z;
		if( fabs( denom ) < EPSILON_DOUBLE ) { return double_v3 { 0.0, 0.0, 0.0 }; }
		const double denomInv = 1.0 / denom;

		const double_v3 term1 = { -p1.distance * n2xn3.x, -p1.distance * n2xn3.y, -p1.distance * n2xn3.z };
		const double_v3 n3xn1 = { n3.y*n1.z - n3.z*n1.y, n3.z*n1.x - n3.x*n1.z, n3.x*n1.y - n3.y*n1.x };
		const double_v3 term2 = { -p2.distance * n3xn1.x, -p2.distance * n3xn1.y, -p2.distance * n3xn1.z };
		const double_v3 n1xn2 = { n1.y * n2.z - n1.z * n2.y, n1.z * n2.x - n1.x * n2.z, n1.x * n2.y - n1.y * n2.x };
		const double_v3 term3 = { -p3.distance * n1xn2.x, -p3.distance * n1xn2.y, -p3.distance * n1xn2.z };

		return double_v3
		{
			( term1.x + term2.x + term3.x ) * denomInv,
			( term1.y + term2.y + term3.y ) * denomInv,
			( term1.z + term2.z + term3.z ) * denomInv
		};
	};

	frustum.corners[FrustumCorner_NearTopLeft] = intersect_planes(frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_NearTopRight] = intersect_planes(frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Right] );
	frustum.corners[FrustumCorner_NearBottomLeft] = intersect_planes( frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_NearBottomRight] = intersect_planes( frustum.planes[FrustumPlane_Near],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Right] );

	frustum.corners[FrustumCorner_FarTopLeft] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_FarTopRight] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Top], frustum.planes[FrustumPlane_Right] );
	frustum.corners[FrustumCorner_FarBottomLeft] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Left] );
	frustum.corners[FrustumCorner_FarBottomRight] = intersect_planes(frustum.planes[FrustumPlane_Far],
		frustum.planes[FrustumPlane_Bottom], frustum.planes[FrustumPlane_Right] );

	return frustum;
}


FrustumFloat frustum_build( const float_m44 &matrixView, const float_m44 &matrixProjection )
{
	return frustum_build( float_m44_multiply( matrixProjection, matrixView ) );
}


FrustumDouble frustum_build( const double_m44 &matrixView, const double_m44 &matrixProjection )
{
	return frustum_build( double_m44_multiply( matrixProjection, matrixView ) );
}


FrustumFloat frustum_build( const float_m44 &matrixView, const float_m44 &matrixProjection,
	float _near, float _far )
{
	const float tanHalfFovY = 1.0f / matrixProjection.data[0x5];
	const float fovY = 2.0f * atanf( tanHalfFovY ) * RAD2DEG_F;
	const float aspect = matrixProjection[0x5] / matrixProjection[0x0];
	const float_m44 matrixProjectionNearFar = float_m44_build_perspective( fovY, aspect, _near, _far );
	return frustum_build( matrixView, matrixProjectionNearFar );
}


FrustumDouble frustum_build( const double_m44 &matrixView, const double_m44 &matrixProjection,
	double _near, double _far )
{
	const double tanHalfFovY = 1.0 / matrixProjection.data[0x5];
	const double fovY = 2.0 * atan( tanHalfFovY ) * RAD2DEG;
	const double aspect = matrixProjection[0x5] / matrixProjection[0x0];
	const double_m44 matrixProjectionNearFar = double_m44_build_perspective( fovY, aspect, _near, _far );
	return frustum_build( matrixView, matrixProjectionNearFar );
}


bool frustum_contains_point( const FrustumFloat &frustum, const float_v3 &point )
{
	for( int i = 0; i < FRUSTUMPLANE_COUNT; i++ )
	{
		const FrustumFloat::Plane &plane = frustum.planes[i];
		if( plane.distance_to_point( point ) < 0.0f ) { return false; }
	}

	return true;
}


bool frustum_contains_point( const FrustumDouble &frustum, const double_v3 &point )
{
	for( int i = 0; i < FRUSTUMPLANE_COUNT; i++ )
	{
		const FrustumDouble::Plane &plane = frustum.planes[i];
		if( plane.distance_to_point( point ) < 0.0 ) { return false; }
	}

	return true;
}


bool frustum_contains_aabb( const FrustumFloat &frustum, const float_v3 &xyzMin, const float_v3 &xyzMax )
{
	for( int i = 0; i < FRUSTUMPLANE_COUNT; i++ )
	{
		const FrustumFloat::Plane &plane = frustum.planes[i];

		const float_v3 vertex =
		{
			( plane.normal.x >= 0.0f ) ? xyzMax.x : xyzMin.x,
			( plane.normal.y >= 0.0f ) ? xyzMax.y : xyzMin.y,
			( plane.normal.z >= 0.0f ) ? xyzMax.z : xyzMin.z
		};

		if( plane.distance_to_point( vertex ) < 0.0f ) { return false; }
	}

	return true;
}


bool frustum_contains_aabb( const FrustumDouble &frustum, const double_v3 &xyzMin, const double_v3 &xyzMax )
{
	for( int i = 0; i < FRUSTUMPLANE_COUNT; i++ )
	{
		const FrustumDouble::Plane &plane = frustum.planes[i];

		const double_v3 vertex =
		{
			( plane.normal.x >= 0.0 ) ? xyzMax.x : xyzMin.x,
			( plane.normal.y >= 0.0 ) ? xyzMax.y : xyzMin.y,
			( plane.normal.z >= 0.0 ) ? xyzMax.z : xyzMin.z
		};

		if( plane.distance_to_point( vertex ) < 0.0 ) { return false; }
	}

	return true;
}


static void frustum_draw( const float_v3 *corners, const Color &color, const bool wireframe )
{
	Assert( corners != nullptr );
	const u8_v4 col = { color.r, color.g, color.b, color.a };
	GfxVertexBuffer<GfxVertex::BuiltinVertex> frustum;
	frustum.init( wireframe ? 24 : 36, GfxWriteMode_OVERWRITE );

	frustum.write_begin();
	if( wireframe )
	{
		auto write_edge = [&]( int i0, int i1 )
		{
			frustum.write( GfxVertex::BuiltinVertex { corners[i0], u16_v2 { 0x0000, 0x0000 }, col } );
			frustum.write( GfxVertex::BuiltinVertex { corners[i1], u16_v2 { 0xFFFF, 0x0000 }, col } );
		};

		write_edge( FrustumCorner_NearTopLeft, FrustumCorner_NearTopRight );
		write_edge( FrustumCorner_NearTopRight, FrustumCorner_NearBottomRight );
		write_edge( FrustumCorner_NearBottomRight, FrustumCorner_NearBottomLeft );
		write_edge( FrustumCorner_NearBottomLeft, FrustumCorner_NearTopLeft );
		write_edge( FrustumCorner_FarTopLeft, FrustumCorner_FarTopRight );
		write_edge( FrustumCorner_FarTopRight, FrustumCorner_FarBottomRight );
		write_edge( FrustumCorner_FarBottomRight,FrustumCorner_FarBottomLeft );
		write_edge( FrustumCorner_FarBottomLeft, FrustumCorner_FarTopLeft );
		write_edge( FrustumCorner_NearTopLeft, FrustumCorner_FarTopLeft );
		write_edge( FrustumCorner_NearTopRight, FrustumCorner_FarTopRight );
		write_edge( FrustumCorner_NearBottomRight, FrustumCorner_FarBottomRight );
		write_edge( FrustumCorner_NearBottomLeft, FrustumCorner_FarBottomLeft );
	}
	else
	{
		auto write_face = [&]( int i0, int i1, int i2, int i3 )
		{
			frustum.write( GfxVertex::BuiltinVertex { corners[i0], u16_v2 { 0x0000, 0x0000 }, col } );
			frustum.write( GfxVertex::BuiltinVertex { corners[i1], u16_v2 { 0xFFFF, 0x0000 }, col } );
			frustum.write( GfxVertex::BuiltinVertex { corners[i2], u16_v2 { 0x0000, 0xFFFF }, col } );
			frustum.write( GfxVertex::BuiltinVertex { corners[i0], u16_v2 { 0x0000, 0x0000 }, col } );
			frustum.write( GfxVertex::BuiltinVertex { corners[i2], u16_v2 { 0x0000, 0xFFFF }, col } );
			frustum.write( GfxVertex::BuiltinVertex { corners[i3], u16_v2 { 0xFFFF, 0xFFFF }, col } );
		};

		write_face( FrustumCorner_NearTopLeft, FrustumCorner_NearTopRight,
			FrustumCorner_NearBottomRight, FrustumCorner_NearBottomLeft );
		write_face( FrustumCorner_FarTopLeft, FrustumCorner_FarTopRight,
			FrustumCorner_FarBottomRight, FrustumCorner_FarBottomLeft );
		write_face( FrustumCorner_NearTopLeft, FrustumCorner_FarTopLeft,
			FrustumCorner_FarBottomLeft, FrustumCorner_NearBottomLeft );
		write_face( FrustumCorner_NearTopRight, FrustumCorner_FarTopRight,
			FrustumCorner_FarBottomRight, FrustumCorner_NearBottomRight );
		write_face( FrustumCorner_NearTopLeft, FrustumCorner_FarTopLeft,
			FrustumCorner_FarTopRight, FrustumCorner_NearTopRight );
		write_face( FrustumCorner_NearBottomLeft, FrustumCorner_FarBottomLeft,
			FrustumCorner_FarBottomRight, FrustumCorner_NearBottomRight );
	}
	frustum.write_end();

	GfxRenderCommand cmd;
	cmd.shader( Shader::SHADER_DEFAULT );
	cmd.depth_function( GfxDepthFunction_NONE );
	Gfx::render_command_execute( cmd, GfxWorkCapture
		{
			Gfx::bind_texture( 0, Texture::TEXTURE_DEFAULT );
			Gfx::draw_vertex_buffer( frustum, wireframe ? GfxPrimitiveType_LineList :
				GfxPrimitiveType_TriangleList );
		} );

	frustum.free();
}


void frustum_draw( const FrustumFloat &frustum, const Color &color, bool wireframe )
{
	frustum_draw( frustum.corners, color, wireframe );
}


void frustum_draw( const FrustumDouble &frustum, const Color &color, bool wireframe )
{
	float_v3 corners[FRUSTUMCORNER_COUNT];
	for( int i = 0; i < FRUSTUMCORNER_COUNT; i++ )
	{
		corners[i] = float_v3 { frustum.corners[i].x, frustum.corners[i].y, frustum.corners[i].z };
	}

	frustum_draw( corners, color, wireframe );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility

void draw_axis_3d( int x, int y, u16 width, u16 height, float_v3 forward, float_v3 up, const Color backgroundColor )
{
	// Vertex Buffer
	GfxVertexBuffer<GfxVertex::BuiltinVertexPositionColor> vertexBuffer;
	vertexBuffer.init( 36 * 3, GfxWriteMode_OVERWRITE );
	vertexBuffer.write_begin();
	{
		auto write_cube = [&]( float_v3 min, float_v3 max, u8_v4 color )
		{
			float_v3 verts[8] =
			{
				{ min.x, min.y, min.z },
				{ max.x, min.y, min.z },
				{ max.x, max.y, min.z },
				{ min.x, max.y, min.z },
				{ min.x, min.y, max.z },
				{ max.x, min.y, max.z },
				{ max.x, max.y, max.z },
				{ min.x, max.y, max.z },
			};

			static const int idx[36] =
			{
				0, 1, 2, 2, 3, 0, // back
				4, 5, 6, 6, 7, 4, // front
				0, 4, 7, 7, 3, 0, // left
				1, 5, 6, 6, 2, 1, // right
				3, 2, 6, 6, 7, 3, // top
				0, 1, 5, 5, 4, 0  // bottom
			};

			for( int i = 0; i < 36; i++ )
			{
				vertexBuffer.write( GfxVertex::BuiltinVertexPositionColor { verts[idx[i]], color } );
			}
		};

		float t = 0.02f; // thickness
		write_cube( float_v3 { 0.0f, -t, -t }, float_v3 { 1.0f, t, t }, u8_v4 { 255, 0, 0, 255 } ); // X
		write_cube( float_v3 { -t, 0.0f, -t }, float_v3 { t, 1.0f, t }, u8_v4 { 0, 255, 0, 255 } ); // Y
		write_cube( float_v3 { -t, -t, 0.0f }, float_v3 { t, t, 1.0f }, u8_v4 { 0, 0, 255, 255 } ); // Z
	}
	vertexBuffer.write_end();

	// Initialize Render Target
	GfxRenderTargetDescription rtDesc;
	rtDesc.colorFormat = GfxColorFormat_R8G8B8A8_FLOAT;
	rtDesc.depthFormat = GfxDepthFormat_R16_FLOAT;

	GfxRenderTarget rt;
	rt.init( width, height, rtDesc );

	// Cache Gfx State
	double_m44 CACHE_MATRIX_MODEL = Gfx::get_matrix_model();
	double_m44 CACHE_MATRIX_VIEW = Gfx::get_matrix_view();
	double_m44 CACHE_MATRIX_PERSPECTIVE = Gfx::get_matrix_perspective();

	// Draw XYZ Gizmo
	GfxRenderPass pass;
	pass.target( 0, rt );
	pass.set_name( "XYZ Gizmo" );
	Gfx::render_pass_begin( pass );
	{
		Gfx::clear_color( backgroundColor );
		Gfx::clear_depth();

		GfxRenderCommand cmd;
		cmd.shader( Shader::SHADER_DEFAULT_RGB );
		cmd.depth_function( GfxDepthFunction_LESS_EQUALS );
		cmd.raster_cull_mode( GfxCullMode_NONE );
		Gfx::render_command_execute( cmd, GfxWorkCapture
			{
				const double_m44 matrixView = double_m44_build_lookat(
					-forward.x * 4.0,
					-forward.y * 4.0,
					-forward.z * 4.0,
					0.0, 0.0, 0.0, up.x, up.y, up.z );

				const double_m44 matrixPerspective = double_m44_build_perspective( 35.0,
					static_cast<double>( width ) / height, 0.1, 16.0 );

				Gfx::set_matrix_mvp( double_m44_build_identity(), matrixView, matrixPerspective );

				Gfx::draw_vertex_buffer( vertexBuffer );
			} );
	}
	Gfx::render_pass_end( pass );

	// Restore GFX State
	Gfx::set_matrix_mvp( CACHE_MATRIX_MODEL, CACHE_MATRIX_VIEW, CACHE_MATRIX_PERSPECTIVE );

	// Draw Render Target
	GfxRenderCommand cmd;
	cmd.shader( Shader::SHADER_DEFAULT );
	Gfx::render_command_execute( cmd, GfxWorkCapture
		{
			Gfx::bind_texture( 0, rt.textureColor );
			draw_quad_uv( x, y, x + width, y + height,
				0x0000, 0x0000, 0xFFFF, 0xFFFF, c_white );
		} );

	// Free Resources
	rt.free();
	vertexBuffer.free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////