#include <view.hpp>

#include <manta/window.hpp>
#include <manta/input.hpp>
#include <manta/3d.hpp>

#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <earth.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace View
{
	double fov = 45.0;
	double roll = 0.0;
	double near;
	double far;
	double aspect;

	double pitch = 10.0;
	double yaw = 250.0;
	bool orbit = true;

	double_v2 dimensions;

	double_v3 position;
	double_v3 forward;
	double_v3 up;

	double_m44 matrixWorld;
	double_m44 matrixLook;
	double_m44 matrixView;
	double_m44 matrixPerspective;

	double orbitDistance = 17'500'000;
	bool pickPointValid = false;
	double_v3 pickPoint = double_v3 { 0.0, 0.0, 0.0 };

	double targetPitch = 0.0;
	double targetYaw = 0.0;
	bool targetting = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static double_v2 point_ecef_to_yaw_pitch( double_v3 point )
{
	double_v2 yawPitch;
	yawPitch.x = atan2f( point.y, point.x ) + PI_F;
	float h = sqrtf( point.x * point.x + point.y * point.y );
	yawPitch.y = atan2f( point.z, h );
	return yawPitch;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void view_controls( const Delta delta )
{
	// Pick
	double_r3 pickRay = ray_screen_point_to_world_perspective(
		double_v2 { Mouse::x_logical(), Mouse::y_logical() },
		double_v2 { Window::width_logical(), Window::height_logical() },
		View::position, View::matrixView, View::matrixPerspective );

	double_r3::hit pickHit;
	if( ray_intersect_triangle_list( pickRay, Earth::collision.data, Earth::collision.count(), pickHit ) )
	{
		View::pickPoint = pickRay.origin + pickRay.vector * pickHit.distance;
		View::pickPointValid = true;
	}
	else
	{
		View::pickPointValid = false;
		View::pickPoint = double_v3 { 0.0, 0.0, 0.0 };
	}

	// Input
	static double pitchVelocity = 0.0;
	static double pitchVelocityTarget = 0.0;
	static double yawVelocity = 0.0;
	static double yawVelocityTarget = 0.0;
	static double zoomVelocity = 0.0;
	static double zoomVelocityTarget = 0.0;

	const double a = Keyboard::check( vk_shift ) ? 2.0 : 1.0;
	pitchVelocityTarget = 0.0;
	if( Keyboard::check( vk_w ) ) { View::orbit = false; View::targetting = false; pitchVelocityTarget = +30.0 * a; }
	if( Keyboard::check( vk_s ) ) { View::orbit = false; View::targetting = false; pitchVelocityTarget = -30.0 * a; }
	yawVelocityTarget = 0.0;
	if( Keyboard::check( vk_a ) ) { View::orbit = false; View::targetting = false; yawVelocityTarget = +20.0 * a; }
	if( Keyboard::check( vk_d ) ) { View::orbit = false; View::targetting = false; yawVelocityTarget = -20.0 * a; }
	zoomVelocityTarget = 0.0;
	if( Keyboard::check( vk_e ) ) { zoomVelocityTarget = -15.0 * a; }
	if( Keyboard::check( vk_q ) ) { zoomVelocityTarget = +15.0 * a; }
	if( Mouse::check_wheel_up() ) { zoomVelocityTarget = -100.0 * a; zoomVelocity = zoomVelocityTarget; }
	if( Mouse::check_wheel_down() ) { zoomVelocityTarget = +100.0 * a; zoomVelocity = zoomVelocityTarget; }
	if( Keyboard::check_pressed( vk_space ) ) { View::targetting = false; View::orbit = !View::orbit; }
	if( View::orbit ) { yawVelocityTarget = 4.0 * a; }

	if( Mouse::check_pressed( mb_left ) && View::pickPointValid )
	{
		const double_v2 yawPitch = point_ecef_to_yaw_pitch( double_v3_normalize( View::pickPoint ) );
		View::targetYaw = fmod( yawPitch.x * RAD2DEG + 180.0, 360.0 );
		View::targetPitch = yawPitch.y * RAD2DEG;
		View::targetting = true;
		View::orbit = false;
	}

	if( View::targetting )
	{
		View::yaw = lerp_degrees( View::yaw, View::targetYaw, 2.0 * delta );
		View::pitch = lerp( View::pitch, View::targetPitch, 2.0 * delta );
		if( fabs( View::targetYaw - View::yaw ) < 0.02 && fabs( View::targetPitch - View::pitch ) < 0.02 )
		{
			View::targetting = false;
		}
	}

	// Movement
	pitchVelocity = lerp_delta( pitchVelocity, pitchVelocityTarget, 20.0, delta );
	View::pitch += pitchVelocity * delta;
	yawVelocity = lerp_delta( yawVelocity, yawVelocityTarget, 20.0, delta );
	View::yaw += yawVelocity * delta;
	zoomVelocity = lerp_delta( zoomVelocity, zoomVelocityTarget, 20.0, delta );
	View::fov += zoomVelocity * delta;
	View::yaw = wrap( View::yaw, 0.0, 360.0 );
	View::pitch = clamp( View::pitch, -85.0, 85.0 );
	View::fov = clamp( View::fov, 20.0, 100.0 );

	// Orbit
	View::position.x = View::orbitDistance * cos( View::pitch * DEG2RAD ) * cos( View::yaw * DEG2RAD );
	View::position.y = View::orbitDistance * cos( View::pitch * DEG2RAD ) * sin( View::yaw * DEG2RAD );
	View::position.z = View::orbitDistance * sin( View::pitch * DEG2RAD );
	View::forward = double_v3_normalize( View::position ) * -1.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void view_update( const Delta delta )
{
	View::roll = wrap( View::roll, 0.0, 360.0 );
	double rollCos = cos( View::roll * DEG2RAD );
	double rollSin = -sin( View::roll * DEG2RAD );
	double_v3 up = double_v3 { 0.0, 0.0, 1.0 };
	View::up = up * rollCos + double_v3_cross( View::forward, up ) * rollSin +
		View::forward * double_v3_dot( View::forward, up ) * ( 1.0 - rollCos );

	View::near = 100000.0;
	View::far = R_EARTH_POLE * 4.0;

	View::dimensions = double_v2 { Window::width_pixels(), Window::height_pixels() };
	View::aspect = View::dimensions.x / View::dimensions.y;

	View::matrixWorld = double_m44_build_identity();
	View::matrixLook = double_m44_build_lookat( 0.0, 0.0, 0.0,
		View::forward.x, View::forward.y, View::forward.z, View::up.x, View::up.y, View::up.z );
	View::matrixView = double_m44_build_lookat( View::position.x, View::position.y, View::position.z,
		0.0, 0.0, 0.0, View::up.x, View::up.y, View::up.z );
	View::matrixPerspective = double_m44_build_perspective( View::fov, View::aspect, View::near, View::far );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////