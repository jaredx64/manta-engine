#include <core/math.hpp>

#include <core/memory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DEPTH_RANGE_0_TO_1 for d3d11 (TODO: move these)
#if GRAPHICS_OPENGL
	#define DEPTH_RANGE_0_TO_1 ( false )
#else
	#define DEPTH_RANGE_0_TO_1 ( true )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 ceilpow2( u32 v )
{
	v -= 1;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v += 1;
	return v;
}


u64 ceilpow2( u64 v )
{
	v -= 1;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	v += 1;
	return v;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double fast_mod( double x, double y )
{
	return x - ( static_cast<int>( x / y ) ) * y;
}


float fast_modf( float x, float y )
{
	return x - ( static_cast<int>( x / y ) ) * y;
}


double fast_floor( double x )
{
	const int f = static_cast<int>( x );
	return static_cast<double>( f - ( f > x ) );
}


float fast_floorf( float x )
{
	const int f = static_cast<int>( x );
	return static_cast<float>( f - ( f > x ) );
}


void fast_sin_cos( double radians, double &sin, double &cos )
{
	// XMScalarSinCos from <DirectXMath.h>

	static const double XM_PI = 3.141592654;
	static const double XM_1DIV2PI = 0.159154943;
	static const double XM_PIDIV2 = 1.570796327;
	static const double XM_2PI = 6.283185307;

	double quotient, sign, y, y2;

	// Map angle to y in [-pi,pi], x = 2*pi*quotient + remainder.
	quotient = XM_1DIV2PI * radians;

	if( radians >= 0.0 )
	{
		quotient = static_cast<double>( static_cast<int>( quotient + 0.5 ) );
	}
	else
	{
		quotient = static_cast<double>( static_cast<int>( quotient - 0.5 ) );
	}

	y = radians - XM_2PI * quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(angle).
	if( y > XM_PIDIV2 )
	{
		y = XM_PI - y;
		sign = -1.0;
	}
	else if( y < -XM_PIDIV2 )
	{
		y = -XM_PI - y;
		sign = -1.0;
	}
	else
	{
		sign = 1.0;
	}

	y2 = y * y;
	sin = ( ( ( ( ( -2.3889859e-08 * y2 + 2.7525562e-06 ) * y2 - 0.00019840874 ) *
		y2 + 0.0083333310 ) * y2 - 0.16666667 ) * y2 + 1.0 ) * y;
	cos = ( ( ( ( ( -2.6051615e-07 * y2 + 2.4760495e-05 ) * y2 - 0.00138883780 ) *
		y2 + 0.0416666380 ) * y2 - 0.50000000 ) * y2 + 1.0 ) * sign;
}


void fast_sinf_cosf( float radians, float &sin, float &cos )
{
	// XMScalarSinCos from <DirectXMath.h>

	static const float XM_PI = 3.141592654f;
	static const float XM_1DIV2PI = 0.159154943f;
	static const float XM_PIDIV2 = 1.570796327f;
	static const float XM_2PI = 6.283185307f;

	float quotient, sign, y, y2;

	// Map angle to y in [-pi,pi], x = 2*pi*quotient + remainder.
	quotient = XM_1DIV2PI * radians;

	if( radians >= 0.0f )
	{
		quotient = static_cast<float>( static_cast<int>( quotient + 0.5f ) );
	}
	else
	{
		quotient = static_cast<float>( static_cast<int>( quotient - 0.5f ) );
	}

	y = radians - XM_2PI * quotient;

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

double lerp_degrees( double value1, double value2, double amount )
{
	value1 = fmod( value1, 360.0 );
	if( value1 < 0.0 ) { value1 += 360.0; }

	value2 = fmod( value2, 360.0 );
	if( value2 < 0.0 ) { value2 += 360.0; }

	double diff = value2 - value1;
	if( diff > 180.0 ) { diff -= 360.0; } else
	if( diff < -180.0 ) { diff += 360.0; }

	double result = value1 + diff * amount;
	result = fmod( result, 360.0 );
	if( result < 0.0 ) { result += 360.0; }
	return result;
}


float lerp_degrees( float value1, float value2, float amount )
{
	value1 = fmod( value1, 360.0f );
	if( value1 < 0.0f ) { value1 += 360.0f; }

	value2 = fmod( value2, 360.0f );
	if( value2 < 0.0f ) { value2 += 360.0f; }

	float diff = value2 - value1;
	if( diff > 180.0f ) { diff -= 360.0f; } else
	if( diff < -180.0f ) { diff += 360.0f; }

	float result = value1 + diff * amount;
	result = fmod( result, 360.0f );
	if( result < 0.0f ) { result += 360.0f; }
	return result;
}


double lerp_radians( double value1, double value2, double amount )
{
	constexpr double PI_2 = PI * 2.0;

	value1 = fmod( value1, PI_2 );
	if( value1 < 0.0 ) { value1 += PI_2; }

	value2 = fmod( value2, PI_2 );
	if( value2 < 0.0 ) { value2 += PI_2; }

	double diff = value2 - value1;
	if( diff > PI_F ) { diff -= PI_2; } else
	if( diff < -PI_F ) { diff += PI_2; }

	double result = value1 + diff * amount;
	result = fmod( result, PI_2 );
	if( result < 0.0 ) { result += PI_2; }
	return result;
}


float lerp_radians( float value1, float value2, float amount )
{
	constexpr float PI_F_2 = PI_F * 2.0f;

	value1 = fmod( value1, PI_F_2 );
	if( value1 < 0.0f ) { value1 += PI_F_2; }

	value2 = fmod( value2, PI_F_2 );
	if( value2 < 0.0f ) { value2 += PI_F_2; }

	float diff = value2 - value1;
	if( diff > PI_F ) { diff -= PI_F_2; } else
	if( diff < -PI_F ) { diff += PI_F_2; }

	float result = value1 + diff * amount;
	result = fmod( result, PI_F_2 );
	if( result < 0.0f ) { result += PI_F_2; }
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float_v2 cubic_bezier( float_v2 root0, float_v2 root1, float_v2 handle0, float_v2 handle1, float t )
{
	const float tSqr = t * t;
	const float tInv = 1.0f - t;
	const float tInvSqr = tInv * tInv;
	const float tCube = tSqr * t;
	const float tInvCube = tInvSqr * tInv;

	return float_v2
		{
			tInvCube * root0.x + 3.0f * tInvSqr * t * handle0.x + 3.0f * tInv * tSqr * handle1.x + tCube * root1.x,
			tInvCube * root0.y + 3.0f * tInvSqr * t * handle0.y + 3.0f * tInv * tSqr * handle1.y + tCube * root1.y,
		};
}


double_v2 cubic_bezier( double_v2 root0, double_v2 root1, double_v2 handle0, double_v2 handle1, double t )
{
	const double tSqr = t * t;
	const double tInv = 1.0 - t;
	const double tInvSqr = tInv * tInv;
	const double tCube = tSqr * t;
	const double tInvCube = tInvSqr * tInv;

	return double_v2
		{
			tInvCube * root0.x + 3.0 * tInvSqr * t * handle0.x + 3.0 * tInv * tSqr * handle1.x + tCube * root1.x,
			tInvCube * root0.y + 3.0 * tInvSqr * t * handle0.y + 3.0 * tInv * tSqr * handle1.y + tCube * root1.y,
		};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4x4 Matrix (Float)

float_m44::float_m44()
{
	for( int i = 0; i < 16; i++ ) { data[i] = 0.0f; }
}


float_m44::float_m44( const float_m44 &matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; }
}


float_m44::float_m44( float_m44 &&matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; }
}


float_m44::float_m44( const double_m44 &matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<float>( matrix.data[i] ); }
}


float_m44::float_m44( double_m44 &&matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<float>( matrix.data[i] ); }
}


