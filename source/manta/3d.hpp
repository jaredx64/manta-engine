#pragma once

#include <core/color.hpp>
#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool ray_intersect_triangle( const float_r3 &ray,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2, float_r3::hit &hit );

extern bool ray_intersect_triangle( const float_r3 &ray,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2, float_r3::hit &hit );

extern bool ray_intersect_triangle_list( const float_r3 &ray,
	const float_v3 *verts, const usize count, float_r3::hit &hit );

extern bool ray_intersect_triangle_list( const float_r3 &ray,
	const double_v3 *verts, const usize count, float_r3::hit &hit );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool ray_intersect_triangle( const double_r3 &ray,
	const double_v3 &v0, const double_v3 &v1, const double_v3 &v2, double_r3::hit &hit );

extern bool ray_intersect_triangle( const double_r3 &ray,
	const float_v3 &v0, const float_v3 &v1, const float_v3 &v2, double_r3::hit &hit );

extern bool ray_intersect_triangle_list( const double_r3 &ray,
	const double_v3 *verts, const usize count, double_r3::hit &hit );

extern bool ray_intersect_triangle_list( const double_r3 &ray,
	const float_v3 *verts, const usize count, double_r3::hit &hit );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern float_r3 ray_screen_point_to_world_perspective( const float_v2 &pointScreen, const float_v2 &dimensionsScreen,
	const float_v3 &pointEye, const float_m44 &matrixView, const float_m44 &matrixProjection );

extern double_r3 ray_screen_point_to_world_perspective( const double_v2 &pointScreen, const double_v2 &dimensionsScreen,
	const double_v3 &pointEye, const double_m44 &matrixView, const double_m44 &matrixProjection );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern float_v2 point_world_to_screen( const float_v3 &pointWorld, const float_v2 &dimensionsScreen,
	const float_m44 &matrixView, const float_m44 &matrixProjection );

extern double_v2 point_world_to_screen( const double_v3 &pointWorld, const double_v2 &dimensionsScreen,
	const double_m44 &matrixView, const double_m44 &matrixProjection );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern float_v3 triangle_calculate_normal_3d( const float_v3 &v0, const float_v3 &v1, const float_v3 &v2 );

extern double_v3 triangle_calculate_normal_3d( const double_v3 &v0, const double_v3 &v1, const double_v3 &v2 );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern FrustumDouble frustum_build( const double_m44 &matrixMVP );
extern FrustumDouble frustum_build( const double_m44 &matrixView, const double_m44 &matrixProjection );

extern FrustumFloat frustum_build( const float_m44 &matmatrixMVPMVP );
extern FrustumFloat frustum_build( const float_m44 &matrixView, const float_m44 &matrixProjection );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool frustum_contains_aabb( const FrustumDouble &frustum, const double_v3 &xyzMin, const double_v3 &xyzMax );

extern bool frustum_contains_aabb( const FrustumFloat &frustum, const float_v3 &xyzMin, const float_v3 &xyzMax );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool frustum_contains_point( const FrustumDouble &frustum, const double_v3 &point );

extern bool frustum_contains_point( const FrustumFloat &frustum, const float_v3 &point );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void frustum_draw( const FrustumDouble &frustum, const Color &color = c_red, const bool wireframe = true );

extern void frustum_draw( const FrustumFloat &frustum, const Color &color = c_red, const bool wireframe = true );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_axis_3d( int x, int y, u16 width, u16 height, float_v3 forward, float_v3 up,
	const Color backgroundColor = Color { 0, 0, 0, 0 } );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////