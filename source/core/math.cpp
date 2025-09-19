#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fast_sin_cos( const float angle, float &sin, float &cos )
{
	// XMScalarSinCos from <DirectXMath.h>

	static const float XM_PI = 3.141592654f;
	static const float XM_1DIV2PI = 0.159154943f;
	static const float XM_PIDIV2 = 1.570796327f;
	static const float XM_2PI = 6.283185307f;

	float quotient, sign, y, y2;

	// Map angle to y in [-pi,pi], x = 2*pi*quotient + remainder.
	quotient = XM_1DIV2PI * angle;

	if( angle >= 0.0f )
	{
		quotient = static_cast<float>( static_cast<int>( quotient + 0.5f ) );
	}
	else
	{
		quotient = static_cast<float>( static_cast<int>( quotient - 0.5f ) );
	}

	y = angle - XM_2PI * quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(angle).
	if( y > XM_PIDIV2 )
	{
		y = XM_PI - y;
		sign = -1.0f;
	}
	else if( y < -XM_PIDIV2 )
	{
		y = -XM_PI - y;
		sign = -1.0f;
	}
	else
	{
		sign = 1.0f;
	}

	y2 = y * y;
	sin = ( ( ( ( ( -2.3889859e-08f * y2 + 2.7525562e-06f ) * y2 - 0.00019840874f ) *
		y2 + 0.0083333310f ) * y2 - 0.16666667f ) * y2 + 1.0f ) * y;
	cos = ( ( ( ( ( -2.6051615e-07f * y2 + 2.4760495e-05f ) * y2 - 0.00138883780f ) *
		y2 + 0.0416666380f ) * y2 - 0.50000000f ) * y2 + 1.0f ) * sign;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