float_m44 &float_m44::operator=( const float_m44 &matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; } return *this;
}


float_m44 &float_m44::operator=( float_m44 &&matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; } return *this;
}


float_m44::operator double_m44() const
{
	double_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<double>( data[i] ); }
	return result;
}


bool float_m44_equal( const float_m44 &a, const float_m44 &b )
{
	return a[0x0] == b[0x0] &&
		a[0x1] == b[0x1] &&
		a[0x2] == b[0x2] &&
		a[0x3] == b[0x3] &&
		a[0x4] == b[0x4] &&
		a[0x5] == b[0x5] &&
		a[0x6] == b[0x6] &&
		a[0x7] == b[0x7] &&
		a[0x8] == b[0x8] &&
		a[0x9] == b[0x9] &&
		a[0xA] == b[0xA] &&
		a[0xB] == b[0xB] &&
		a[0xC] == b[0xC] &&
		a[0xD] == b[0xD] &&
		a[0xE] == b[0xE] &&
		a[0xF] == b[0xF];
}


float_m44 float_m44_transpose( const float_m44 &matrix )
{
	float_m44 m;
	m[0x0] = matrix[0x0];
	m[0x1] = matrix[0x4];
	m[0x2] = matrix[0x8];
	m[0x3] = matrix[0xC];
	m[0x4] = matrix[0x1];
	m[0x5] = matrix[0x5];
	m[0x6] = matrix[0x9];
	m[0x7] = matrix[0xD];
	m[0x8] = matrix[0x2];
	m[0x9] = matrix[0x6];
	m[0xA] = matrix[0xA];
	m[0xB] = matrix[0xE];
	m[0xC] = matrix[0x3];
	m[0xD] = matrix[0x7];
	m[0xE] = matrix[0xB];
	m[0xF] = matrix[0xF];
	return m;
}


