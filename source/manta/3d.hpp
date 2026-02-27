#pragma once

#include <core/color.hpp>
#include <core/types.hpp>
#include <core/traits.hpp>
#include <core/list.hpp>
#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Point

extern float_v2 point_world_to_screen( const float_v3 &pointWorld, const float_v2 &dimensionsScreen,
	const float_m44 &matrixView, const float_m44 &matrixProjection );
extern double_v2 point_world_to_screen( const double_v3 &pointWorld, const double_v2 &dimensionsScreen,
	const double_m44 &matrixView, const double_m44 &matrixProjection );

extern float_v3 point_closest_on_line( const float_v3 &point, const float_v3 &v0, const float_v3 &v1 );
extern double_v3 point_closest_on_line( const double_v3 &point, const double_v3 &v0, const double_v3 &v1 );

extern float_v3 closest_point_on_ray( const float_v3 &point, const float_r3 &ray );
extern double_v3 closest_point_on_ray( const double_v3 &point, const double_r3 &ray );

extern float_v3 closest_point_on_triangle( const float_v3 &point,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2 );
extern double_v3 closest_point_on_triangle( const double_v3 &point,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2 );

extern float_v3 point_closest_on_aabb( const float_v3 &point, const float_v3 &min, const float_v3 &max );
extern double_v3 point_closest_on_aabb( const double_v3 &point, const double_v3 &min, const double_v3 &max );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ray

extern float_r3 ray_screen_point_to_world_perspective( const float_v2 &pointScreen, const float_v2 &dimensionsScreen,
	const float_v3 &pointEye, const float_m44 &matrixView, const float_m44 &matrixProjection );
extern double_r3 ray_screen_point_to_world_perspective( const double_v2 &pointScreen, const double_v2 &dimensionsScreen,
	const double_v3 &pointEye, const double_m44 &matrixView, const double_m44 &matrixProjection );

extern bool ray_intersects_aabb( const float_r3 &ray, const float_v3 &xyzMin, const float_v3 &xyzMax,
	float &distance );
extern bool ray_intersects_aabb( const double_r3 &ray, const double_v3 &xyzMin, const double_v3 &xyzMax,
	float &distance );

extern bool ray_intersect_triangle( const float_r3 &ray,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2, float_r3::hit &hit );
extern bool ray_intersect_triangle( const double_r3 &ray,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2, double_r3::hit &hit );

extern bool ray_intersect_triangle_list( const float_r3 &ray,
	const float_v3 *verts, usize count, float_r3::hit &hit );
extern bool ray_intersect_triangle_list( const double_r3 &ray,
	const double_v3 *verts, usize count, double_r3::hit &hit );

extern bool ray_intersect_triangle_list( const float_r3 &ray, const float_m44 &matrix,
	const float_v3 *verts, usize count, float_r3::hit &hit );
extern bool ray_intersect_triangle_list( const double_r3 &ray, const double_m44 &matrix,
	const double_v3 *verts, usize count, double_r3::hit &hit );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Triangle

extern float_v3 triangle_calculate_normal_3d( const float_v3 &v0, const float_v3 &v1, const float_v3 &v2 );
extern double_v3 triangle_calculate_normal_3d( const double_v3 &v0, const double_v3 &v1, const double_v3 &v2 );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AABB

struct AABBSweepHitFloat
{
	float time;
	float_v3 position;
	float_v3 normal;
	usize triangleID = USIZE_MAX;
};

struct AABBSweepHitDouble
{
	double time;
	double_v3 position;
	double_v3 normal;
	usize triangleID = USIZE_MAX;
};

extern bool aabb_sweep_triangle( const float_v3 &aabbMin, const float_v3 &aabbMax,
	const float_v3 &position, const float_v3 &velocity,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2,
	AABBSweepHitFloat &hit );

extern bool aabb_sweep_triangle( const double_v3 &aabbMin, const double_v3 &aabbMax,
	const double_v3 &position, const double_v3 &velocity,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2,
	AABBSweepHitDouble &hit );

extern bool aabb_sweep_triangle_list( const float_v3 &aabbMin, const float_v3 &aabbMax,
	const float_v3 &position, const float_v3 &velocity,
	const float_v3 *verts, usize count,
	AABBSweepHitFloat &hit );

extern bool aabb_sweep_triangle_list( const double_v3 &aabbMin, const double_v3 &aabbMax,
	const double_v3 &position, const double_v3 &velocity,
	const double_v3 *verts, usize count,
	AABBSweepHitDouble &hit );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sphere

namespace SphereGeometry
{
	struct EdgeKey
	{
		u32 v1, v2;

		bool operator==( const EdgeKey &o ) const
		{
			return ( v1 == o.v1 && v2 == o.v2 ) || ( v1 == o.v2 && v2 == o.v1 );
		}
	};
}

