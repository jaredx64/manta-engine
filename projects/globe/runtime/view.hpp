#pragma once

#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace View
{
	extern double fov;
	extern double roll;
	extern double near;
	extern double far;
	extern double aspect;

	extern double pitch;
	extern double yaw;
	extern bool orbit;

	extern double_v2 dimensions;

	extern double_v3 position;
	extern double_v3 forward;
	extern double_v3 up;

	extern double_m44 matrixWorld;
	extern double_m44 matrixLook;
	extern double_m44 matrixView;
	extern double_m44 matrixPerspective;

	extern double orbitDistance;
	extern bool pickPointValid;
	extern double_v3 pickPoint;

	extern double targetPitch;
	extern double targetYaw;
	extern bool targetting;
};

void view_controls( const Delta delta );
void view_update( const Delta delta );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////