float_m44 float_m44_inverse( const float_m44 &matrix )
{
	float_m44 m;
	float_m44 inv;

	inv[0x0] = matrix[0x5] * matrix[0xA] * matrix[0xF] -
		matrix[0x5] * matrix[0xB] * matrix[0xE] -
		matrix[0x9] * matrix[0x6] * matrix[0xF] +
		matrix[0x9] * matrix[0x7] * matrix[0xE] +
		matrix[0xD] * matrix[0x6] * matrix[0xB] -
		matrix[0xD] * matrix[0x7] * matrix[0xA];

	inv[0x4] = -matrix[0x4] * matrix[0xA] * matrix[0xF] +
		matrix[0x4] * matrix[0xB] * matrix[0xE] +
		matrix[0x8] * matrix[0x6] * matrix[0xF] -
		matrix[0x8] * matrix[0x7] * matrix[0xE] -
		matrix[0xC] * matrix[0x6] * matrix[0xB] +
		matrix[0xC] * matrix[0x7] * matrix[0xA];

	inv[0x8] = matrix[0x4] * matrix[0x9] * matrix[0xF] -
		matrix[0x4] * matrix[0xB] * matrix[0xD] -
		matrix[0x8] * matrix[0x5] * matrix[0xF] +
		matrix[0x8] * matrix[0x7] * matrix[0xD] +
		matrix[0xC] * matrix[0x5] * matrix[0xB] -
		matrix[0xC] * matrix[0x7] * matrix[0x9];

	inv[0xC] = -matrix[0x4] * matrix[0x9] * matrix[0xE] +
		matrix[0x4] * matrix[0xA] * matrix[0xD] +
		matrix[0x8] * matrix[0x5] * matrix[0xE] -
		matrix[0x8] * matrix[0x6] * matrix[0xD] -
		matrix[0xC] * matrix[0x5] * matrix[0xA] +
		matrix[0xC] * matrix[0x6] * matrix[0x9];

	inv[0x1] = -matrix[0x1] * matrix[0xA] * matrix[0xF] +
		matrix[0x1] * matrix[0xB] * matrix[0xE] +
		matrix[0x9] * matrix[0x2] * matrix[0xF] -
		matrix[0x9] * matrix[0x3] * matrix[0xE] -
		matrix[0xD] * matrix[0x2] * matrix[0xB] +
		matrix[0xD] * matrix[0x3] * matrix[0xA];

	inv[0x5] = matrix[0x0] * matrix[0xA] * matrix[0xF] -
		matrix[0x0] * matrix[0xB] * matrix[0xE] -
		matrix[0x8] * matrix[0x2] * matrix[0xF] +
		matrix[0x8] * matrix[0x3] * matrix[0xE] +
		matrix[0xC] * matrix[0x2] * matrix[0xB] -
		matrix[0xC] * matrix[0x3] * matrix[0xA];

	inv[0x9] = -matrix[0x0] * matrix[0x9] * matrix[0xF] +
		matrix[0x0] * matrix[0xB] * matrix[0xD] +
		matrix[0x8] * matrix[0x1] * matrix[0xF] -
		matrix[0x8] * matrix[0x3] * matrix[0xD] -
		matrix[0xC] * matrix[0x1] * matrix[0xB] +
		matrix[0xC] * matrix[0x3] * matrix[0x9];

	inv[0xD] = matrix[0x0] * matrix[0x9] * matrix[0xE] -
		matrix[0x0] * matrix[0xA] * matrix[0xD] -
		matrix[0x8] * matrix[0x1] * matrix[0xE] +
		matrix[0x8] * matrix[0x2] * matrix[0xD] +
		matrix[0xC] * matrix[0x1] * matrix[0xA] -
		matrix[0xC] * matrix[0x2] * matrix[0x9];

	inv[0x2] = matrix[0x1] * matrix[0x6] * matrix[0xF] -
		matrix[0x1] * matrix[0x7] * matrix[0xE] -
		matrix[0x5] * matrix[0x2] * matrix[0xF] +
		matrix[0x5] * matrix[0x3] * matrix[0xE] +
		matrix[0xD] * matrix[0x2] * matrix[0x7] -
		matrix[0xD] * matrix[0x3] * matrix[0x6];

	inv[0x6] = -matrix[0x0] * matrix[0x6] * matrix[0xF] +
		matrix[0x0] * matrix[0x7] * matrix[0xE] +
		matrix[0x4] * matrix[0x2] * matrix[0xF] -
		matrix[0x4] * matrix[0x3] * matrix[0xE] -
		matrix[0xC] * matrix[0x2] * matrix[0x7] +
		matrix[0xC] * matrix[0x3] * matrix[0x6];

	inv[0xA] = matrix[0x0] * matrix[0x5] * matrix[0xF] -
		matrix[0x0] * matrix[0x7] * matrix[0xD] -
		matrix[0x4] * matrix[0x1] * matrix[0xF] +
		matrix[0x4] * matrix[0x3] * matrix[0xD] +
		matrix[0xC] * matrix[0x1] * matrix[0x7] -
		matrix[0xC] * matrix[0x3] * matrix[0x5];

	inv[0xE] = -matrix[0x0] * matrix[0x5] * matrix[0xE] +
		matrix[0x0] * matrix[0x6] * matrix[0xD] +
		matrix[0x4] * matrix[0x1] * matrix[0xE] -
		matrix[0x4] * matrix[0x2] * matrix[0xD] -
		matrix[0xC] * matrix[0x1] * matrix[0x6] +
		matrix[0xC] * matrix[0x2] * matrix[0x5];

	inv[0x3] = -matrix[0x1] * matrix[0x6] * matrix[0xB] +
		matrix[0x1] * matrix[0x7] * matrix[0xA] +
		matrix[0x5] * matrix[0x2] * matrix[0xB] -
		matrix[0x5] * matrix[0x3] * matrix[0xA] -
		matrix[0x9] * matrix[0x2] * matrix[0x7] +
		matrix[0x9] * matrix[0x3] * matrix[0x6];

	inv[0x7] = matrix[0x0] * matrix[0x6] * matrix[0xB] -
		matrix[0x0] * matrix[0x7] * matrix[0xA] -
		matrix[0x4] * matrix[0x2] * matrix[0xB] +
		matrix[0x4] * matrix[0x3] * matrix[0xA] +
		matrix[0x8] * matrix[0x2] * matrix[0x7] -
		matrix[0x8] * matrix[0x3] * matrix[0x6];

	inv[0xB] = -matrix[0x0] * matrix[0x5] * matrix[0xB] +
		matrix[0x0] * matrix[0x7] * matrix[0x9] +
		matrix[0x4] * matrix[0x1] * matrix[0xB] -
		matrix[0x4] * matrix[0x3] * matrix[0x9] -
		matrix[0x8] * matrix[0x1] * matrix[0x7] +
		matrix[0x8] * matrix[0x3] * matrix[0x5];

	inv[0xF] = matrix[0x0] * matrix[0x5] * matrix[0xA] -
		matrix[0x0] * matrix[0x6] * matrix[0x9] -
		matrix[0x4] * matrix[0x1] * matrix[0xA] +
		matrix[0x4] * matrix[0x2] * matrix[0x9] +
		matrix[0x8] * matrix[0x1] * matrix[0x6] -
		matrix[0x8] * matrix[0x2] * matrix[0x5];

	float det = matrix[0x0] * inv[0x0] + matrix[0x1] * inv[0x4] +
		matrix[0x2] * inv[0x8] + matrix[0x3] * inv[0xC];

	if( det == 0.0f )
	{
		m = float_m44_build_zeros();
		return m;
	}

	det = 1.0f / det;
	for( int i = 0; i < 16; i++ ) { m[i] = inv[i] * det; }
	return m;
}


float_m44 float_m44_multiply( const float_m44 &a, const float_m44 &b )
{
	float_m44 m;
	m[0x0] = a[0x0] * b[0x0] + a[0x4] * b[0x1] + a[0x8] * b[0x2] + a[0xC] * b[0x3];
	m[0x4] = a[0x0] * b[0x4] + a[0x4] * b[0x5] + a[0x8] * b[0x6] + a[0xC] * b[0x7];
	m[0x8] = a[0x0] * b[0x8] + a[0x4] * b[0x9] + a[0x8] * b[0xA] + a[0xC] * b[0xB];
	m[0xC] = a[0x0] * b[0xC] + a[0x4] * b[0xD] + a[0x8] * b[0xE] + a[0xC] * b[0xF];
	m[0x1] = a[0x1] * b[0x0] + a[0x5] * b[0x1] + a[0x9] * b[0x2] + a[0xD] * b[0x3];
	m[0x5] = a[0x1] * b[0x4] + a[0x5] * b[0x5] + a[0x9] * b[0x6] + a[0xD] * b[0x7];
	m[0x9] = a[0x1] * b[0x8] + a[0x5] * b[0x9] + a[0x9] * b[0xA] + a[0xD] * b[0xB];
	m[0xD] = a[0x1] * b[0xC] + a[0x5] * b[0xD] + a[0x9] * b[0xE] + a[0xD] * b[0xF];
	m[0x2] = a[0x2] * b[0x0] + a[0x6] * b[0x1] + a[0xA] * b[0x2] + a[0xE] * b[0x3];
	m[0x6] = a[0x2] * b[0x4] + a[0x6] * b[0x5] + a[0xA] * b[0x6] + a[0xE] * b[0x7];
	m[0xA] = a[0x2] * b[0x8] + a[0x6] * b[0x9] + a[0xA] * b[0xA] + a[0xE] * b[0xB];
	m[0xE] = a[0x2] * b[0xC] + a[0x6] * b[0xD] + a[0xA] * b[0xE] + a[0xE] * b[0xF];
	m[0x3] = a[0x3] * b[0x0] + a[0x7] * b[0x1] + a[0xB] * b[0x2] + a[0xF] * b[0x3];
	m[0x7] = a[0x3] * b[0x4] + a[0x7] * b[0x5] + a[0xB] * b[0x6] + a[0xF] * b[0x7];
	m[0xB] = a[0x3] * b[0x8] + a[0x7] * b[0x9] + a[0xB] * b[0xA] + a[0xF] * b[0xB];
	m[0xF] = a[0x3] * b[0xC] + a[0x7] * b[0xD] + a[0xB] * b[0xE] + a[0xF] * b[0xF];
	return m;
}