namespace Hash
{
	inline u32 hash( SphereGeometry::EdgeKey key ) { return hash( key.v1 ^ key.v2 ); }
	inline bool equals( SphereGeometry::EdgeKey a, SphereGeometry::EdgeKey b ) { return a == b; }
	inline bool is_null( SphereGeometry::EdgeKey a ) { return a == SphereGeometry::EdgeKey { U32_MAX, U32_MAX }; }
	inline void set_null( SphereGeometry::EdgeKey &a ) { a = SphereGeometry::EdgeKey { U32_MAX, U32_MAX }; }
}

struct SphereSweepHitFloat
{
	bool hit;
	float time;
	float_v3 position;
	float_v3 normal;
};

struct SphereSweepHitDouble
{
	bool hit;
	double time;
	double_v3 position;
	double_v3 normal;
};

extern SphereSweepHitFloat sphere_sweep_triangle_list( const float_v3 &center, float radius,
	const float_v3 &velocity, const float_v3 *verts, usize count );
extern SphereSweepHitDouble sphere_sweep_triangle_list( const double_v3 &center, double radius,
	const double_v3 &velocity, const double_v3 *verts, usize count );

extern u32 sphere_generate_geometry_icosahedron( u32 subdivisions, List<float_v3> *positions = nullptr,
	List<float_v3> *normals = nullptr, List<float_v2> *uvs = nullptr, List<u32> *indices = nullptr );
extern u32 sphere_generate_geometry_latlon( u32 resolution, List<float_v3> *positions = nullptr,
	List<float_v3> *normals = nullptr, List<float_v2> *uvs = nullptr, List<u32> *indices = nullptr );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Frustum

enum_type( FrustumCorner, int )
{
	FrustumCorner_NearTopLeft,
	FrustumCorner_NearTopRight,
	FrustumCorner_NearBottomLeft,
	FrustumCorner_NearBottomRight,
	FrustumCorner_FarTopLeft,
	FrustumCorner_FarTopRight,
	FrustumCorner_FarBottomLeft,
	FrustumCorner_FarBottomRight,
	FRUSTUMCORNER_COUNT,
};


enum_type( FrustumPlane, int )
{
	FrustumPlane_Left,
	FrustumPlane_Right,
	FrustumPlane_Bottom,
	FrustumPlane_Top,
	FrustumPlane_Near,
	FrustumPlane_Far,
	FRUSTUMPLANE_COUNT,
};


struct FrustumDouble
{
	struct Plane
	{
		double_v3 normal;
		double distance;

		float distance_to_point( const double_v3 &point ) const
		{
			return normal.x * point.x + normal.y * point.y + normal.z * point.z + distance;
		}
	};

	double_v3 corners[FRUSTUMCORNER_COUNT];
	Plane planes[6];
};


struct FrustumFloat
{
	struct Plane
	{
		float_v3 normal;
		double distance;

		float distance_to_point( const float_v3 &point ) const
		{
			return normal.x * point.x + normal.y * point.y + normal.z * point.z + distance;
		}
	};

	float_v3 corners[FRUSTUMCORNER_COUNT];
	Plane planes[FRUSTUMPLANE_COUNT];
};

extern FrustumFloat frustum_build( const float_m44 &matrixMVP );
extern FrustumDouble frustum_build( const double_m44 &matrixMVP );

extern FrustumFloat frustum_build( const float_m44 &matrixView, const float_m44 &matrixProjection );
extern FrustumDouble frustum_build( const double_m44 &matrixView, const double_m44 &matrixProjection );

extern FrustumFloat frustum_build( const float_m44 &matrixView, const float_m44 &matrixProjection,
	float near, float far );
extern FrustumDouble frustum_build( const double_m44 &matrixView, const double_m44 &matrixProjection,
	double near, double far );

extern bool frustum_contains_point( const FrustumFloat &frustum, const float_v3 &point );
extern bool frustum_contains_point( const FrustumDouble &frustum, const double_v3 &point );

extern bool frustum_contains_aabb( const FrustumFloat &frustum, const float_v3 &xyzMin, const float_v3 &xyzMax );
extern bool frustum_contains_aabb( const FrustumDouble &frustum, const double_v3 &xyzMin, const double_v3 &xyzMax );

extern void frustum_draw( const FrustumFloat &frustum, const Color &color = c_red, bool wireframe = true );
extern void frustum_draw( const FrustumDouble &frustum, const Color &color = c_red, bool wireframe = true );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility

extern void draw_axis_3d( int x, int y, u16 width, u16 height, float_v3 forward, float_v3 up,
	const Color backgroundColor = Color { 0, 0, 0, 0 } );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////