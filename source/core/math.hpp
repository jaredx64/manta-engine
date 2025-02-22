#pragma once

#include <core/types.hpp>

#include <vendor/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PI ( 3.14159265358979f )
#define DEG2RAD ( PI / 180.0f )
#define RAD2DEG ( 180.0f / PI )

#define ALIGN_16( value ) ( ( static_cast<int>( value ) >> 4 ) << 4 )
#define ALIGN_16_CEIL( value ) ( ( ( static_cast<int>( value ) >> 4 ) + 1 ) << 4 )
#define ALIGN_POWER( value, power ) ( ( static_cast<int>( value ) >> power ) << power )
#define ALIGN_POWER_CEIL( value, power ) ( ( ( static_cast<int>( value ) >> power ) + 1 ) << power )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Matrix;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline float fast_fmod(float x, float y)
{
	return x - ( static_cast<int>( x / y ) ) * y;
}


inline int fast_floor( float x )
{
	int    i = static_cast<int>( x );
	return i - ( i > x );
}


inline u32 ceilpow2( u32 v )
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


inline float degtorad( float degree )
{
	return degree * ( PI / 180.0f );
}


inline float radtodeg( float radian )
{
	return radian * ( 180.0f / PI );
}


template <typename T> inline T lengthdir_x( T dist, float angle )
{
	return static_cast<T>( static_cast<float>( dist ) * cosf( degtorad( angle ) ) );
}


template <typename T> inline T lengthdir_y( T dist, float angle )
{
	return static_cast<T>( static_cast<float>( dist ) * sinf( degtorad( angle ) ) );
}


extern void fast_sin_cos( const float angle, float &sin, float &cos );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> inline T min( const T val1, const T val2 )
{
	return ( val1 < val2 ) ? val1 : val2;
}


template <typename T> inline T max( const T val1, const T val2 )
{
	return ( val1 > val2 ) ? val1 : val2;
}


template <typename T> inline T clamp( T value, const T min, const T max )
{
	if( value < min ) { value = min; }
	if( value > max ) { value = max; }
	return value;
}


template <typename T> inline T wrap( const T &value, const T &min, const T &max )
{
	return ( value > max ? min : ( value < min ? max : value ) );
}


template <typename T> inline void wrap( const T &value, const T &min, const T &max, T &outValue )
{
	outValue = ( value > max ? min : ( value < min ? max : value ) );
}


template <typename T> inline T lerp( const T value1, const T value2, const float amount )
{
	//if( amount <= 0.0f ) { return value1; }
	//if( amount >= 1.0f ) { return value2; }
	return static_cast<T>( value1 + ( value2 - value1 ) * amount );
}


template <typename T> inline void lerp( const T &value1, const T value2, const float amount, T &result )
{
	//if( amount <= 0.0f ) { result = value1; return; }
	//if( amount >= 1.0f ) { result = value2; return; }
	result = static_cast<T>( value1 + ( value2 - value1 ) * amount );
}


template <typename T> inline T lerp_delta( const T &value1, const T value2, const float amount, const Delta delta )
{
	return lerp( value1, value2, 1.0f - expf( -amount * delta ) );
}


inline float lerp_angle_degrees( float value1, float value2, float amount )
{
	value1 *= DEG2RAD;
	value2 *= DEG2RAD;
	float dtheta = value2 - value1;
	if( dtheta > PI ) { value1 += 2.0f * PI; }
	if( dtheta < -PI ) { value1 -= 2.0f * PI; }
	return lerp( value1, value2, amount ) * RAD2DEG;
}


inline float lerp_angle_radians( float value1, float value2, float amount )
{
	float dtheta = value2 - value1;
	if( dtheta > PI ) { value1 += 2.0f * PI; }
	if( dtheta < -PI ) { value1 -= 2.0f * PI; }
	return lerp( value1, value2, amount ) * RAD2DEG;
}


template <typename T>
inline bool tween( T &variable, const T target, const T factor, const Delta delta, const float threshold = 0.25f )
{
	if( fabs( static_cast<double>( variable - target ) ) < static_cast<double>( threshold ) )
	{
		variable = target;
		return false;
	}
	else
	{
		variable += static_cast<T>( static_cast<float>( target - variable ) /
		                       max( static_cast<float>( factor / delta ), 1.0f ) );
		return true;
	}
}

#if !USE_OFFICIAL_HEADERS
inline float fabsf( const float value ) { return static_cast<float>( fabs( value ) ); }
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool matrix_equal( const Matrix &a, const Matrix &b );

extern Matrix matrix_transpose( const Matrix &m );

extern Matrix matrix_inverse( const Matrix &m );

extern Matrix matrix_multiply( const Matrix &a, const Matrix &b );

extern Matrix matrix_multiply_scalar( const Matrix &a, const float scalar );

extern Matrix matrix_add( const Matrix &a, const Matrix &b );

extern Matrix matrix_sub( const Matrix &a, const Matrix &b );

extern Matrix matrix_build_zeros();

extern Matrix matrix_build_identity();

extern Matrix matrix_build_scaling( const float xscale, const float yscale, const float zscale );

extern Matrix matrix_build_translation( const float xtrans, const float ytrans, const float ztrans );

extern Matrix matrix_build_rotation_x( const float rad );

extern Matrix matrix_build_rotation_y( const float rad );

extern Matrix matrix_build_rotation_z( const float rad );

extern Matrix matrix_build_orthographic( float left, float right, float top, float bottom, float znear, float zfar );

extern Matrix matrix_build_perspective( const float fov, const float aspect, const float znear, const float zfar );

extern Matrix matrix_build_lookat( const float x, const float y, const float z,
	const float xto, const float yto, const float zto,
	const float xup, const float yup, const float zup );

extern Matrix matrix_build_ndc( const float width, const float height );

struct Matrix
{
	float data[16];

	Matrix() { for( int i = 0; i < 16; i++ ) { data[i] = 0.0f; } }
	Matrix( const Matrix &m ) { for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } }
	Matrix( Matrix &&m ) { for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } }
	Matrix &operator=( const Matrix &m ) { for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } return *this; }
	Matrix &operator=( Matrix &&m ) { for( int i = 0; i < 16; i++ ) { data[i] = m.data[i]; } return *this; }

	float &operator[]( const int index ) { return data[index]; }
	float operator[]( const int index ) const { return data[index]; }
	bool operator==( const Matrix &m ) { return matrix_equal( *this, m ); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool point_in_rect( const int px, const int py,
	                       const int x1, const int y1, const int x2, const int y2 )
{
	return ( px >= x1 && px <= x2 && py >= y1 && py <= y2 );
}

inline bool point_in_rect( const float px, const float py,
                           const float x1, const float y1, const float x2, const float y2 )
{
	return ( px >= x1 && px <= x2 && py >= y1 && py <= y2 );
}

inline bool point_in_rect( const double px, const double py,
                           const double x1, const double y1, const double x2, const double y2 )
{
	return ( px >= x1 && px <= x2 && py >= y1 && py <= y2 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////