float_m44 float_m44_multiply_scalar( const float_m44 &a, const float scalar )
{
	float_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = a[i] * scalar; }
	return m;
}


float_m44 float_m44_add( const float_m44 &a, const float_m44 &b )
{
	float_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = a[i] + b[i]; }
	return m;
}


float_m44 float_m44_sub( const float_m44 &a, const float_m44 &b )
{
	float_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = a[i] - b[i]; }
	return m;
}


float_m44 float_m44_build_zeros()
{
	float_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = 0.0f; }
	return m;
}


float_m44 float_m44_build_identity()
{
	float_m44 m = float_m44_build_zeros();
	m[0x0] = 1.0f;
	m[0x5] = 1.0f;
	m[0xA] = 1.0f;
	m[0xF] = 1.0f;
	return m;
}


float_m44 float_m44_build_scaling( float xscale, float yscale, float zscale )
{
	float_m44 m = float_m44_build_zeros();
	m[0x0] = xscale;
	m[0x5] = yscale;
	m[0xA] = zscale;
	m[0xF] = 1.0f;
	return m;
}


float_m44 float_m44_build_translation( float xtrans, float ytrans, float ztrans )
{
	float_m44 m = float_m44_build_identity();
	m[0xC] = xtrans;
	m[0xD] = ytrans;
	m[0xE] = ztrans;
	return m;
}


float_m44 float_m44_build_rotation_x( float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	float_m44 m = float_m44_build_identity();
	m[0x5] = c;
	m[0x6] = -s;
	m[0x9] = s;
	m[0xA] = c;
	return m;
}


float_m44 float_m44_build_rotation_y( float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	float_m44 m = float_m44_build_identity();
	m[0x0] = c;
	m[0x2] = s;
	m[0x8] = -s;
	m[0xA] = c;
	return m;
}


float_m44 float_m44_build_rotation_z( float rad )
{
	float s = static_cast<float>( sinf( rad ) );
	float c = static_cast<float>( cosf( rad ) );

	float_m44 m = float_m44_build_identity();
	m[0x0] = c;
	m[0x1] = -s;
	m[0x4] = s;
	m[0x5] = c;
	return m;
}


float_m44 float_m44_build_rotation_axis( float x, float y, float z, float rad )
{
	const double c = cosf( rad );
	const double s = sinf( rad );
	const double t = 1.0f - c;

	float_m44 m = float_m44_build_identity();
	m[0x0] = t * x * x + c;
	m[0x4] = t * x * y - s * z;
	m[0x8] = t * x * z + s * y;
	m[0xC] = 0.0f;
	m[0x1] = t * x * y + s * z;
	m[0x5] = t * y * y + c;
	m[0x9] = t * y * z - s * x;
	m[0xD] = 0.0f;
	m[0x2] = t * x * z - s * y;
	m[0x6] = t * y * z + s * x;
	m[0xA] = t * z * z + c;
	m[0xE] = 0.0f;
	m[0x3] = 0.0f;
	m[0x7] = 0.0f;
	m[0xB] = 0.0f;
	m[0xF] = 1.0f;
	return m;
}

float_m44 float_m44_build_basis( float fX, float fY, float fZ, float rX, float rY, float rZ,
	float uX, float uY, float uZ )
{
	float_m44 m = float_m44_build_zeros();
	m[0x0] = rX;
	m[0x1] = rY;
	m[0x2] = rZ;
	m[0x3] = 0.0f;
	m[0x4] = uX;
	m[0x5] = uY;
	m[0x6] = uZ;
	m[0x7] = 0.0f;
	m[0x8] = fX;
	m[0x9] = fY;
	m[0xA] = fZ;
	m[0xB] = 0.0f;
	m[0xC] = 0.0f;
	m[0xD] = 0.0f;
	m[0xE] = 0.0f;
	m[0xF] = 1.0f;
	return m;
}


float_m44 float_m44_build_orthographic( float left, float right, float top, float bottom,
	float znear, float zfar )
{
	float_m44 m = float_m44_build_zeros();
	m[0x0] = 2.0f / ( right - left );
	m[0x1] = 0.0f;
	m[0x2] = 0.0f;
	m[0x3] = 0.0f;
	m[0x4] = 0.0f;
	m[0x5] = 2.0f / ( top - bottom );
	m[0x6] = 0.0f;
	m[0x7] = 0.0f;
	m[0x8] = 0.0f;
	m[0x9] = 0.0f;

#if DEPTH_RANGE_0_TO_1
	m[0xA] = 1.0f / ( zfar - znear );
#else
	m[0xA] = 2.0f / ( zfar - znear );
#endif

	m[0xB] = 0.0f;
	m[0xC] = ( left + right ) / ( left - right );
	m[0xD] = ( top + bottom ) / ( bottom - top );

#if DEPTH_RANGE_0_TO_1
	m[0xE] = znear / ( znear - zfar );
#else
	m[0xE] = ( zfar + znear ) / ( znear - zfar );
#endif

	m[0xF] = 1.0f;
	return m;
}


float_m44 float_m44_build_perspective( float fov, float aspect, float znear, float zfar )
{
	const float x = 1.0f / tanf( fov * ( 0.5f * DEG2RAD_F ) );

	float_m44 m;
	m[0x0] =  x / aspect;
	m[0x1] =  0.0f;
	m[0x2] =  0.0f;
	m[0x3] =  0.0f;
	m[0x4] =  0.0f;
	m[0x5] =  x;
	m[0x6] =  0.0f;
	m[0x7] =  0.0f;
	m[0x8] =  0.0f;
	m[0x9] =  0.0f;
#if DEPTH_RANGE_0_TO_1
	m[0xA] = zfar / ( znear - zfar );
	m[0xB] = -1.0f;
	m[0xC] =  0.0f;
	m[0xD] =  0.0f;
	m[0xE] = ( znear * zfar ) / ( znear - zfar );
#else
	m[0xA] = ( zfar + znear ) / ( znear - zfar );
	m[0xB] = -1.0f;
	m[0xC] =  0.0f;
	m[0xD] =  0.0f;
	m[0xE] = ( 2.0f * znear * zfar ) / ( znear - zfar );
#endif
	m[0xF] =  0.0f;
	return m;
}


