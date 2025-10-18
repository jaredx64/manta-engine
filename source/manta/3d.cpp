#include <manta/3d.hpp>

#include <core/math.hpp>
#include <manta/gfx.hpp>
#include <manta/draw.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ray_intersect_triangle( const float_r3 &ray,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2, float_r3::hit &hit )
{
	const float EPSILON = 1e-6f;
	float_v3 edge1 = float_v3 { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
	float_v3 edge2 = float_v3 { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

	float_v3 pvec = float_v3_cross( ray.vector, edge2 );
	float det = edge1.dot( pvec );
	if( det > -EPSILON && det < EPSILON ) { return false; }

	float invDet = 1.0f / det;
	float_v3 tvec = float_v3 { ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z };
	hit.baryU = tvec.dot( pvec ) * invDet;
	if( hit.baryU < 0.0f || hit.baryU > 1.0f ) { return false; }

	float_v3 qvec = tvec.cross( edge1 );
	hit.baryV = ray.vector.dot( qvec ) * invDet;
	if( hit.baryV < 0.0f || hit.baryU + hit.baryV > 1.0f ) { return false; }

	hit.distance = edge2.dot( qvec ) * invDet;
	return ( hit.distance > EPSILON );
}


bool ray_intersect_triangle( const float_r3 &ray,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2, float_r3::hit &hit )
{
	return ray_intersect_triangle( ray,
		float_v3 { v0.x, v0.y, v0.z },
		float_v3 { v1.x, v1.y, v1.z },
		float_v3 { v2.x, v2.y, v2.z }, hit );
}


bool ray_intersect_triangle_list( const float_r3 &ray,
	const float_v3 *verts, const usize count, float_r3::hit &hit )
{
	hit.distance = 1e30f;
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

	return hit.distance < 1e30f;
}


bool ray_intersect_triangle_list( const float_r3 &ray,
	const double_v3 *verts, const usize count, float_r3::hit &hit )
{
	hit.distance = 1e30f;
	float_r3::hit cur;

	for( usize i = 0; i + 2 < count; i += 3 )
	{
		const float_v3 v0 = float_v3 { verts[i + 0].x, verts[i + 0].y, verts[i + 0].z };
		const float_v3 v1 = float_v3 { verts[i + 1].x, verts[i + 1].y, verts[i + 1].z };
		const float_v3 v2 = float_v3 { verts[i + 2].x, verts[i + 2].y, verts[i + 2].z };
		if( ray_intersect_triangle( ray, v0, v1, v2, cur ) && cur.distance < hit.distance )
		{
			hit = cur;
			hit.triangleID = i / 3;
		}
	}

	return hit.distance < 1e30f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ray_intersect_triangle( const double_r3 &ray,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2, double_r3::hit &hit )
{
	const double EPSILON = 1e-12;
	double_v3 edge1 = double_v3 { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
	double_v3 edge2 = double_v3 { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

	double_v3 pvec = double_v3_cross( ray.vector, edge2 );
	double det = edge1.dot( pvec );
	if( det > -EPSILON && det < EPSILON ) { return false; }

	double invDet = 1.0 / det;
	double_v3 tvec = double_v3 { ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z };
	hit.baryU = tvec.dot( pvec ) * invDet;
	if( hit.baryU < 0.0 || hit.baryU > 1.0 ) { return false; }

	double_v3 qvec = tvec.cross( edge1 );
	hit.baryV = ray.vector.dot( qvec ) * invDet;
	if( hit.baryV < 0.0 || hit.baryU + hit.baryV > 1.0 ) { return false; }

	hit.distance = edge2.dot( qvec ) * invDet;
	return ( hit.distance > EPSILON );
}


bool ray_intersect_triangle( const double_r3 &ray,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2, double_r3::hit &hit )
{
	return ray_intersect_triangle( ray,
		double_v3 { v0.x, v0.y, v0.z },
		double_v3 { v1.x, v1.y, v1.z },
		double_v3 { v2.x, v2.y, v2.z }, hit );
}


bool ray_intersect_triangle_list( const double_r3 &ray,
	const double_v3 *verts, const usize count, double_r3::hit &hit )
{
	hit.distance = 1e30;
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

	return hit.distance < 1e30;
}


bool ray_intersect_triangle_list( const double_r3 &ray,
	const float_v3 *verts, const usize count, double_r3::hit &hit )
{
	hit.distance = 1e30;
	double_r3::hit cur;

	for( usize i = 0; i + 2 < count; i += 3 )
	{
		const double_v3 v0 = double_v3 { verts[i + 0].x, verts[i + 0].y, verts[i + 0].z };
		const double_v3 v1 = double_v3 { verts[i + 1].x, verts[i + 1].y, verts[i + 1].z };
		const double_v3 v2 = double_v3 { verts[i + 2].x, verts[i + 2].y, verts[i + 2].z };
		if( ray_intersect_triangle( ray, v0, v1, v2, cur ) && cur.distance < hit.distance )
		{
			hit = cur;
			hit.triangleID = i / 3;
		}
	}

	return hit.distance < 1e30;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float_v3 triangle_calculate_normal_3d( const float_v3 &v0, const float_v3 &v1, const float_v3 &v2 )
{
	return ( v1 - v0 ).cross( v2 - v0 ).normalize();
}


double_v3 triangle_calculate_normal_3d( const double_v3 &v0, const double_v3 &v1, const double_v3 &v2 )
{
	return ( v1 - v0 ).cross( v2 - v0 ).normalize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		if( fabs( denom ) < 1e-12 ) { return double_v3 { 0.0, 0.0, 0.0 }; }
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


FrustumDouble frustum_build( const double_m44 &matrixView, const double_m44 &matrixProjection )
{
	return frustum_build( double_m44_multiply( matrixProjection, matrixView ) );
}


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
		if( fabsf( denom ) < 1e-12f ) { return float_v3 { 0.0f, 0.0f, 0.0f }; }
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


FrustumFloat frustum_build( const float_m44 &matrixView, const float_m44 &matrixProjection )
{
	return frustum_build( float_m44_multiply( matrixProjection, matrixView ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool frustum_contains_point( const FrustumDouble &frustum, const double_v3 &point )
{
	for( int i = 0; i < FRUSTUMPLANE_COUNT; i++ )
	{
		const FrustumDouble::Plane &plane = frustum.planes[i];
		if( plane.distance_to_point( point ) < 0.0 ) { return false; }
	}

	return true;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void frustum_draw( const float_v3 *corners, const Color &color, const bool wireframe )
{
	Assert( corners != nullptr );
	const u8_v4 col = { color.r, color.g, color.b, color.a };
	GfxVertexBuffer<GfxVertex::BuiltinVertex> frustum;
	frustum.init( wireframe ? 24 : 36, GfxCPUAccessMode_WRITE_NO_OVERWRITE );

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
	cmd.set_shader( Shader::SHADER_DEFAULT );
	cmd.depth_set_function( GfxDepthFunction_NONE );
	Gfx::render_command_execute( cmd, GfxWorkCapture
		{
			Gfx::bind_texture( 0, Texture::TEXTURE_DEFAULT );
			Gfx::draw_vertex_buffer( frustum, wireframe ? GfxPrimitiveType_LineList :
				GfxPrimitiveType_TriangleList );
		} );

	frustum.free();
}


void frustum_draw( const FrustumDouble &frustum, const Color &color, const bool wireframe )
{
	float_v3 corners[FRUSTUMCORNER_COUNT];
	for( int i = 0; i < FRUSTUMCORNER_COUNT; i++ )
	{
		corners[i] = float_v3 { frustum.corners[i].x, frustum.corners[i].y, frustum.corners[i].z };
	}

	frustum_draw( corners, color, wireframe );
}


void frustum_draw( const FrustumFloat &frustum, const Color &color, const bool wireframe )
{
	frustum_draw( frustum.corners, color, wireframe );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_axis_3d( int x, int y, u16 width, u16 height, float_v3 forward, float_v3 up, const Color backgroundColor )
{
	// Initialize Vertices
	GfxVertexBuffer<GfxVertex::BuiltinVertexPositionColor> vertexBuffer;
	vertexBuffer.init( 36 * 3, GfxCPUAccessMode_WRITE_NO_OVERWRITE );
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
	pass.set_target( 0, rt );
	pass.set_name( "XYZ Gizmo" );
	Gfx::render_pass_begin( pass );
	{
		Gfx::clear_color( backgroundColor );
		Gfx::clear_depth();

		GfxRenderCommand cmd;
		cmd.set_shader( Shader::SHADER_DEFAULT_RGB );
		cmd.depth_set_function( GfxDepthFunction_LESS_EQUALS );
		cmd.raster_set_cull_mode( GfxRasterCullMode_NONE );
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
	cmd.set_shader( Shader::SHADER_DEFAULT );
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