float_m44 float_m44_build_lookat( float x, float y, float z, float xto, float yto, float zto,
	float xup, float yup, float zup )
{
	float fX = xto - x;
	float fY = yto - y;
	float fZ = zto - z;
	float invLength = 1.0f / sqrt( fX * fX + fY * fY + fZ * fZ );
	fX *= invLength;
	fY *= invLength;
	fZ *= invLength;

	float rX = yup * fZ - zup * fY;
	float rY = zup * fX - xup * fZ;
	float rZ = xup * fY - yup * fX;
	invLength = 1.0f / sqrt( rX * rX + rY * rY + rZ * rZ );
	rX *= invLength;
	rY *= invLength;
	rZ *= invLength;

	float uX = fY * rZ - fZ * rY;
	float uY = fZ * rX - fX * rZ;
	float uZ = fX * rY - fY * rX;
	invLength = 1.0f / sqrt( uX * uX + uY * uY + uZ * uZ );
	uX *= invLength;
	uY *= invLength;
	uZ *= invLength;

	float_m44 m = float_m44_build_zeros();
	m[0x0] = rX;
	m[0x1] = uX;
	m[0x2] = -fX;
	m[0x3] = 0.0f;
	m[0x4] = rY;
	m[0x5] = uY;
	m[0x6] = -fY;
	m[0x7] = 0.0f;
	m[0x8] = rZ;
	m[0x9] = uZ;
	m[0xA] = -fZ;
	m[0xB] = 0.0f;
	m[0xC] = -( rX * x + rY * y + rZ * z );
	m[0xD] = -( uX * x + uY * y + uZ * z );
	m[0xE] = fX * x + fY * y + fZ * z;
	m[0xF] = 1.0f;
	return m;
}


float_m44 float_m44_build_ndc( float width, float height )
{
	float_m44 m;
	m[0x0] = width * 0.5;
	m[0x5] = height * -0.5;
	m[0xA] = 1.0f;
	m[0xC] = width * 0.5;
	m[0xD] = height * 0.5;
	m[0xF] = 1.0f;
	return m;
}


float_m44 float_m44_from_double_m44( const double_m44 &matrix )
{
	float_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<float>( matrix.data[i] ); }
	return result;
}


float_m44 float_m44_extract_rotation_forward_x( const float_m44 &matrix )
{
	// NOTE: Assumption: +X forward, +Y right, +Z up
	float_m44 m = float_m44_build_identity();
	m[0x0] = -matrix[0x2];
	m[0x1] = -matrix[0x6];
	m[0x2] = -matrix[0xA];
	m[0x4] = matrix[0x0];
	m[0x5] = matrix[0x4];
	m[0x6] = matrix[0x8];
	m[0x8] = matrix[0x1];
	m[0x9] = matrix[0x5];
	m[0xA] = matrix[0x9];
	return m;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4x4 Matrix (Double)

double_m44::double_m44()
{
	for( int i = 0; i < 16; i++ ) { data[i] = 0.0; }
}


double_m44::double_m44( const float_m44 &matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<double>( matrix.data[i] ); }
}


double_m44::double_m44( float_m44 &&matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = static_cast<double>( matrix.data[i] ); }
}


double_m44::double_m44( const double_m44 &matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; }
}


double_m44::double_m44( double_m44 &&matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; }
}


double_m44 &double_m44::operator=( const double_m44 &matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; } return *this;
}


double_m44 &double_m44::operator=( double_m44 &&matrix )
{
	for( int i = 0; i < 16; i++ ) { data[i] = matrix.data[i]; } return *this;
}


double_m44::operator float_m44() const
{
	float_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<float>( data[i] ); }
	return result;
}


bool double_m44_equal( const double_m44 &a, const double_m44 &b )
{
	return a[0x0] == b[0x0] &&
		a[0x1] == b[0x1] &&
		a[0x2] == b[0x2] &&
		a[0x3] == b[0x3] &&
		a[0x4] == b[0x4] &&
		a[0x5] == b[0x5] &&
		a[0x6] == b[0x6] &&
		a[0x7] == b[0x7] &&
		a[0x8] == b[0x8] &&
		a[0x9] == b[0x9] &&
		a[0xA] == b[0xA] &&
		a[0xB] == b[0xB] &&
		a[0xC] == b[0xC] &&
		a[0xD] == b[0xD] &&
		a[0xE] == b[0xE] &&
		a[0xF] == b[0xF];
}


double_m44 float_m44_transpose( const double_m44 &matrix )
{
	double_m44 m;
	m[0x0] = matrix[0x0];
	m[0x1] = matrix[0x4];
	m[0x2] = matrix[0x8];
	m[0x3] = matrix[0xC];
	m[0x4] = matrix[0x1];
	m[0x5] = matrix[0x5];
	m[0x6] = matrix[0x9];
	m[0x7] = matrix[0xD];
	m[0x8] = matrix[0x2];
	m[0x9] = matrix[0x6];
	m[0xA] = matrix[0xA];
	m[0xB] = matrix[0xE];
	m[0xC] = matrix[0x3];
	m[0xD] = matrix[0x7];
	m[0xE] = matrix[0xB];
	m[0xF] = matrix[0xF];
	return m;
}


double_m44 double_m44_inverse( const double_m44 &matrix )
{
	double_m44 m;
	double_m44 inv;

	inv[0x0] = matrix[0x5] * matrix[0xA] * matrix[0xF] -
		matrix[0x5] * matrix[0xB] * matrix[0xE] -
		matrix[0x9] * matrix[0x6] * matrix[0xF] +
		matrix[0x9] * matrix[0x7] * matrix[0xE] +
		matrix[0xD] * matrix[0x6] * matrix[0xB] -
		matrix[0xD] * matrix[0x7] * matrix[0xA];

	inv[0x4] = -matrix[0x4] * matrix[0xA] * matrix[0xF] +
		matrix[0x4] * matrix[0xB] * matrix[0xE] +
		matrix[0x8] * matrix[0x6] * matrix[0xF] -
		matrix[0x8] * matrix[0x7] * matrix[0xE] -
		matrix[0xC] * matrix[0x6] * matrix[0xB] +
		matrix[0xC] * matrix[0x7] * matrix[0xA];

	inv[0x8] = matrix[0x4] * matrix[0x9] * matrix[0xF] -
		matrix[0x4] * matrix[0xB] * matrix[0xD] -
		matrix[0x8] * matrix[0x5] * matrix[0xF] +
		matrix[0x8] * matrix[0x7] * matrix[0xD] +
		matrix[0xC] * matrix[0x5] * matrix[0xB] -
		matrix[0xC] * matrix[0x7] * matrix[0x9];

	inv[0xC] = -matrix[0x4] * matrix[0x9] * matrix[0xE] +
		matrix[0x4] * matrix[0xA] * matrix[0xD] +
		matrix[0x8] * matrix[0x5] * matrix[0xE] -
		matrix[0x8] * matrix[0x6] * matrix[0xD] -
		matrix[0xC] * matrix[0x5] * matrix[0xA] +
		matrix[0xC] * matrix[0x6] * matrix[0x9];

	inv[0x1] = -matrix[0x1] * matrix[0xA] * matrix[0xF] +
		matrix[0x1] * matrix[0xB] * matrix[0xE] +
		matrix[0x9] * matrix[0x2] * matrix[0xF] -
		matrix[0x9] * matrix[0x3] * matrix[0xE] -
		matrix[0xD] * matrix[0x2] * matrix[0xB] +
		matrix[0xD] * matrix[0x3] * matrix[0xA];

	inv[0x5] = matrix[0x0] * matrix[0xA] * matrix[0xF] -
		matrix[0x0] * matrix[0xB] * matrix[0xE] -
		matrix[0x8] * matrix[0x2] * matrix[0xF] +
		matrix[0x8] * matrix[0x3] * matrix[0xE] +
		matrix[0xC] * matrix[0x2] * matrix[0xB] -
		matrix[0xC] * matrix[0x3] * matrix[0xA];

	inv[0x9] = -matrix[0x0] * matrix[0x9] * matrix[0xF] +
		matrix[0x0] * matrix[0xB] * matrix[0xD] +
		matrix[0x8] * matrix[0x1] * matrix[0xF] -
		matrix[0x8] * matrix[0x3] * matrix[0xD] -
		matrix[0xC] * matrix[0x1] * matrix[0xB] +
		matrix[0xC] * matrix[0x3] * matrix[0x9];

	inv[0xD] = matrix[0x0] * matrix[0x9] * matrix[0xE] -
		matrix[0x0] * matrix[0xA] * matrix[0xD] -
		matrix[0x8] * matrix[0x1] * matrix[0xE] +
		matrix[0x8] * matrix[0x2] * matrix[0xD] +
		matrix[0xC] * matrix[0x1] * matrix[0xA] -
		matrix[0xC] * matrix[0x2] * matrix[0x9];

	inv[0x2] = matrix[0x1] * matrix[0x6] * matrix[0xF] -
		matrix[0x1] * matrix[0x7] * matrix[0xE] -
		matrix[0x5] * matrix[0x2] * matrix[0xF] +
		matrix[0x5] * matrix[0x3] * matrix[0xE] +
		matrix[0xD] * matrix[0x2] * matrix[0x7] -
		matrix[0xD] * matrix[0x3] * matrix[0x6];

	inv[0x6] = -matrix[0x0] * matrix[0x6] * matrix[0xF] +
		matrix[0x0] * matrix[0x7] * matrix[0xE] +
		matrix[0x4] * matrix[0x2] * matrix[0xF] -
		matrix[0x4] * matrix[0x3] * matrix[0xE] -
		matrix[0xC] * matrix[0x2] * matrix[0x7] +
		matrix[0xC] * matrix[0x3] * matrix[0x6];

	inv[0xA] = matrix[0x0] * matrix[0x5] * matrix[0xF] -
		matrix[0x0] * matrix[0x7] * matrix[0xD] -
		matrix[0x4] * matrix[0x1] * matrix[0xF] +
		matrix[0x4] * matrix[0x3] * matrix[0xD] +
		matrix[0xC] * matrix[0x1] * matrix[0x7] -
		matrix[0xC] * matrix[0x3] * matrix[0x5];

	inv[0xE] = -matrix[0x0] * matrix[0x5] * matrix[0xE] +
		matrix[0x0] * matrix[0x6] * matrix[0xD] +
		matrix[0x4] * matrix[0x1] * matrix[0xE] -
		matrix[0x4] * matrix[0x2] * matrix[0xD] -
		matrix[0xC] * matrix[0x1] * matrix[0x6] +
		matrix[0xC] * matrix[0x2] * matrix[0x5];

	inv[0x3] = -matrix[0x1] * matrix[0x6] * matrix[0xB] +
		matrix[0x1] * matrix[0x7] * matrix[0xA] +
		matrix[0x5] * matrix[0x2] * matrix[0xB] -
		matrix[0x5] * matrix[0x3] * matrix[0xA] -
		matrix[0x9] * matrix[0x2] * matrix[0x7] +
		matrix[0x9] * matrix[0x3] * matrix[0x6];

	inv[0x7] = matrix[0x0] * matrix[0x6] * matrix[0xB] -
		matrix[0x0] * matrix[0x7] * matrix[0xA] -
		matrix[0x4] * matrix[0x2] * matrix[0xB] +
		matrix[0x4] * matrix[0x3] * matrix[0xA] +
		matrix[0x8] * matrix[0x2] * matrix[0x7] -
		matrix[0x8] * matrix[0x3] * matrix[0x6];

	inv[0xB] = -matrix[0x0] * matrix[0x5] * matrix[0xB] +
		matrix[0x0] * matrix[0x7] * matrix[0x9] +
		matrix[0x4] * matrix[0x1] * matrix[0xB] -
		matrix[0x4] * matrix[0x3] * matrix[0x9] -
		matrix[0x8] * matrix[0x1] * matrix[0x7] +
		matrix[0x8] * matrix[0x3] * matrix[0x5];

	inv[0xF] = matrix[0x0] * matrix[0x5] * matrix[0xA] -
		matrix[0x0] * matrix[0x6] * matrix[0x9] -
		matrix[0x4] * matrix[0x1] * matrix[0xA] +
		matrix[0x4] * matrix[0x2] * matrix[0x9] +
		matrix[0x8] * matrix[0x1] * matrix[0x6] -
		matrix[0x8] * matrix[0x2] * matrix[0x5];

	double det = matrix[0x0] * inv[0x0] + matrix[0x1] * inv[0x4] +
		matrix[0x2] * inv[0x8] + matrix[0x3] * inv[0xC];

	if( det == 0.0 )
	{
		m = double_m44_build_zeros();
		return m;
	}

	det = 1.0 / det;
	for( int i = 0; i < 16; i++ ) { m[i] = inv[i] * det; }
	return m;
}


double_m44 double_m44_multiply( const double_m44 &a, const double_m44 &b )
{
	double_m44 m;
	m[0x0] = a[0x0] * b[0x0] + a[0x4] * b[0x1] + a[0x8] * b[0x2] + a[0xC] * b[0x3];
	m[0x4] = a[0x0] * b[0x4] + a[0x4] * b[0x5] + a[0x8] * b[0x6] + a[0xC] * b[0x7];
	m[0x8] = a[0x0] * b[0x8] + a[0x4] * b[0x9] + a[0x8] * b[0xA] + a[0xC] * b[0xB];
	m[0xC] = a[0x0] * b[0xC] + a[0x4] * b[0xD] + a[0x8] * b[0xE] + a[0xC] * b[0xF];
	m[0x1] = a[0x1] * b[0x0] + a[0x5] * b[0x1] + a[0x9] * b[0x2] + a[0xD] * b[0x3];
	m[0x5] = a[0x1] * b[0x4] + a[0x5] * b[0x5] + a[0x9] * b[0x6] + a[0xD] * b[0x7];
	m[0x9] = a[0x1] * b[0x8] + a[0x5] * b[0x9] + a[0x9] * b[0xA] + a[0xD] * b[0xB];
	m[0xD] = a[0x1] * b[0xC] + a[0x5] * b[0xD] + a[0x9] * b[0xE] + a[0xD] * b[0xF];
	m[0x2] = a[0x2] * b[0x0] + a[0x6] * b[0x1] + a[0xA] * b[0x2] + a[0xE] * b[0x3];
	m[0x6] = a[0x2] * b[0x4] + a[0x6] * b[0x5] + a[0xA] * b[0x6] + a[0xE] * b[0x7];
	m[0xA] = a[0x2] * b[0x8] + a[0x6] * b[0x9] + a[0xA] * b[0xA] + a[0xE] * b[0xB];
	m[0xE] = a[0x2] * b[0xC] + a[0x6] * b[0xD] + a[0xA] * b[0xE] + a[0xE] * b[0xF];
	m[0x3] = a[0x3] * b[0x0] + a[0x7] * b[0x1] + a[0xB] * b[0x2] + a[0xF] * b[0x3];
	m[0x7] = a[0x3] * b[0x4] + a[0x7] * b[0x5] + a[0xB] * b[0x6] + a[0xF] * b[0x7];
	m[0xB] = a[0x3] * b[0x8] + a[0x7] * b[0x9] + a[0xB] * b[0xA] + a[0xF] * b[0xB];
	m[0xF] = a[0x3] * b[0xC] + a[0x7] * b[0xD] + a[0xB] * b[0xE] + a[0xF] * b[0xF];
	return m;
}


double_m44 double_m44_multiply_scalar( const double_m44 &a, double scalar )
{
	double_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = a[i] * scalar; }
	return m;
}


double_m44 double_m44_add( const double_m44 &a, const double_m44 &b )
{
	double_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = a[i] + b[i]; }
	return m;
}


double_m44 double_m44_sub( const double_m44 &a, const double_m44 &b )
{
	double_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = a[i] - b[i]; }
	return m;
}


double_m44 double_m44_build_zeros()
{
	double_m44 m;
	for( int i = 0; i < 16; i++ ) { m[i] = 0.0; }
	return m;
}


double_m44 double_m44_build_identity()
{
	double_m44 m = double_m44_build_zeros();
	m[0x0] = 1.0;
	m[0x5] = 1.0;
	m[0xA] = 1.0;
	m[0xF] = 1.0;
	return m;
}


double_m44 double_m44_build_scaling( double xscale, double yscale, double zscale )
{
	double_m44 m = double_m44_build_zeros();
	m[0x0] = xscale;
	m[0x5] = yscale;
	m[0xA] = zscale;
	m[0xF] = 1.0;
	return m;
}


double_m44 double_m44_build_translation( double xtrans, double ytrans, double ztrans )
{
	double_m44 m = double_m44_build_identity();
	m[0xC] = xtrans;
	m[0xD] = ytrans;
	m[0xE] = ztrans;
	return m;
}


double_m44 double_m44_build_rotation_x( double rad )
{
	double c = cos( rad );
	double s = sin( rad );

	double_m44 m = double_m44_build_identity();
	m[0x5] = c;
	m[0x6] = s;
	m[0x9] = -s;
	m[0xA] = c;
	return m;
}


double_m44 double_m44_build_rotation_y( double rad )
{
	double c = cos( rad );
	double s = sin( rad );

	double_m44 m = double_m44_build_identity();
	m[0x0] = c;
	m[0x2] = -s;
	m[0x8] = s;
	m[0xA] = c;
	return m;
}


double_m44 double_m44_build_rotation_z( double rad )
{
	double c = cos( rad );
	double s = sin( rad );

	double_m44 m = double_m44_build_identity();
	m[0x0] = c;
	m[0x1] = s;
	m[0x4] = -s;
	m[0x5] = c;
	return m;
}


double_m44 double_m44_build_rotation_axis( double x, double y, double z, double rad )
{
	const double c = cos( rad );
	const double s = sin( rad );
	const double t = 1.0 - c;

	double_m44 m = double_m44_build_identity();
	m[0x0] = t * x * x + c;
	m[0x4] = t * x * y - s * z;
	m[0x8] = t * x * z + s * y;
	m[0xC] = 0.0;
	m[0x1] = t * x * y + s * z;
	m[0x5] = t * y * y + c;
	m[0x9] = t * y * z - s * x;
	m[0xD] = 0.0;
	m[0x2] = t * x * z - s * y;
	m[0x6] = t * y * z + s * x;
	m[0xA] = t * z * z + c;
	m[0xE] = 0.0;
	m[0x3] = 0.0;
	m[0x7] = 0.0;
	m[0xB] = 0.0;
	m[0xF] = 1.0;
	return m;
}

double_m44 double_m44_build_basis( double fX, double fY, double fZ, double rX, double rY, double rZ,
	double uX, double uY, double uZ )
{
	double_m44 m = double_m44_build_zeros();
	m[0x0] = rX;
	m[0x1] = rY;
	m[0x2] = rZ;
	m[0x3] = 0.0;
	m[0x4] = uX;
	m[0x5] = uY;
	m[0x6] = uZ;
	m[0x7] = 0.0;
	m[0x8] = fX;
	m[0x9] = fY;
	m[0xA] = fZ;
	m[0xB] = 0.0;
	m[0xC] = 0.0;
	m[0xD] = 0.0;
	m[0xE] = 0.0;
	m[0xF] = 1.0;
	return m;
}


double_m44 double_m44_build_orthographic( double left, double right, double top, double bottom,
	double znear, double zfar )
{
	double_m44 m = double_m44_build_zeros();
	m[0x0] = 2.0 / ( right - left );
	m[0x1] = 0.0;
	m[0x2] = 0.0;
	m[0x3] = 0.0;
	m[0x4] = 0.0;
	m[0x5] = 2.0 / ( top - bottom );
	m[0x6] = 0.0;
	m[0x7] = 0.0;
	m[0x8] = 0.0;
	m[0x9] = 0.0;

	#if DEPTH_RANGE_0_TO_1
		m[0xA] = 1.0 / ( zfar - znear );
	#else
		m[0xA] = 2.0 / ( zfar - znear );
	#endif

	m[0xB] = 0.0f;
	m[0xC] = ( left + right ) / ( left - right );
	m[0xD] = ( top + bottom ) / ( bottom - top );

	#if DEPTH_RANGE_0_TO_1
		m[0xE] = znear / ( znear - zfar );
	#else
		m[0xE] = ( zfar + znear ) / ( znear - zfar );
	#endif

	m[0xF] = 1.0;
	return m;
}


double_m44 double_m44_build_perspective( double fov, double aspect, double znear, double zfar )
{
	const double x = 1.0 / tan( fov * ( 0.5 * DEG2RAD ) );

	double_m44 m;
	m[0x0] =  x / aspect;
	m[0x1] =  0.0;
	m[0x2] =  0.0;
	m[0x3] =  0.0;
	m[0x4] =  0.0;
	m[0x5] =  x;
	m[0x6] =  0.0;
	m[0x7] =  0.0;
	m[0x8] =  0.0;
	m[0x9] =  0.0;
#if DEPTH_RANGE_0_TO_1
	m[0xA] = zfar / ( znear - zfar );
	m[0xB] = -1.0;
	m[0xC] =  0.0;
	m[0xD] =  0.0;
	m[0xE] = ( znear * zfar ) / ( znear - zfar );
#else
	m[0xA] = ( zfar + znear ) / ( znear - zfar );
	m[0xB] = -1.0;
	m[0xC] =  0.0;
	m[0xD] =  0.0;
	m[0xE] = ( 2.0 * znear * zfar ) / ( znear - zfar );
#endif
	m[0xF] =  0.0;
	return m;
}


double_m44 double_m44_build_lookat( double x, double y, double z, double xto, double yto, double zto,
	double xup, double yup, double zup )
{
	double fX = xto - x;
	double fY = yto - y;
	double fZ = zto - z;
	double invLength = 1.0 / sqrt( fX * fX + fY * fY + fZ * fZ );
	fX *= invLength;
	fY *= invLength;
	fZ *= invLength;

	double rX = yup * fZ - zup * fY;
	double rY = zup * fX - xup * fZ;
	double rZ = xup * fY - yup * fX;
	invLength = 1.0 / sqrt( rX * rX + rY * rY + rZ * rZ );
	rX *= invLength;
	rY *= invLength;
	rZ *= invLength;

	double uX = fY * rZ - fZ * rY;
	double uY = fZ * rX - fX * rZ;
	double uZ = fX * rY - fY * rX;
	invLength = 1.0 / sqrt( uX * uX + uY * uY + uZ * uZ );
	uX *= invLength;
	uY *= invLength;
	uZ *= invLength;

	double_m44 m = double_m44_build_zeros();
	m[0x0] = rX;
	m[0x1] = uX;
	m[0x2] = -fX;
	m[0x3] = 0.0;
	m[0x4] = rY;
	m[0x5] = uY;
	m[0x6] = -fY;
	m[0x7] = 0.0;
	m[0x8] = rZ;
	m[0x9] = uZ;
	m[0xA] = -fZ;
	m[0xB] = 0.0;
	m[0xC] = -( rX * x + rY * y + rZ * z );
	m[0xD] = -( uX * x + uY * y + uZ * z );
	m[0xE] = fX * x + fY * y + fZ * z;
	m[0xF] = 1.0;
	return m;
}


double_m44 double_m44_build_ndc( double width, double height )
{
	double_m44 m;
	m[0x0] = width * 0.5;
	m[0x5] = height * -0.5;
	m[0xA] = 1.0;
	m[0xC] = width * 0.5;
	m[0xD] = height * 0.5;
	m[0xF] = 1.0;
	return m;
}


double_m44 double_m44_from_float_m44( const double_m44 &matrix )
{
	double_m44 result;
	for( int i = 0; i < 16; ++i ) { result.data[i] = static_cast<double>( matrix.data[i] ); }
	return result;
}


double_m44 double_m44_extract_rotation_forward_x( const double_m44 &matrix )
{
	// NOTE: Assumption: +X forward, +Y right, +Z up
	double_m44 m = double_m44_build_identity();
	m[0x0] = -matrix[0x2];
	m[0x1] = -matrix[0x6];
	m[0x2] = -matrix[0xA];
	m[0x4] = matrix[0x0];
	m[0x5] = matrix[0x4];
	m[0x6] = matrix[0x8];
	m[0x8] = matrix[0x1];
	m[0x9] = matrix[0x5];
	m[0xA] = matrix[0x9];
	return m;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RadialFloatGraph::add_node( RadialFloatNode node )
{
	if( count >= FLOAT_GRAPH_COUNT ) { return -1; }

	int insertIndex = 0;
	while( insertIndex < count && nodes[insertIndex].time < node.time ) { insertIndex++; }

	if( insertIndex < count )
	{
		memory_move( &nodes[insertIndex + 1], &nodes[insertIndex],
			( count - insertIndex ) * sizeof( RadialFloatNode ) );
	}

	nodes[insertIndex] = node;
	count++;
	return insertIndex;
}


void RadialFloatGraph::remove_node( int index )
{
	if( index < count - 1 )
	{
		memory_move( &nodes[index], &nodes[index + 1],
			( count - index - 1 ) * sizeof( RadialFloatNode ) );
	}

	count--;
}


float RadialFloatGraph::get_value( float time ) const
{
	if( count == 0 ) { return 0.0f; }
	if( count == 1 ) { return nodes[0].value; }
	const float t = fmod( time, 1.0f );

	int right = -1;
	for( int i = 0; i < count; i++ )
	{
		if( nodes[i].time == t )
		{
			return nodes[i].value;
		}

		if( nodes[i].time > t )
		{
			right = i;
			break;
		}
	}

	int left;
	float span;
	float localT;

	if( right == -1 )
	{
		left = count - 1;
		right = 0;
		const float t0 = nodes[left].time;
		const float t1 = nodes[right].time + 1.0f;
		span = t1 - t0;
		localT = ( t - t0 ) / span;
	}
	else
	{
		left = right - 1;
		if( left < 0 )
		{
			left = count - 1;
			const float t0 = nodes[left].time;
			const float t1 = nodes[right].time;
			span = ( t1 + 1.0f ) - t0;
			localT = ( t + 1.0f - t0 ) / span;
		}
		else
		{
			const float t0 = nodes[left].time;
			const float t1 = nodes[right].time;
			span = t1 - t0;
			localT = ( t - t0 ) / span;
		}
	}

	if( span <= 0.0f ) { return nodes[left].value; }
	return lerp( nodes[left].value, nodes[right].value, localT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////