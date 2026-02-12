#pragma once

#include <core/types.hpp>

#include <vendor/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MATH_PRE_EXPANDED_VECTOR_TYPES ( 1 )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PI ( 3.14159265358979 )
#define DEG2RAD ( PI / 180.0 )
#define RAD2DEG ( 180.0 / PI )

#define PI_F ( 3.14159265358979f )
#define DEG2RAD_F ( PI_F / 180.0f )
#define RAD2DEG_F ( 180.0f / PI_F )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ALIGN_16( value ) ( ( static_cast<int>( value ) >> 4 ) << 4 )
#define ALIGN_16_CEIL( value ) ( ( ( static_cast<int>( value ) >> 4 ) + 1 ) << 4 )
#define ALIGN_POWER( value, power ) ( ( static_cast<int>( value ) >> power ) << power )
#define ALIGN_POWER_CEIL( value, power ) ( ( ( static_cast<int>( value ) >> power ) + 1 ) << power )

extern u32 ceilpow2( u32 v );
extern u64 ceilpow2( u64 v );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if USE_CUSTOM_C_HEADERS
inline float fabsf( float value ) { return static_cast<float>( fabs( value ) ); }
#endif

extern double fast_mod( float x, float y );
extern float fast_modf( float x, float y );

extern double fast_floor( double x );
extern float fast_floorf( float x );

extern void fast_sin_cos( double radians, double &sin, double &cos );
extern void fast_sinf_cosf( float radians, float &sin, float &cos );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> inline T lengthdir_x( T dist, double degrees )
{
	return static_cast<T>( static_cast<double>( dist ) * cos( degrees * DEG2RAD ) );
}


template <typename T> inline T lengthdir_x( T dist, float degrees )
{
	return static_cast<T>( static_cast<float>( dist ) * cosf( degrees * DEG2RAD_F ) );
}


template <typename T> inline T lengthdir_y( T dist, double degrees )
{
	return static_cast<T>( static_cast<double>( dist ) * sin( degrees * DEG2RAD_F ) );
}


template <typename T> inline T lengthdir_y( T dist, float degrees )
{
	return static_cast<T>( static_cast<float>( dist ) * sinf( degrees * DEG2RAD_F ) );
}


template <typename T> inline T lengthdir_x_rad( T dist, double radians )
{
	return static_cast<T>( static_cast<double>( dist ) * cos( radians ) );
}


template <typename T> inline T lengthdir_x_rad( T dist, float radians )
{
	return static_cast<T>( static_cast<float>( dist ) * cosf( radians ) );
}


template <typename T> inline T lengthdir_y_rad( T dist, double radians )
{
	return static_cast<T>( static_cast<double>( dist ) * sin( radians ) );
}


template <typename T> inline T lengthdir_y_rad( T dist, float radians )
{
	return static_cast<T>( static_cast<float>( dist ) * sinf( radians ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> inline T min( T val1, T val2 )
{
	return ( val1 < val2 ) ? val1 : val2;
}


template <typename T> inline T max( T val1, T val2 )
{
	return ( val1 > val2 ) ? val1 : val2;
}


template <typename T> constexpr T clamp( T value, T min, T max )
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern double lerp_degrees( double value1, double value2, double amount );
extern float lerp_degrees( float value1, float value2, float amount );

extern double lerp_radians( double value1, double value2, double amount );
extern float lerp_radians( float value1, float value2, float amount );


template <typename T> inline T lerp( T value1, T value2, float amount )
{
	return static_cast<T>( value1 + ( value2 - value1 ) * amount );
}


template <typename T> inline void lerp( const T &value1, T value2, float amount, T &result )
{
	result = static_cast<T>( value1 + ( value2 - value1 ) * amount );
}


template <typename T> inline T lerp_delta( const T &value1, T value2, float amount, Delta delta )
{
	return lerp( value1, value2, 1.0f - expf( -amount * delta ) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> inline bool tween( T &variable, T target, T factor, Delta delta, double threshold = 0.25 )
{
	if( fabs( static_cast<double>( variable - target ) ) < static_cast<double>( threshold ) )
	{
		variable = target;
		return false;
	}
	else
	{
		variable += static_cast<T>( static_cast<double>( target - variable ) /
			max( static_cast<double>( factor / delta ), 1.0 ) );
		return true;
	}
}


template <typename T> inline bool tweenf( T &variable, T target, T factor, Delta delta, float threshold = 0.25f )
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool point_in_rect( int px, int py, int x1, int y1, int x2, int y2 )
{
	return ( px >= x1 && px <= x2 && py >= y1 && py <= y2 );
}

inline bool point_in_rect( float px, float py, float x1, float y1, float x2, float y2 )
{
	return ( px >= x1 && px <= x2 && py >= y1 && py <= y2 );
}

inline bool point_in_rect( double px, double py, double x1, double y1, double x2, double y2 )
{
	return ( px >= x1 && px <= x2 && py >= y1 && py <= y2 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern float_v2 cubic_bezier( float_v2 root0, float_v2 root1, float_v2 handle0, float_v2 handle1, float t );
extern double_v2 cubic_bezier( double_v2 root0, double_v2 root1, double_v2 handle0, double_v2 handle1, double t );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4x4 Matrix (Float)

extern bool float_m44_equal( const float_m44 &matrixA, const float_m44 &matrixB );

extern float_m44 float_m44_transpose( const float_m44 &matrix );

extern float_m44 float_m44_inverse( const float_m44 &matrix );

extern float_m44 float_m44_multiply( const float_m44 &matrixA, const float_m44 &matrixB );

extern float_m44 float_m44_multiply_scalar( const float_m44 &matrixA, float scalar );

extern float_m44 float_m44_add( const float_m44 &matrixA, const float_m44 &matrixB );

extern float_m44 float_m44_sub( const float_m44 &matrixA, const float_m44 &matrixB );

extern float_m44 float_m44_build_zeros();

extern float_m44 float_m44_build_identity();

extern float_m44 float_m44_build_scaling( float xscale, float yscale, float zscale );

extern float_m44 float_m44_build_translation( float xtrans, float ytrans, float ztrans );

extern float_m44 float_m44_build_rotation_x( float rad );

extern float_m44 float_m44_build_rotation_y( float rad );

extern float_m44 float_m44_build_rotation_z( float rad );

extern float_m44 float_m44_build_rotation_axis( float x, float y, float z, float rad );

extern float_m44 float_m44_build_basis( float fX, float fY, float fZ, float rX, float rY, float rZ,
	float uX, float uY, float uZ );

extern float_m44 float_m44_build_orthographic( float left, float right, float top, float bottom,
	float znear, float zfar );

extern float_m44 float_m44_build_perspective( float fov, float aspect, float znear, float zfar );

extern float_m44 float_m44_build_lookat( float x, float y, float z, float xto, float yto, float zto,
	float xup, float yup, float zup );

extern float_m44 float_m44_build_ndc( float width, float height );

extern float_m44 float_m44_from_double_m44( const double_m44 &matrix );

extern float_m44 float_m44_extract_rotation_forward_x( const float_m44 &matrix );

struct float_m44
{
	float data[16];

	float_m44();
	float_m44( const float_m44 &matrix );
	float_m44( float_m44 &&matrix );
	float_m44( const double_m44 &matrix );
	float_m44( double_m44 &&matrix );
	float_m44 &operator=( const float_m44 &matrix );
	float_m44 &operator=( float_m44 &&matrix );

	explicit operator double_m44() const;

	float &operator[]( int index ) { return data[index]; }
	float operator[]( int index ) const { return data[index]; }
	bool operator==( const float_m44 &matrix ) { return float_m44_equal( *this, matrix ); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4x4 Matrix (Double)

extern bool double_m44_equal( const double_m44 &matrixA, const double_m44 &matrixB );

extern double_m44 double_m44_transpose( const double_m44 &matrix );

extern double_m44 double_m44_inverse( const double_m44 &matrix );

extern double_m44 double_m44_multiply( const double_m44 &matrixA, const double_m44 &matrixB );

extern double_m44 double_m44_multiply_scalar( const double_m44 &matrixA, double scalar );

extern double_m44 double_m44_add( const double_m44 &matrixA, const double_m44 &matrixB );

extern double_m44 double_m44_sub( const double_m44 &matrixA, const double_m44 &matrixB );

extern double_m44 double_m44_build_zeros();

extern double_m44 double_m44_build_identity();

extern double_m44 double_m44_build_scaling( double xscale, double yscale, double zscale );

extern double_m44 double_m44_build_translation( double xtrans, double ytrans, double ztrans );

extern double_m44 double_m44_build_rotation_x( double rad );

extern double_m44 double_m44_build_rotation_y( double rad );

extern double_m44 double_m44_build_rotation_z( double rad );

extern double_m44 double_m44_build_rotation_axis( double x, double y, double z, double rad );

extern double_m44 double_m44_build_basis( double fX, double fY, double fZ, double rX, double rY, double rZ,
	double uX, double uY, double uZ );

extern double_m44 double_m44_build_orthographic( double left, double right, double top, double bottom,
	double znear, double zfar );

extern double_m44 double_m44_build_perspective( double fov, double aspect, double znear, double zfar );

extern double_m44 double_m44_build_lookat( double x, double y, double z, double xto, double yto, double zto,
	double xup, double yup, double zup );

extern double_m44 double_m44_build_ndc( double width, double height );

extern double_m44 double_m44_from_float_m44( const double_m44 &matrix );

extern double_m44 double_m44_extract_rotation_forward_x( const double_m44 &matrix );

struct double_m44
{
	double data[16];

	double_m44();
	double_m44( const float_m44 &matrix );
	double_m44( float_m44 &&matrix );
	double_m44( const double_m44 &matrix );
	double_m44( double_m44 &&matrix );
	double_m44 &operator=( const double_m44 &matrix );
	double_m44 &operator=( double_m44 &&matrix );

	explicit operator float_m44() const;

	double &operator[]( int index ) { return data[index]; }
	double operator[]( int index ) const { return data[index]; }
	bool operator==( const double_m44 &matrix ) { return double_m44_equal( *this, matrix ); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D Vector

template <typename T> struct Vector2D
{
	using VectorType = Vector2D<T>;
	T x, y;

	Vector2D( T x = 0, T y = 0 ) : x { x }, y { y } { }
	Vector2D( const VectorType &v ) : x { v.x }, y { v.y } { }
	VectorType &operator=( const VectorType &v ) { x = v.x; y = v.y; return *this; }

	/* Automatically cast vector types (if valid component cast exists) */
	template <typename S> Vector2D( Vector2D<S> &v ) :
		x { static_cast<T>( v.x ) },
		y { static_cast<T>( v.y ) } { }

	template <typename S> Vector2D( S x, S y ) :
		x { static_cast<T>( x ) },
		y { static_cast<T>( y ) } { }

	template <typename S> VectorType &operator=( const Vector2D<S> &v )
	{
		x = static_cast<T>( v.x );
		y = static_cast<T>( v.y );
		return *this;
	}

	T dot( const VectorType &v ) const
	{
		return x * v.x + y * v.y;
	}

	T length() const
	{
		return static_cast<T>( sqrt( static_cast<double>( x * x + y * y ) ) );
	}

	T length_sqr() const
	{
		return x * x + y * y;
	}

	VectorType &add( const VectorType &v )
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	VectorType &sub( const VectorType &v )
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	VectorType &multiply( const VectorType &v )
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	VectorType &multiply( const double s )
	{
		x = static_cast<T>( x * s );
		y = static_cast<T>( y * s );
		return *this;
	}

	VectorType &multiply( const float_m44 &matrix )
	{
		const float h[4] =
		{
			static_cast<float>( x ),
			static_cast<float>( y ),
			0.0f,
			1.0f
		};

		float r[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		for( int i = 0; i < 16; ++i ) { r[i % 4] += matrix[i] * h[i / 4]; }

		const float invR3 = r[3] == 0.0f ? 1.0f : 1.0f / r[3];
		x = static_cast<T>( r[0] * invR3 );
		y = static_cast<T>( r[1] * invR3 );
		return *this;
	}

	VectorType &multiply( const double_m44 &matrix )
	{
		const double h[4] =
		{
			static_cast<double>( x ),
			static_cast<double>( y ),
			0.0,
			1.0
		};

		double r[4] = { 0.0, 0.0, 0.0, 0.0 };

		for( int i = 0; i < 16; ++i ) { r[i % 4] += matrix[i] * h[i / 4]; }

		const float invR3 = r[3] == 0.0 ? 1.0 : 1.0 / r[3];
		x = static_cast<T>( r[0] * invR3 );
		y = static_cast<T>( r[1] * invR3 );
		return *this;
	}

	VectorType &divide( const VectorType &v )
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	VectorType &divide( double s )
	{
		x = static_cast<T>( x / s );
		y = static_cast<T>( y / s );
		return *this;
	}

	T cross( const VectorType &v ) const
	{
		return x * v.y - y * v.x;
	}

	VectorType &normalize()
	{
		const double l = static_cast<double>( this->length() );
		this->divide( l );
		return *this;
	}

	VectorType &project( const VectorType &onto )
	{
		const T d = this->dot( onto );
		const T l = onto.length_sqr();
		this->x = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.x );
		this->y = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.y );
		return *this;
	}

	VectorType &rotate( double angle, const VectorType &origin = { } )
	{
		const T tX = x - origin.x;
		const T tY = y - origin.y;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		x = static_cast<T>( tX * cT - tY * sT + origin.x );
		y = static_cast<T>( tX * sT + tY * cT + origin.y );
		return *this;
	}

	VectorType &reflect( const VectorType &normal )
	{
		VectorType n { normal };
		this->sub( n.multiply( this->dot( normal ) * 2.0 ) );
		return *this;
	}

	VectorType &lerp( const VectorType &v, const double amount )
	{
		x = static_cast<T>( x + ( v.x - x ) * amount );
		y = static_cast<T>( y + ( v.y - y ) * amount );
		return *this;
	}

	T angle_degrees() const
	{
		return static_cast<T>( atan2( static_cast<double>( y ), static_cast<double>( x ) ) * RAD2DEG );
	}

	T angle_radians() const
	{
		return static_cast<T>( atan2( static_cast<double>( y ), static_cast<double>( x ) ) );
	}

	VectorType operator+( const VectorType &v ) const { VectorType r { *this }; r.add( v );      return r; }
	VectorType operator-( const VectorType &v ) const { VectorType r { *this }; r.sub( v );      return r; }
	VectorType operator*( const VectorType &v ) const { VectorType r { *this }; r.multiply( v ); return r; }
	VectorType operator*( double s )            const { VectorType r { *this }; r.multiply( s ); return r; }
	VectorType operator/( const VectorType &v ) const { VectorType r { *this }; r.divide( v );   return r; }
	VectorType operator/( double s )            const { VectorType r { *this }; r.divide( s );   return r; }

	VectorType &operator+=( const VectorType &v ) { this->add( v );      return *this; }
	VectorType &operator-=( const VectorType &v ) { this->sub( v );      return *this; }
	VectorType &operator*=( const VectorType &v ) { this->multiply( v ); return *this; }
	VectorType &operator*=( double s )            { this->multiply( s ); return *this; }
	VectorType &operator/=( const VectorType &v ) { this->divide( v );   return *this; }
	VectorType &operator/=( double s )            { this->divide( s );   return *this; }

	bool operator==( const VectorType &v ) const { return x == v.x && y == v.y; }
};


#if !MATH_PRE_EXPANDED_VECTOR_TYPES

#define VECTOR_TYPE_2D_HEADER( Vec, T )                                                                  \
	extern template struct Vector2D<T>;                                                                  \
	extern bool Vec##_equal( const Vec &a, const Vec &b );                                               \
	extern    T Vec##_dot( const Vec &a, const Vec &b );                                                 \
	extern    T Vec##_length( const Vec &v );                                                            \
	extern    T Vec##_length_sqr( const Vec &v );                                                        \
	extern  Vec Vec##_add( const Vec &a, const Vec &b );                                                 \
	extern  Vec Vec##_sub( const Vec &a, const Vec &b );                                                 \
	extern  Vec Vec##_divide( const Vec &a, const Vec &b );                                              \
	extern  Vec Vec##_divide( const Vec &v, double s );                                                  \
	extern  Vec Vec##_multiply( const Vec &v, double s );                                                \
	extern  Vec Vec##_multiply( const Vec &a, const Vec &b );                                            \
	extern  Vec Vec##_multiply( const Vec &v, const double_m44 &m );                                     \
	extern  Vec Vec##_multiply( const Vec &v, const float_m44 &m );                                      \
	extern    T Vec##_cross( const Vec &a, const Vec &b );                                               \
	extern  Vec Vec##_normalize( const Vec &v );                                                         \
	extern  Vec Vec##_project( const Vec &v, const Vec &onto );                                          \
	extern  Vec Vec##_rotate( const Vec &v, float angle, const Vec &origin = { } );                      \
	extern  Vec Vec##_reflect( const Vec &v, const Vec &normal );                                        \
	extern Vec Vec##_lerp( const Vec &a, const Vec &b, float amount );                                   \
	extern   T Vec##_distance( const Vec &a, const Vec &b );                                             \
	extern   T Vec##_distance_sqr( const Vec &a, const Vec &b );                                         \

VECTOR_TYPE_2D_HEADER( u8_v2, u8 )
VECTOR_TYPE_2D_HEADER( i16_v2, i16 )
VECTOR_TYPE_2D_HEADER( u16_v2, u16 )
VECTOR_TYPE_2D_HEADER( int_v2, i32 )
VECTOR_TYPE_2D_HEADER( u32_v2, u32 )
VECTOR_TYPE_2D_HEADER( i64_v2, i64 )
VECTOR_TYPE_2D_HEADER( u64_v2, u64 )
VECTOR_TYPE_2D_HEADER( float_v2, float )
VECTOR_TYPE_2D_HEADER( double_v2, double )

#else

extern bool u8_v2_equal( const u8_v2 &a, const u8_v2 &b );
extern u8 u8_v2_dot( const u8_v2 &a, const u8_v2 &b );
extern u8 u8_v2_length( const u8_v2 &v );
extern u8 u8_v2_length_sqr( const u8_v2 &v );
extern u8_v2 u8_v2_add( const u8_v2 &a, const u8_v2 &b );
extern u8_v2 u8_v2_sub( const u8_v2 &a, const u8_v2 &b );
extern u8_v2 u8_v2_divide( const u8_v2 &a, const u8_v2 &b );
extern u8_v2 u8_v2_divide( const u8_v2 &v, double s );
extern u8_v2 u8_v2_multiply( const u8_v2 &v, double s );
extern u8_v2 u8_v2_multiply( const u8_v2 &a, const u8_v2 &b );
extern u8_v2 u8_v2_multiply( const u8_v2 &v, const double_m44 &m );
extern u8_v2 u8_v2_multiply( const u8_v2 &v, const float_m44 &m );
extern u8 u8_v2_cross( const u8_v2 &a, const u8_v2 &b );
extern u8_v2 u8_v2_normalize( const u8_v2 &v );
extern u8_v2 u8_v2_project( const u8_v2 &v, const u8_v2 &onto );
extern u8_v2 u8_v2_rotate( const u8_v2 &v, float angle, const u8_v2 &origin = { } );
extern u8_v2 u8_v2_reflect( const u8_v2 &v, const u8_v2 &normal );
extern u8_v2 u8_v2_lerp( const u8_v2 &a, const u8_v2 &b, float amount );
extern u8 u8_v2_distance( const u8_v2 &a, const u8_v2 &b );
extern u8 u8_v2_distance_sqr( const u8_v2 &a, const u8_v2 &b );

extern bool i16_v2_equal( const i16_v2 &a, const i16_v2 &b );
extern i16 i16_v2_dot( const i16_v2 &a, const i16_v2 &b );
extern i16 i16_v2_length( const i16_v2 &v );
extern i16 i16_v2_length_sqr( const i16_v2 &v );
extern i16_v2 i16_v2_add( const i16_v2 &a, const i16_v2 &b );
extern i16_v2 i16_v2_sub( const i16_v2 &a, const i16_v2 &b );
extern i16_v2 i16_v2_divide( const i16_v2 &a, const i16_v2 &b );
extern i16_v2 i16_v2_divide( const i16_v2 &v, double s );
extern i16_v2 i16_v2_multiply( const i16_v2 &v, double s );
extern i16_v2 i16_v2_multiply( const i16_v2 &a, const i16_v2 &b );
extern i16_v2 i16_v2_multiply( const i16_v2 &v, const double_m44 &m );
extern i16_v2 i16_v2_multiply( const i16_v2 &v, const float_m44 &m );
extern i16 i16_v2_cross( const i16_v2 &a, const i16_v2 &b );
extern i16_v2 i16_v2_normalize( const i16_v2 &v );
extern i16_v2 i16_v2_project( const i16_v2 &v, const i16_v2 &onto );
extern i16_v2 i16_v2_rotate( const i16_v2 &v, float angle, const i16_v2 &origin = { } );
extern i16_v2 i16_v2_reflect( const i16_v2 &v, const i16_v2 &normal );
extern i16_v2 i16_v2_lerp( const i16_v2 &a, const i16_v2 &b, float amount );
extern i16 i16_v2_distance( const i16_v2 &a, const i16_v2 &b );
extern i16 i16_v2_distance_sqr( const i16_v2 &a, const i16_v2 &b );

extern bool u16_v2_equal( const u16_v2 &a, const u16_v2 &b );
extern u16 u16_v2_dot( const u16_v2 &a, const u16_v2 &b );
extern u16 u16_v2_length( const u16_v2 &v );
extern u16 u16_v2_length_sqr( const u16_v2 &v );
extern u16_v2 u16_v2_add( const u16_v2 &a, const u16_v2 &b );
extern u16_v2 u16_v2_sub( const u16_v2 &a, const u16_v2 &b );
extern u16_v2 u16_v2_divide( const u16_v2 &a, const u16_v2 &b );
extern u16_v2 u16_v2_divide( const u16_v2 &v, double s );
extern u16_v2 u16_v2_multiply( const u16_v2 &v, double s );
extern u16_v2 u16_v2_multiply( const u16_v2 &a, const u16_v2 &b );
extern u16_v2 u16_v2_multiply( const u16_v2 &v, const double_m44 &m );
extern u16_v2 u16_v2_multiply( const u16_v2 &v, const float_m44 &m );
extern u16 u16_v2_cross( const u16_v2 &a, const u16_v2 &b );
extern u16_v2 u16_v2_normalize( const u16_v2 &v );
extern u16_v2 u16_v2_project( const u16_v2 &v, const u16_v2 &onto );
extern u16_v2 u16_v2_rotate( const u16_v2 &v, float angle, const u16_v2 &origin = { } );
extern u16_v2 u16_v2_reflect( const u16_v2 &v, const u16_v2 &normal );
extern u16_v2 u16_v2_lerp( const u16_v2 &a, const u16_v2 &b, float amount );
extern u16 u16_v2_distance( const u16_v2 &a, const u16_v2 &b );
extern u16 u16_v2_distance_sqr( const u16_v2 &a, const u16_v2 &b );

extern bool int_v2_equal( const int_v2 &a, const int_v2 &b );
extern i32 int_v2_dot( const int_v2 &a, const int_v2 &b );
extern i32 int_v2_length( const int_v2 &v );
extern i32 int_v2_length_sqr( const int_v2 &v );
extern int_v2 int_v2_add( const int_v2 &a, const int_v2 &b );
extern int_v2 int_v2_sub( const int_v2 &a, const int_v2 &b );
extern int_v2 int_v2_divide( const int_v2 &a, const int_v2 &b );
extern int_v2 int_v2_divide( const int_v2 &v, double s );
extern int_v2 int_v2_multiply( const int_v2 &v, double s );
extern int_v2 int_v2_multiply( const int_v2 &a, const int_v2 &b );
extern int_v2 int_v2_multiply( const int_v2 &v, const double_m44 &m );
extern int_v2 int_v2_multiply( const int_v2 &v, const float_m44 &m );
extern i32 int_v2_cross( const int_v2 &a, const int_v2 &b );
extern int_v2 int_v2_normalize( const int_v2 &v );
extern int_v2 int_v2_project( const int_v2 &v, const int_v2 &onto );
extern int_v2 int_v2_rotate( const int_v2 &v, float angle, const int_v2 &origin = { } );
extern int_v2 int_v2_reflect( const int_v2 &v, const int_v2 &normal );
extern int_v2 int_v2_lerp( const int_v2 &a, const int_v2 &b, float amount );
extern i32 int_v2_distance( const int_v2 &a, const int_v2 &b );
extern i32 int_v2_distance_sqr( const int_v2 &a, const int_v2 &b );

extern bool u32_v2_equal( const u32_v2 &a, const u32_v2 &b );
extern u32 u32_v2_dot( const u32_v2 &a, const u32_v2 &b );
extern u32 u32_v2_length( const u32_v2 &v );
extern u32 u32_v2_length_sqr( const u32_v2 &v );
extern u32_v2 u32_v2_add( const u32_v2 &a, const u32_v2 &b );
extern u32_v2 u32_v2_sub( const u32_v2 &a, const u32_v2 &b );
extern u32_v2 u32_v2_divide( const u32_v2 &a, const u32_v2 &b );
extern u32_v2 u32_v2_divide( const u32_v2 &v, double s );
extern u32_v2 u32_v2_multiply( const u32_v2 &v, double s );
extern u32_v2 u32_v2_multiply( const u32_v2 &a, const u32_v2 &b );
extern u32_v2 u32_v2_multiply( const u32_v2 &v, const double_m44 &m );
extern u32_v2 u32_v2_multiply( const u32_v2 &v, const float_m44 &m );
extern u32 u32_v2_cross( const u32_v2 &a, const u32_v2 &b );
extern u32_v2 u32_v2_normalize( const u32_v2 &v );
extern u32_v2 u32_v2_project( const u32_v2 &v, const u32_v2 &onto );
extern u32_v2 u32_v2_rotate( const u32_v2 &v, float angle, const u32_v2 &origin = { } );
extern u32_v2 u32_v2_reflect( const u32_v2 &v, const u32_v2 &normal );
extern u32_v2 u32_v2_lerp( const u32_v2 &a, const u32_v2 &b, float amount );
extern u32 u32_v2_distance( const u32_v2 &a, const u32_v2 &b );
extern u32 u32_v2_distance_sqr( const u32_v2 &a, const u32_v2 &b );

extern bool i64_v2_equal( const i64_v2 &a, const i64_v2 &b );
extern i64 i64_v2_dot( const i64_v2 &a, const i64_v2 &b );
extern i64 i64_v2_length( const i64_v2 &v );
extern i64 i64_v2_length_sqr( const i64_v2 &v );
extern i64_v2 i64_v2_add( const i64_v2 &a, const i64_v2 &b );
extern i64_v2 i64_v2_sub( const i64_v2 &a, const i64_v2 &b );
extern i64_v2 i64_v2_divide( const i64_v2 &a, const i64_v2 &b );
extern i64_v2 i64_v2_divide( const i64_v2 &v, double s );
extern i64_v2 i64_v2_multiply( const i64_v2 &v, double s );
extern i64_v2 i64_v2_multiply( const i64_v2 &a, const i64_v2 &b );
extern i64_v2 i64_v2_multiply( const i64_v2 &v, const double_m44 &m );
extern i64_v2 i64_v2_multiply( const i64_v2 &v, const float_m44 &m );
extern i64 i64_v2_cross( const i64_v2 &a, const i64_v2 &b );
extern i64_v2 i64_v2_normalize( const i64_v2 &v );
extern i64_v2 i64_v2_project( const i64_v2 &v, const i64_v2 &onto );
extern i64_v2 i64_v2_rotate( const i64_v2 &v, float angle, const i64_v2 &origin = { } );
extern i64_v2 i64_v2_reflect( const i64_v2 &v, const i64_v2 &normal );
extern i64_v2 i64_v2_lerp( const i64_v2 &a, const i64_v2 &b, float amount );
extern i64 i64_v2_distance( const i64_v2 &a, const i64_v2 &b );
extern i64 i64_v2_distance_sqr( const i64_v2 &a, const i64_v2 &b );

extern bool u64_v2_equal( const u64_v2 &a, const u64_v2 &b );
extern u64 u64_v2_dot( const u64_v2 &a, const u64_v2 &b );
extern u64 u64_v2_length( const u64_v2 &v );
extern u64 u64_v2_length_sqr( const u64_v2 &v );
extern u64_v2 u64_v2_add( const u64_v2 &a, const u64_v2 &b );
extern u64_v2 u64_v2_sub( const u64_v2 &a, const u64_v2 &b );
extern u64_v2 u64_v2_divide( const u64_v2 &a, const u64_v2 &b );
extern u64_v2 u64_v2_divide( const u64_v2 &v, double s );
extern u64_v2 u64_v2_multiply( const u64_v2 &v, double s );
extern u64_v2 u64_v2_multiply( const u64_v2 &a, const u64_v2 &b );
extern u64_v2 u64_v2_multiply( const u64_v2 &v, const double_m44 &m );
extern u64_v2 u64_v2_multiply( const u64_v2 &v, const float_m44 &m );
extern u64 u64_v2_cross( const u64_v2 &a, const u64_v2 &b );
extern u64_v2 u64_v2_normalize( const u64_v2 &v );
extern u64_v2 u64_v2_project( const u64_v2 &v, const u64_v2 &onto );
extern u64_v2 u64_v2_rotate( const u64_v2 &v, float angle, const u64_v2 &origin = { } );
extern u64_v2 u64_v2_reflect( const u64_v2 &v, const u64_v2 &normal );
extern u64_v2 u64_v2_lerp( const u64_v2 &a, const u64_v2 &b, float amount );
extern u64 u64_v2_distance( const u64_v2 &a, const u64_v2 &b );
extern u64 u64_v2_distance_sqr( const u64_v2 &a, const u64_v2 &b );

extern bool float_v2_equal( const float_v2 &a, const float_v2 &b );
extern float float_v2_dot( const float_v2 &a, const float_v2 &b );
extern float float_v2_length( const float_v2 &v );
extern float float_v2_length_sqr( const float_v2 &v );
extern float_v2 float_v2_add( const float_v2 &a, const float_v2 &b );
extern float_v2 float_v2_sub( const float_v2 &a, const float_v2 &b );
extern float_v2 float_v2_divide( const float_v2 &a, const float_v2 &b );
extern float_v2 float_v2_divide( const float_v2 &v, double s );
extern float_v2 float_v2_multiply( const float_v2 &v, double s );
extern float_v2 float_v2_multiply( const float_v2 &a, const float_v2 &b );
extern float_v2 float_v2_multiply( const float_v2 &v, const double_m44 &m );
extern float_v2 float_v2_multiply( const float_v2 &v, const float_m44 &m );
extern float float_v2_cross( const float_v2 &a, const float_v2 &b );
extern float_v2 float_v2_normalize( const float_v2 &v );
extern float_v2 float_v2_project( const float_v2 &v, const float_v2 &onto );
extern float_v2 float_v2_rotate( const float_v2 &v, float angle, const float_v2 &origin = { } );
extern float_v2 float_v2_reflect( const float_v2 &v, const float_v2 &normal );
extern float_v2 float_v2_lerp( const float_v2 &a, const float_v2 &b, float amount );
extern float float_v2_distance( const float_v2 &a, const float_v2 &b );
extern float float_v2_distance_sqr( const float_v2 &a, const float_v2 &b );

extern bool double_v2_equal( const double_v2 &a, const double_v2 &b );
extern double double_v2_dot( const double_v2 &a, const double_v2 &b );
extern double double_v2_length( const double_v2 &v );
extern double double_v2_length_sqr( const double_v2 &v );
extern double_v2 double_v2_add( const double_v2 &a, const double_v2 &b );
extern double_v2 double_v2_sub( const double_v2 &a, const double_v2 &b );
extern double_v2 double_v2_divide( const double_v2 &a, const double_v2 &b );
extern double_v2 double_v2_divide( const double_v2 &v, double s );
extern double_v2 double_v2_multiply( const double_v2 &v, double s );
extern double_v2 double_v2_multiply( const double_v2 &a, const double_v2 &b );
extern double_v2 double_v2_multiply( const double_v2 &v, const double_m44 &m );
extern double_v2 double_v2_multiply( const double_v2 &v, const float_m44 &m );
extern double double_v2_cross( const double_v2 &a, const double_v2 &b );
extern double_v2 double_v2_normalize( const double_v2 &v );
extern double_v2 double_v2_project( const double_v2 &v, const double_v2 &onto );
extern double_v2 double_v2_rotate( const double_v2 &v, float angle, const double_v2 &origin = { } );
extern double_v2 double_v2_reflect( const double_v2 &v, const double_v2 &normal );
extern double_v2 double_v2_lerp( const double_v2 &a, const double_v2 &b, float amount );
extern double double_v2_distance( const double_v2 &a, const double_v2 &b );
extern double double_v2_distance_sqr( const double_v2 &a, const double_v2 &b );

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3D Vector

template <typename T> struct Vector3D
{
	using VectorType = Vector3D<T>;
	T x, y, z;

	Vector3D( T x = 0, T y = 0, T z = 0 ) : x { x }, y { y }, z { z } { }
	Vector3D( const VectorType &v ) : x { v.x }, y { v.y }, z { v.z } { }
	VectorType &operator=( const VectorType &v ) { x = v.x; y = v.y; z = v.z; return *this; }

	/* Automatically cast vector types (if valid component cast exists) */
	template <typename S> Vector3D( Vector3D<S> &v ) :
		x { static_cast<T>( v.x ) },
		y { static_cast<T>( v.y ) },
		z { static_cast<T>( v.z ) } { }

	template <typename S> Vector3D( S x, S y, S z ) :
		x { static_cast<T>( x ) },
		y { static_cast<T>( y ) },
		z { static_cast<T>( z ) } { }

	template <typename S> VectorType &operator=( const Vector3D<S> &v )
	{
		x = static_cast<T>( v.x );
		y = static_cast<T>( v.y );
		z = static_cast<T>( v.z );
		return *this;
	}

	Vector2D<T> xy() const { return Vector2D { x, y }; }
	Vector2D<T> yz() const { return Vector2D { y, z }; }

	T dot( const VectorType &v ) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	T length() const
	{
		return static_cast<T>( sqrt( static_cast<double>( x * x + y * y + z * z ) ) );
	}

	T length_sqr() const
	{
		return x * x + y * y + z * z;
	}

	VectorType &add( const VectorType &v )
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	VectorType &sub( const VectorType &v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	VectorType &multiply( const VectorType &v )
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	VectorType &multiply( double s )
	{
		x = static_cast<T>( x * s );
		y = static_cast<T>( y * s );
		z = static_cast<T>( z * s );
		return *this;
	}

	VectorType &multiply( const float_m44 &matrix )
	{
		const float h[4] =
		{
			static_cast<float>( x ),
			static_cast<float>( y ),
			static_cast<float>( z ),
			1.0f
		};

		float r[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		for( int i = 0; i < 16; ++i ) { r[i % 4] += matrix[i] * h[i / 4]; }

		const float invR3 = r[3] == 0.0f ? 1.0f : 1.0f / r[3];
		x = static_cast<T>( r[0] * invR3 );
		y = static_cast<T>( r[1] * invR3 );
		z = static_cast<T>( r[2] * invR3 );
		return *this;
	}

	VectorType &multiply( const double_m44 &matrix )
	{
		const double h[4] =
		{
			static_cast<double>( x ),
			static_cast<double>( y ),
			static_cast<double>( z ),
			1.0
		};

		double r[4] = { 0.0, 0.0, 0.0, 0.0 };

		for( int i = 0; i < 16; ++i ) { r[i % 4] += matrix[i] * h[i / 4]; }

		const float invR3 = r[3] == 0.0 ? 1.0 : 1.0 / r[3];
		x = static_cast<T>( r[0] * invR3 );
		y = static_cast<T>( r[1] * invR3 );
		z = static_cast<T>( r[2] * invR3 );
		return *this;
	}

	VectorType &divide( const VectorType &v )
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	VectorType &divide( double s )
	{
		x = static_cast<T>( x / s );
		y = static_cast<T>( y / s );
		z = static_cast<T>( z / s );
		return *this;
	}

	VectorType &cross( const VectorType &v )
	{
		VectorType r;
		r.x = y * v.z - z * v.y;
		r.y = z * v.x - x * v.z;
		r.z = x * v.y - y * v.x;
		*this = r;
		return *this;
	}

	VectorType &normalize()
	{
		const double l = static_cast<double>( this->length() );
		this->divide( l );
		return *this;
	}

	VectorType &project( const VectorType &onto )
	{
		const T d = this->dot( onto );
		const T l = onto.length_sqr();
		this->x = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.x );
		this->y = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.y );
		this->z = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.z );
		return *this;
	}

	VectorType &rotate_x( double angle, const VectorType &origin = {} )
	{
		const double tY = static_cast<double>( y ) - origin.y;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		y = static_cast<T>( tY * cT - tZ * sT + origin.y );
		z = static_cast<T>( tY * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_y( double angle, const VectorType &origin = {} )
	{
		const double tX = static_cast<double>( x )- origin.x;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		x = static_cast<T>( tX * cT + tZ * sT + origin.x );
		z = static_cast<T>( -tX * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_z( double angle, const VectorType &origin = {} )
	{
		const double tX = static_cast<double>( x ) - origin.x;
		const double tY = static_cast<double>( y ) - origin.y;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		x = static_cast<T>( tX * cT - tY * sT + origin.x );
		y = static_cast<T>( tX * sT + tY * cT + origin.y );
		return *this;
	}

	VectorType &reflect( const VectorType &normal )
	{
		VectorType n { normal };
		this->sub( n.multiply( this->dot( normal ) * 2.0 ) );
		return *this;
	}

	VectorType &lerp( const VectorType &v, double amount )
	{
		x = static_cast<T>( x + ( v.x - x ) * amount );
		y = static_cast<T>( y + ( v.y - y ) * amount );
		z = static_cast<T>( z + ( v.z - z ) * amount );
		return *this;
	}

	VectorType operator+( const VectorType &v ) const { VectorType r { *this }; r.add( v );      return r; }
	VectorType operator-( const VectorType &v ) const { VectorType r { *this }; r.sub( v );      return r; }
	VectorType operator*( const VectorType &v ) const { VectorType r { *this }; r.multiply( v ); return r; }
	VectorType operator*( double s )            const { VectorType r { *this }; r.multiply( s ); return r; }
	VectorType operator/( const VectorType &v ) const { VectorType r { *this }; r.divide( v );   return r; }
	VectorType operator/( double s )            const { VectorType r { *this }; r.divide( s );   return r; }

	VectorType &operator+=( const VectorType &v ) { this->add( v );      return *this; }
	VectorType &operator-=( const VectorType &v ) { this->sub( v );      return *this; }
	VectorType &operator*=( const VectorType &v ) { this->multiply( v ); return *this; }
	VectorType &operator*=( double s )            { this->multiply( s ); return *this; }
	VectorType &operator/=( const VectorType &v ) { this->divide( v );   return *this; }
	VectorType &operator/=( double s )            { this->divide( s );   return *this; }

	bool operator==( const VectorType &v ) const { return x == v.x && y == v.y && z == v.z; }
};


#if !MATH_PRE_EXPANDED_VECTOR_TYPES

#define VECTOR_TYPE_3D_HEADER( Vec, T )                                               \
	extern template struct Vector3D<T>;                                               \
	extern bool Vec##_equal( const Vec &a, const Vec &b );                            \
	extern    T Vec##_dot( const Vec &a, const Vec &b );                              \
	extern    T Vec##_length( const Vec &v );                                         \
	extern    T Vec##_length_sqr( const Vec &v );                                     \
	extern  Vec Vec##_add( const Vec &a, const Vec &b );                              \
	extern  Vec Vec##_sub( const Vec &a, const Vec &b );                              \
	extern  Vec Vec##_divide( const Vec &a, const Vec &b );                           \
	extern  Vec Vec##_divide( const Vec &v, double s );                               \
	extern  Vec Vec##_multiply( const Vec &v, double s );                             \
	extern  Vec Vec##_multiply( const Vec &a, const Vec &b );                         \
	extern  Vec Vec##_multiply( const Vec &v, const double_m44 &m );                  \
	extern  Vec Vec##_multiply( const Vec &v, const float_m44 &m );                   \
	extern  Vec Vec##_cross( const Vec &a, const Vec &b );                            \
	extern  Vec Vec##_normalize( const Vec &v );                                      \
	extern  Vec Vec##_project( const Vec &v, const Vec &onto );                       \
	extern  Vec Vec##_rotate_x( const Vec &v, float angle, const Vec &origin = { } ); \
	extern  Vec Vec##_rotate_y( const Vec &v, float angle, const Vec &origin = { } ); \
	extern  Vec Vec##_rotate_z( const Vec &v, float angle, const Vec &origin = { } ); \
	extern  Vec Vec##_reflect( const Vec &v, const Vec &normal );                     \
	extern  Vec Vec##_lerp( const Vec &a, const Vec &b, float amount );               \
	extern    T Vec##_distance( const Vec &a, const Vec &b );                         \
	extern    T Vec##_distance_sqr( const Vec &a, const Vec &b );

VECTOR_TYPE_3D_HEADER( u8_v3, u8 )
VECTOR_TYPE_3D_HEADER( i16_v3, i16 )
VECTOR_TYPE_3D_HEADER( u16_v3, u16 )
VECTOR_TYPE_3D_HEADER( int_v3, i32 )
VECTOR_TYPE_3D_HEADER( u32_v3, u32 )
VECTOR_TYPE_3D_HEADER( i64_v3, i64 )
VECTOR_TYPE_3D_HEADER( u64_v3, u64 )
VECTOR_TYPE_3D_HEADER( float_v3, float )
VECTOR_TYPE_3D_HEADER( double_v3, double )

#else

extern bool u8_v3_equal( const u8_v3 &a, const u8_v3 &b );
extern u8 u8_v3_dot( const u8_v3 &a, const u8_v3 &b );
extern u8 u8_v3_length( const u8_v3 &v );
extern u8 u8_v3_length_sqr( const u8_v3 &v );
extern u8_v3 u8_v3_add( const u8_v3 &a, const u8_v3 &b );
extern u8_v3 u8_v3_sub( const u8_v3 &a, const u8_v3 &b );
extern u8_v3 u8_v3_divide( const u8_v3 &a, const u8_v3 &b );
extern u8_v3 u8_v3_divide( const u8_v3 &v, double s );
extern u8_v3 u8_v3_multiply( const u8_v3 &v, double s );
extern u8_v3 u8_v3_multiply( const u8_v3 &a, const u8_v3 &b );
extern u8_v3 u8_v3_multiply( const u8_v3 &v, const double_m44 &m );
extern u8_v3 u8_v3_multiply( const u8_v3 &v, const float_m44 &m );
extern u8_v3 u8_v3_cross( const u8_v3 &a, const u8_v3 &b );
extern u8_v3 u8_v3_normalize( const u8_v3 &v );
extern u8_v3 u8_v3_project( const u8_v3 &v, const u8_v3 &onto );
extern u8_v3 u8_v3_rotate_x( const u8_v3 &v, float angle, const u8_v3 &origin = { } );
extern u8_v3 u8_v3_rotate_y( const u8_v3 &v, float angle, const u8_v3 &origin = { } );
extern u8_v3 u8_v3_rotate_z( const u8_v3 &v, float angle, const u8_v3 &origin = { } );
extern u8_v3 u8_v3_reflect( const u8_v3 &v, const u8_v3 &normal );
extern u8_v3 u8_v3_lerp( const u8_v3 &a, const u8_v3 &b, float amount );
extern u8 u8_v3_distance( const u8_v3 &a, const u8_v3 &b );
extern u8 u8_v3_distance_sqr( const u8_v3 &a, const u8_v3 &b );

extern bool i16_v3_equal( const i16_v3 &a, const i16_v3 &b );
extern i16 i16_v3_dot( const i16_v3 &a, const i16_v3 &b );
extern i16 i16_v3_length( const i16_v3 &v );
extern i16 i16_v3_length_sqr( const i16_v3 &v );
extern i16_v3 i16_v3_add( const i16_v3 &a, const i16_v3 &b );
extern i16_v3 i16_v3_sub( const i16_v3 &a, const i16_v3 &b );
extern i16_v3 i16_v3_divide( const i16_v3 &a, const i16_v3 &b );
extern i16_v3 i16_v3_divide( const i16_v3 &v, double s );
extern i16_v3 i16_v3_multiply( const i16_v3 &v, double s );
extern i16_v3 i16_v3_multiply( const i16_v3 &a, const i16_v3 &b );
extern i16_v3 i16_v3_multiply( const i16_v3 &v, const double_m44 &m );
extern i16_v3 i16_v3_multiply( const i16_v3 &v, const float_m44 &m );
extern i16_v3 i16_v3_cross( const i16_v3 &a, const i16_v3 &b );
extern i16_v3 i16_v3_normalize( const i16_v3 &v );
extern i16_v3 i16_v3_project( const i16_v3 &v, const i16_v3 &onto );
extern i16_v3 i16_v3_rotate_x( const i16_v3 &v, float angle, const i16_v3 &origin = { } );
extern i16_v3 i16_v3_rotate_y( const i16_v3 &v, float angle, const i16_v3 &origin = { } );
extern i16_v3 i16_v3_rotate_z( const i16_v3 &v, float angle, const i16_v3 &origin = { } );
extern i16_v3 i16_v3_reflect( const i16_v3 &v, const i16_v3 &normal );
extern i16_v3 i16_v3_lerp( const i16_v3 &a, const i16_v3 &b, float amount );
extern i16 i16_v3_distance( const i16_v3 &a, const i16_v3 &b );
extern i16 i16_v3_distance_sqr( const i16_v3 &a, const i16_v3 &b );

extern bool u16_v3_equal( const u16_v3 &a, const u16_v3 &b );
extern u16 u16_v3_dot( const u16_v3 &a, const u16_v3 &b );
extern u16 u16_v3_length( const u16_v3 &v );
extern u16 u16_v3_length_sqr( const u16_v3 &v );
extern u16_v3 u16_v3_add( const u16_v3 &a, const u16_v3 &b );
extern u16_v3 u16_v3_sub( const u16_v3 &a, const u16_v3 &b );
extern u16_v3 u16_v3_divide( const u16_v3 &a, const u16_v3 &b );
extern u16_v3 u16_v3_divide( const u16_v3 &v, double s );
extern u16_v3 u16_v3_multiply( const u16_v3 &v, double s );
extern u16_v3 u16_v3_multiply( const u16_v3 &a, const u16_v3 &b );
extern u16_v3 u16_v3_multiply( const u16_v3 &v, const double_m44 &m );
extern u16_v3 u16_v3_multiply( const u16_v3 &v, const float_m44 &m );
extern u16_v3 u16_v3_cross( const u16_v3 &a, const u16_v3 &b );
extern u16_v3 u16_v3_normalize( const u16_v3 &v );
extern u16_v3 u16_v3_project( const u16_v3 &v, const u16_v3 &onto );
extern u16_v3 u16_v3_rotate_x( const u16_v3 &v, float angle, const u16_v3 &origin = { } );
extern u16_v3 u16_v3_rotate_y( const u16_v3 &v, float angle, const u16_v3 &origin = { } );
extern u16_v3 u16_v3_rotate_z( const u16_v3 &v, float angle, const u16_v3 &origin = { } );
extern u16_v3 u16_v3_reflect( const u16_v3 &v, const u16_v3 &normal );
extern u16_v3 u16_v3_lerp( const u16_v3 &a, const u16_v3 &b, float amount );
extern u16 u16_v3_distance( const u16_v3 &a, const u16_v3 &b );
extern u16 u16_v3_distance_sqr( const u16_v3 &a, const u16_v3 &b );

extern bool int_v3_equal( const int_v3 &a, const int_v3 &b );
extern i32 int_v3_dot( const int_v3 &a, const int_v3 &b );
extern i32 int_v3_length( const int_v3 &v );
extern i32 int_v3_length_sqr( const int_v3 &v );
extern int_v3 int_v3_add( const int_v3 &a, const int_v3 &b );
extern int_v3 int_v3_sub( const int_v3 &a, const int_v3 &b );
extern int_v3 int_v3_divide( const int_v3 &a, const int_v3 &b );
extern int_v3 int_v3_divide( const int_v3 &v, double s );
extern int_v3 int_v3_multiply( const int_v3 &v, double s );
extern int_v3 int_v3_multiply( const int_v3 &a, const int_v3 &b );
extern int_v3 int_v3_multiply( const int_v3 &v, const double_m44 &m );
extern int_v3 int_v3_multiply( const int_v3 &v, const float_m44 &m );
extern int_v3 int_v3_cross( const int_v3 &a, const int_v3 &b );
extern int_v3 int_v3_normalize( const int_v3 &v );
extern int_v3 int_v3_project( const int_v3 &v, const int_v3 &onto );
extern int_v3 int_v3_rotate_x( const int_v3 &v, float angle, const int_v3 &origin = { } );
extern int_v3 int_v3_rotate_y( const int_v3 &v, float angle, const int_v3 &origin = { } );
extern int_v3 int_v3_rotate_z( const int_v3 &v, float angle, const int_v3 &origin = { } );
extern int_v3 int_v3_reflect( const int_v3 &v, const int_v3 &normal );
extern int_v3 int_v3_lerp( const int_v3 &a, const int_v3 &b, float amount );
extern i32 int_v3_distance( const int_v3 &a, const int_v3 &b );
extern i32 int_v3_distance_sqr( const int_v3 &a, const int_v3 &b );

extern bool u32_v3_equal( const u32_v3 &a, const u32_v3 &b );
extern u32 u32_v3_dot( const u32_v3 &a, const u32_v3 &b );
extern u32 u32_v3_length( const u32_v3 &v );
extern u32 u32_v3_length_sqr( const u32_v3 &v );
extern u32_v3 u32_v3_add( const u32_v3 &a, const u32_v3 &b );
extern u32_v3 u32_v3_sub( const u32_v3 &a, const u32_v3 &b );
extern u32_v3 u32_v3_divide( const u32_v3 &a, const u32_v3 &b );
extern u32_v3 u32_v3_divide( const u32_v3 &v, double s );
extern u32_v3 u32_v3_multiply( const u32_v3 &v, double s );
extern u32_v3 u32_v3_multiply( const u32_v3 &a, const u32_v3 &b );
extern u32_v3 u32_v3_multiply( const u32_v3 &v, const double_m44 &m );
extern u32_v3 u32_v3_multiply( const u32_v3 &v, const float_m44 &m );
extern u32_v3 u32_v3_cross( const u32_v3 &a, const u32_v3 &b );
extern u32_v3 u32_v3_normalize( const u32_v3 &v );
extern u32_v3 u32_v3_project( const u32_v3 &v, const u32_v3 &onto );
extern u32_v3 u32_v3_rotate_x( const u32_v3 &v, float angle, const u32_v3 &origin = { } );
extern u32_v3 u32_v3_rotate_y( const u32_v3 &v, float angle, const u32_v3 &origin = { } );
extern u32_v3 u32_v3_rotate_z( const u32_v3 &v, float angle, const u32_v3 &origin = { } );
extern u32_v3 u32_v3_reflect( const u32_v3 &v, const u32_v3 &normal );
extern u32_v3 u32_v3_lerp( const u32_v3 &a, const u32_v3 &b, float amount );
extern u32 u32_v3_distance( const u32_v3 &a, const u32_v3 &b );
extern u32 u32_v3_distance_sqr( const u32_v3 &a, const u32_v3 &b );

extern bool i64_v3_equal( const i64_v3 &a, const i64_v3 &b );
extern i64 i64_v3_dot( const i64_v3 &a, const i64_v3 &b );
extern i64 i64_v3_length( const i64_v3 &v );
extern i64 i64_v3_length_sqr( const i64_v3 &v );
extern i64_v3 i64_v3_add( const i64_v3 &a, const i64_v3 &b );
extern i64_v3 i64_v3_sub( const i64_v3 &a, const i64_v3 &b );
extern i64_v3 i64_v3_divide( const i64_v3 &a, const i64_v3 &b );
extern i64_v3 i64_v3_divide( const i64_v3 &v, double s );
extern i64_v3 i64_v3_multiply( const i64_v3 &v, double s );
extern i64_v3 i64_v3_multiply( const i64_v3 &a, const i64_v3 &b );
extern i64_v3 i64_v3_multiply( const i64_v3 &v, const double_m44 &m );
extern i64_v3 i64_v3_multiply( const i64_v3 &v, const float_m44 &m );
extern i64_v3 i64_v3_cross( const i64_v3 &a, const i64_v3 &b );
extern i64_v3 i64_v3_normalize( const i64_v3 &v );
extern i64_v3 i64_v3_project( const i64_v3 &v, const i64_v3 &onto );
extern i64_v3 i64_v3_rotate_x( const i64_v3 &v, float angle, const i64_v3 &origin = { } );
extern i64_v3 i64_v3_rotate_y( const i64_v3 &v, float angle, const i64_v3 &origin = { } );
extern i64_v3 i64_v3_rotate_z( const i64_v3 &v, float angle, const i64_v3 &origin = { } );
extern i64_v3 i64_v3_reflect( const i64_v3 &v, const i64_v3 &normal );
extern i64_v3 i64_v3_lerp( const i64_v3 &a, const i64_v3 &b, float amount );
extern i64 i64_v3_distance( const i64_v3 &a, const i64_v3 &b );
extern i64 i64_v3_distance_sqr( const i64_v3 &a, const i64_v3 &b );

extern bool u64_v3_equal( const u64_v3 &a, const u64_v3 &b );
extern u64 u64_v3_dot( const u64_v3 &a, const u64_v3 &b );
extern u64 u64_v3_length( const u64_v3 &v );
extern u64 u64_v3_length_sqr( const u64_v3 &v );
extern u64_v3 u64_v3_add( const u64_v3 &a, const u64_v3 &b );
extern u64_v3 u64_v3_sub( const u64_v3 &a, const u64_v3 &b );
extern u64_v3 u64_v3_divide( const u64_v3 &a, const u64_v3 &b );
extern u64_v3 u64_v3_divide( const u64_v3 &v, double s );
extern u64_v3 u64_v3_multiply( const u64_v3 &v, double s );
extern u64_v3 u64_v3_multiply( const u64_v3 &a, const u64_v3 &b );
extern u64_v3 u64_v3_multiply( const u64_v3 &v, const double_m44 &m );
extern u64_v3 u64_v3_multiply( const u64_v3 &v, const float_m44 &m );
extern u64_v3 u64_v3_cross( const u64_v3 &a, const u64_v3 &b );
extern u64_v3 u64_v3_normalize( const u64_v3 &v );
extern u64_v3 u64_v3_project( const u64_v3 &v, const u64_v3 &onto );
extern u64_v3 u64_v3_rotate_x( const u64_v3 &v, float angle, const u64_v3 &origin = { } );
extern u64_v3 u64_v3_rotate_y( const u64_v3 &v, float angle, const u64_v3 &origin = { } );
extern u64_v3 u64_v3_rotate_z( const u64_v3 &v, float angle, const u64_v3 &origin = { } );
extern u64_v3 u64_v3_reflect( const u64_v3 &v, const u64_v3 &normal );
extern u64_v3 u64_v3_lerp( const u64_v3 &a, const u64_v3 &b, float amount );
extern u64 u64_v3_distance( const u64_v3 &a, const u64_v3 &b );
extern u64 u64_v3_distance_sqr( const u64_v3 &a, const u64_v3 &b );

extern bool float_v3_equal( const float_v3 &a, const float_v3 &b );
extern float float_v3_dot( const float_v3 &a, const float_v3 &b );
extern float float_v3_length( const float_v3 &v );
extern float float_v3_length_sqr( const float_v3 &v );
extern float_v3 float_v3_add( const float_v3 &a, const float_v3 &b );
extern float_v3 float_v3_sub( const float_v3 &a, const float_v3 &b );
extern float_v3 float_v3_divide( const float_v3 &a, const float_v3 &b );
extern float_v3 float_v3_divide( const float_v3 &v, double s );
extern float_v3 float_v3_multiply( const float_v3 &v, double s );
extern float_v3 float_v3_multiply( const float_v3 &a, const float_v3 &b );
extern float_v3 float_v3_multiply( const float_v3 &v, const double_m44 &m );
extern float_v3 float_v3_multiply( const float_v3 &v, const float_m44 &m );
extern float_v3 float_v3_cross( const float_v3 &a, const float_v3 &b );
extern float_v3 float_v3_normalize( const float_v3 &v );
extern float_v3 float_v3_project( const float_v3 &v, const float_v3 &onto );
extern float_v3 float_v3_rotate_x( const float_v3 &v, float angle, const float_v3 &origin = { } );
extern float_v3 float_v3_rotate_y( const float_v3 &v, float angle, const float_v3 &origin = { } );
extern float_v3 float_v3_rotate_z( const float_v3 &v, float angle, const float_v3 &origin = { } );
extern float_v3 float_v3_reflect( const float_v3 &v, const float_v3 &normal );
extern float_v3 float_v3_lerp( const float_v3 &a, const float_v3 &b, float amount );
extern float float_v3_distance( const float_v3 &a, const float_v3 &b );
extern float float_v3_distance_sqr( const float_v3 &a, const float_v3 &b );

extern bool double_v3_equal( const double_v3 &a, const double_v3 &b );
extern double double_v3_dot( const double_v3 &a, const double_v3 &b );
extern double double_v3_length( const double_v3 &v );
extern double double_v3_length_sqr( const double_v3 &v );
extern double_v3 double_v3_add( const double_v3 &a, const double_v3 &b );
extern double_v3 double_v3_sub( const double_v3 &a, const double_v3 &b );
extern double_v3 double_v3_divide( const double_v3 &a, const double_v3 &b );
extern double_v3 double_v3_divide( const double_v3 &v, double s );
extern double_v3 double_v3_multiply( const double_v3 &v, double s );
extern double_v3 double_v3_multiply( const double_v3 &a, const double_v3 &b );
extern double_v3 double_v3_multiply( const double_v3 &v, const double_m44 &m );
extern double_v3 double_v3_multiply( const double_v3 &v, const float_m44 &m );
extern double_v3 double_v3_cross( const double_v3 &a, const double_v3 &b );
extern double_v3 double_v3_normalize( const double_v3 &v );
extern double_v3 double_v3_project( const double_v3 &v, const double_v3 &onto );
extern double_v3 double_v3_rotate_x( const double_v3 &v, float angle, const double_v3 &origin = { } );
extern double_v3 double_v3_rotate_y( const double_v3 &v, float angle, const double_v3 &origin = { } );
extern double_v3 double_v3_rotate_z( const double_v3 &v, float angle, const double_v3 &origin = { } );
extern double_v3 double_v3_reflect( const double_v3 &v, const double_v3 &normal );
extern double_v3 double_v3_lerp( const double_v3 &a, const double_v3 &b, float amount );
extern double double_v3_distance( const double_v3 &a, const double_v3 &b );
extern double double_v3_distance_sqr( const double_v3 &a, const double_v3 &b );

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4D Vector

template <typename T> struct Vector4D
{
	using VectorType = Vector4D<T>;
	T x, y, z, w;

	Vector4D( T x = 0, T y = 0, T z = 0, T w = 0 ) : x { x }, y { y }, z { z }, w { w } { }
	Vector4D( const VectorType &v ) : x { v.x }, y { v.y }, z { v.z }, w { v.w } { }
	VectorType &operator=( const VectorType &v ) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

	/* Automatically cast vector types (if valid component cast exists) */
	template <typename S> Vector4D( Vector4D<S> &v ) :
		x { static_cast<T>( v.x ) },
		y { static_cast<T>( v.y ) },
		z { static_cast<T>( v.z ) },
		w { static_cast<T>( v.w ) } { }

	template <typename S> Vector4D( S x, S y, S z, S w ) :
		x { static_cast<T>( x ) },
		y { static_cast<T>( y ) },
		z { static_cast<T>( z ) },
		w { static_cast<T>( w ) } { }

	template <typename S> VectorType &operator=( const Vector4D<S> &v )
	{
		x = static_cast<T>( v.x );
		y = static_cast<T>( v.y );
		z = static_cast<T>( v.z );
		w = static_cast<T>( v.w );
		return *this;
	}

	Vector2D<T> xy()  const { return Vector2D<T>{ x, y }; }
	Vector2D<T> yz()  const { return Vector2D<T>{ y, z }; }
	Vector2D<T> zw()  const { return Vector2D<T>{ z, w }; }
	Vector3D<T> xyz() const { return Vector3D<T>{ x, y, z }; }
	Vector3D<T> yzw() const { return Vector3D<T>{ y, z, w }; }

	T dot( const VectorType &v ) const
	{
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}

	T length() const
	{
		return static_cast<T>( sqrt( static_cast<double>( x * x + y * y + z * z + w * w ) ) );
	}

	T length_sqr() const
	{
		return x * x + y * y + z * z + w * w;
	}

	VectorType &add( const VectorType &v )
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	VectorType &sub( const VectorType &v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	VectorType &multiply( const VectorType &v )
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	VectorType &multiply( double s )
	{
		x = static_cast<T>( x * s );
		y = static_cast<T>( y * s );
		z = static_cast<T>( z * s );
		w = static_cast<T>( w * s );
		return *this;
	}

	VectorType &multiply( const float_m44 &matrix )
	{
		const float h[4] =
		{
			static_cast<float>( x ),
			static_cast<float>( y ),
			static_cast<float>( z ),
			static_cast<float>( w ),
		};

		float r[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		for( int i = 0; i < 16; ++i ) { r[i % 4] += matrix[i] * h[i / 4]; }

		const float invR3 = r[3] == 0.0f ? 1.0f : 1.0f / r[3];
		x = static_cast<T>( r[0] * invR3 );
		y = static_cast<T>( r[1] * invR3 );
		z = static_cast<T>( r[2] * invR3 );
		w = static_cast<T>( r[3] );
		return *this;
	}

	VectorType &multiply( const double_m44 &matrix )
	{
		const double h[4] =
		{
			static_cast<double>( x ),
			static_cast<double>( y ),
			static_cast<double>( z ),
			static_cast<double>( w ),
		};

		double r[4] = { 0.0, 0.0, 0.0, 0.0 };

		for( int i = 0; i < 16; ++i ) { r[i % 4] += matrix[i] * h[i / 4]; }

		const float invR3 = r[3] == 0.0 ? 1.0 : 1.0 / r[3];
		x = static_cast<T>( r[0] * invR3 );
		y = static_cast<T>( r[1] * invR3 );
		z = static_cast<T>( r[2] * invR3 );
		w = static_cast<T>( r[3] );
		return *this;
	}

	VectorType &divide( const VectorType &v )
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	VectorType &divide( double s )
	{
		x = static_cast<T>( x / s );
		y = static_cast<T>( y / s );
		z = static_cast<T>( z / s );
		w = static_cast<T>( w / s );
		return *this;
	}

	VectorType &cross( const VectorType &v )
	{
		VectorType r;
		r.x = y * v.z - z * v.y;
		r.y = z * v.x - x * v.z;
		r.z = x * v.y - y * v.x;
		r.w = 0; // TODO: 4D cross returns 3D vector
		*this = r;
		return *this;
	}

	VectorType &normalize()
	{
		const double l = static_cast<double>( this->length() );
		this->divide( l );
		return *this;
	}

	VectorType &project( const VectorType &onto )
	{
		const T d = this->dot( onto );
		const T l = onto.length_sqr();
		this->x = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.x );
		this->y = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.y );
		this->z = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.z );
		this->w = static_cast<T>( ( d / static_cast<double>( l ) ) * onto.w );
		return *this;
	}

	VectorType &rotate_x( double angle, const VectorType &origin = {} )
	{
		const double tY = static_cast<double>( y ) - origin.y;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		y = static_cast<T>( tY * cT - tZ * sT + origin.y );
		z = static_cast<T>( tY * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_y( double angle, const VectorType &origin = {} )
	{
		const double tX = static_cast<double>( x )- origin.x;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		x = static_cast<T>( tX * cT + tZ * sT + origin.x );
		z = static_cast<T>( -tX * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_z( double angle, const VectorType &origin = {} )
	{
		const double tX = static_cast<double>( x ) - origin.x;
		const double tY = static_cast<double>( y ) - origin.y;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		x = static_cast<T>( tX * cT - tY * sT + origin.x );
		y = static_cast<T>( tX * sT + tY * cT + origin.y );
		return *this;
	}

	VectorType &reflect( const VectorType &normal )
	{
		VectorType n { normal };
		this->sub( n.multiply( this->dot( normal ) * 2.0 ) );
		return *this;
	}

	VectorType &lerp( const VectorType &v, double amount )
	{
		x = static_cast<T>( x + ( v.x - x ) * amount );
		y = static_cast<T>( y + ( v.y - y ) * amount );
		z = static_cast<T>( z + ( v.z - z ) * amount );
		w = static_cast<T>( w + ( v.w - w ) * amount );
		return *this;
	}

	VectorType operator+( const VectorType &v ) const { VectorType r { *this }; r.add( v );      return r; }
	VectorType operator-( const VectorType &v ) const { VectorType r { *this }; r.sub( v );      return r; }
	VectorType operator*( const VectorType &v ) const { VectorType r { *this }; r.multiply( v ); return r; }
	VectorType operator*( double s )            const { VectorType r { *this }; r.multiply( s ); return r; }
	VectorType operator/( const VectorType &v ) const { VectorType r { *this }; r.divide( v );   return r; }
	VectorType operator/( double s )            const { VectorType r { *this }; r.divide( s );   return r; }

	VectorType &operator+=( const VectorType &v ) { this->add( v );      return *this; }
	VectorType &operator-=( const VectorType &v ) { this->sub( v );      return *this; }
	VectorType &operator*=( const VectorType &v ) { this->multiply( v ); return *this; }
	VectorType &operator*=( double s )            { this->multiply( s ); return *this; }
	VectorType &operator/=( const VectorType &v ) { this->divide( v );   return *this; }
	VectorType &operator/=( double s )            { this->divide( s );   return *this; }

	bool operator==( const VectorType &v ) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
};


#if !MATH_PRE_EXPANDED_VECTOR_TYPES

#define VECTOR_TYPE_4D_HEADER( Vec, T )                                               \
	extern template struct Vector4D<T>;                                               \
	extern bool Vec##_equal( const Vec &a, const Vec &b );                            \
	extern    T Vec##_dot( const Vec &a, const Vec &b );                              \
	extern    T Vec##_length( const Vec &v );                                         \
	extern    T Vec##_length_sqr( const Vec &v );                                     \
	extern  Vec Vec##_add( const Vec &a, const Vec &b );                              \
	extern  Vec Vec##_sub( const Vec &a, const Vec &b );                              \
	extern  Vec Vec##_divide( const Vec &a, const Vec &b );                           \
	extern  Vec Vec##_divide( const Vec &v, double s );                               \
	extern  Vec Vec##_multiply( const Vec &v, double s );                             \
	extern  Vec Vec##_multiply( const Vec &a, const Vec &b );                         \
	extern  Vec Vec##_multiply( const Vec &v, const double_m44 &m );                  \
	extern  Vec Vec##_multiply( const Vec &v, const float_m44 &m );                   \
	extern  Vec Vec##_cross( const Vec &a, const Vec &b );                            \
	extern  Vec Vec##_normalize( const Vec &v );                                      \
	extern  Vec Vec##_project( const Vec &v, const Vec &onto );                       \
	extern  Vec Vec##_rotate_x( const Vec &v, float angle, const Vec &origin = { } ); \
	extern  Vec Vec##_rotate_y( const Vec &v, float angle, const Vec &origin = { } ); \
	extern  Vec Vec##_rotate_z( const Vec &v, float angle, const Vec &origin = { } ); \
	extern  Vec Vec##_reflect( const Vec &v, const Vec &normal );                     \
	extern  Vec Vec##_lerp( const Vec &a, const Vec &b, float amount );               \
	extern    T Vec##_distance( const Vec &a, const Vec &b );                         \
	extern    T Vec##_distance_sqr( const Vec &a, const Vec &b );

VECTOR_TYPE_4D_HEADER( u8_v4, u8 )
VECTOR_TYPE_4D_HEADER( i16_v4, i16 )
VECTOR_TYPE_4D_HEADER( u16_v4, u16 )
VECTOR_TYPE_4D_HEADER( int_v4, i32 )
VECTOR_TYPE_4D_HEADER( u32_v4, u32 )
VECTOR_TYPE_4D_HEADER( i64_v4, i64 )
VECTOR_TYPE_4D_HEADER( u64_v4, u64 )
VECTOR_TYPE_4D_HEADER( float_v4, float )
VECTOR_TYPE_4D_HEADER( double_v4, double )

#else

extern bool u8_v4_equal( const u8_v4 &a, const u8_v4 &b );
extern u8 u8_v4_dot( const u8_v4 &a, const u8_v4 &b );
extern u8 u8_v4_length( const u8_v4 &v );
extern u8 u8_v4_length_sqr( const u8_v4 &v );
extern u8_v4 u8_v4_add( const u8_v4 &a, const u8_v4 &b );
extern u8_v4 u8_v4_sub( const u8_v4 &a, const u8_v4 &b );
extern u8_v4 u8_v4_divide( const u8_v4 &a, const u8_v4 &b );
extern u8_v4 u8_v4_divide( const u8_v4 &v, double s );
extern u8_v4 u8_v4_multiply( const u8_v4 &v, double s );
extern u8_v4 u8_v4_multiply( const u8_v4 &a, const u8_v4 &b );
extern u8_v4 u8_v4_multiply( const u8_v4 &v, const double_m44 &m );
extern u8_v4 u8_v4_multiply( const u8_v4 &v, const float_m44 &m );
extern u8_v4 u8_v4_cross( const u8_v4 &a, const u8_v4 &b );
extern u8_v4 u8_v4_normalize( const u8_v4 &v );
extern u8_v4 u8_v4_project( const u8_v4 &v, const u8_v4 &onto );
extern u8_v4 u8_v4_rotate_x( const u8_v4 &v, float angle, const u8_v4 &origin = { } );
extern u8_v4 u8_v4_rotate_y( const u8_v4 &v, float angle, const u8_v4 &origin = { } );
extern u8_v4 u8_v4_rotate_z( const u8_v4 &v, float angle, const u8_v4 &origin = { } );
extern u8_v4 u8_v4_reflect( const u8_v4 &v, const u8_v4 &normal );
extern u8_v4 u8_v4_lerp( const u8_v4 &a, const u8_v4 &b, float amount );
extern u8 u8_v4_distance( const u8_v4 &a, const u8_v4 &b );
extern u8 u8_v4_distance_sqr( const u8_v4 &a, const u8_v4 &b );

extern bool i16_v4_equal( const i16_v4 &a, const i16_v4 &b );
extern i16 i16_v4_dot( const i16_v4 &a, const i16_v4 &b );
extern i16 i16_v4_length( const i16_v4 &v );
extern i16 i16_v4_length_sqr( const i16_v4 &v );
extern i16_v4 i16_v4_add( const i16_v4 &a, const i16_v4 &b );
extern i16_v4 i16_v4_sub( const i16_v4 &a, const i16_v4 &b );
extern i16_v4 i16_v4_divide( const i16_v4 &a, const i16_v4 &b );
extern i16_v4 i16_v4_divide( const i16_v4 &v, double s );
extern i16_v4 i16_v4_multiply( const i16_v4 &v, double s );
extern i16_v4 i16_v4_multiply( const i16_v4 &a, const i16_v4 &b );
extern i16_v4 i16_v4_multiply( const i16_v4 &v, const double_m44 &m );
extern i16_v4 i16_v4_multiply( const i16_v4 &v, const float_m44 &m );
extern i16_v4 i16_v4_cross( const i16_v4 &a, const i16_v4 &b );
extern i16_v4 i16_v4_normalize( const i16_v4 &v );
extern i16_v4 i16_v4_project( const i16_v4 &v, const i16_v4 &onto );
extern i16_v4 i16_v4_rotate_x( const i16_v4 &v, float angle, const i16_v4 &origin = { } );
extern i16_v4 i16_v4_rotate_y( const i16_v4 &v, float angle, const i16_v4 &origin = { } );
extern i16_v4 i16_v4_rotate_z( const i16_v4 &v, float angle, const i16_v4 &origin = { } );
extern i16_v4 i16_v4_reflect( const i16_v4 &v, const i16_v4 &normal );
extern i16_v4 i16_v4_lerp( const i16_v4 &a, const i16_v4 &b, float amount );
extern i16 i16_v4_distance( const i16_v4 &a, const i16_v4 &b );
extern i16 i16_v4_distance_sqr( const i16_v4 &a, const i16_v4 &b );

extern bool u16_v4_equal( const u16_v4 &a, const u16_v4 &b );
extern u16 u16_v4_dot( const u16_v4 &a, const u16_v4 &b );
extern u16 u16_v4_length( const u16_v4 &v );
extern u16 u16_v4_length_sqr( const u16_v4 &v );
extern u16_v4 u16_v4_add( const u16_v4 &a, const u16_v4 &b );
extern u16_v4 u16_v4_sub( const u16_v4 &a, const u16_v4 &b );
extern u16_v4 u16_v4_divide( const u16_v4 &a, const u16_v4 &b );
extern u16_v4 u16_v4_divide( const u16_v4 &v, double s );
extern u16_v4 u16_v4_multiply( const u16_v4 &v, double s );
extern u16_v4 u16_v4_multiply( const u16_v4 &a, const u16_v4 &b );
extern u16_v4 u16_v4_multiply( const u16_v4 &v, const double_m44 &m );
extern u16_v4 u16_v4_multiply( const u16_v4 &v, const float_m44 &m );
extern u16_v4 u16_v4_cross( const u16_v4 &a, const u16_v4 &b );
extern u16_v4 u16_v4_normalize( const u16_v4 &v );
extern u16_v4 u16_v4_project( const u16_v4 &v, const u16_v4 &onto );
extern u16_v4 u16_v4_rotate_x( const u16_v4 &v, float angle, const u16_v4 &origin = { } );
extern u16_v4 u16_v4_rotate_y( const u16_v4 &v, float angle, const u16_v4 &origin = { } );
extern u16_v4 u16_v4_rotate_z( const u16_v4 &v, float angle, const u16_v4 &origin = { } );
extern u16_v4 u16_v4_reflect( const u16_v4 &v, const u16_v4 &normal );
extern u16_v4 u16_v4_lerp( const u16_v4 &a, const u16_v4 &b, float amount );
extern u16 u16_v4_distance( const u16_v4 &a, const u16_v4 &b );
extern u16 u16_v4_distance_sqr( const u16_v4 &a, const u16_v4 &b );

extern bool int_v4_equal( const int_v4 &a, const int_v4 &b );
extern i32 int_v4_dot( const int_v4 &a, const int_v4 &b );
extern i32 int_v4_length( const int_v4 &v );
extern i32 int_v4_length_sqr( const int_v4 &v );
extern int_v4 int_v4_add( const int_v4 &a, const int_v4 &b );
extern int_v4 int_v4_sub( const int_v4 &a, const int_v4 &b );
extern int_v4 int_v4_divide( const int_v4 &a, const int_v4 &b );
extern int_v4 int_v4_divide( const int_v4 &v, double s );
extern int_v4 int_v4_multiply( const int_v4 &v, double s );
extern int_v4 int_v4_multiply( const int_v4 &a, const int_v4 &b );
extern int_v4 int_v4_multiply( const int_v4 &v, const double_m44 &m );
extern int_v4 int_v4_multiply( const int_v4 &v, const float_m44 &m );
extern int_v4 int_v4_cross( const int_v4 &a, const int_v4 &b );
extern int_v4 int_v4_normalize( const int_v4 &v );
extern int_v4 int_v4_project( const int_v4 &v, const int_v4 &onto );
extern int_v4 int_v4_rotate_x( const int_v4 &v, float angle, const int_v4 &origin = { } );
extern int_v4 int_v4_rotate_y( const int_v4 &v, float angle, const int_v4 &origin = { } );
extern int_v4 int_v4_rotate_z( const int_v4 &v, float angle, const int_v4 &origin = { } );
extern int_v4 int_v4_reflect( const int_v4 &v, const int_v4 &normal );
extern int_v4 int_v4_lerp( const int_v4 &a, const int_v4 &b, float amount );
extern i32 int_v4_distance( const int_v4 &a, const int_v4 &b );
extern i32 int_v4_distance_sqr( const int_v4 &a, const int_v4 &b );

extern bool u32_v4_equal( const u32_v4 &a, const u32_v4 &b );
extern u32 u32_v4_dot( const u32_v4 &a, const u32_v4 &b );
extern u32 u32_v4_length( const u32_v4 &v );
extern u32 u32_v4_length_sqr( const u32_v4 &v );
extern u32_v4 u32_v4_add( const u32_v4 &a, const u32_v4 &b );
extern u32_v4 u32_v4_sub( const u32_v4 &a, const u32_v4 &b );
extern u32_v4 u32_v4_divide( const u32_v4 &a, const u32_v4 &b );
extern u32_v4 u32_v4_divide( const u32_v4 &v, double s );
extern u32_v4 u32_v4_multiply( const u32_v4 &v, double s );
extern u32_v4 u32_v4_multiply( const u32_v4 &a, const u32_v4 &b );
extern u32_v4 u32_v4_multiply( const u32_v4 &v, const double_m44 &m );
extern u32_v4 u32_v4_multiply( const u32_v4 &v, const float_m44 &m );
extern u32_v4 u32_v4_cross( const u32_v4 &a, const u32_v4 &b );
extern u32_v4 u32_v4_normalize( const u32_v4 &v );
extern u32_v4 u32_v4_project( const u32_v4 &v, const u32_v4 &onto );
extern u32_v4 u32_v4_rotate_x( const u32_v4 &v, float angle, const u32_v4 &origin = { } );
extern u32_v4 u32_v4_rotate_y( const u32_v4 &v, float angle, const u32_v4 &origin = { } );
extern u32_v4 u32_v4_rotate_z( const u32_v4 &v, float angle, const u32_v4 &origin = { } );
extern u32_v4 u32_v4_reflect( const u32_v4 &v, const u32_v4 &normal );
extern u32_v4 u32_v4_lerp( const u32_v4 &a, const u32_v4 &b, float amount );
extern u32 u32_v4_distance( const u32_v4 &a, const u32_v4 &b );
extern u32 u32_v4_distance_sqr( const u32_v4 &a, const u32_v4 &b );

extern bool i64_v4_equal( const i64_v4 &a, const i64_v4 &b );
extern i64 i64_v4_dot( const i64_v4 &a, const i64_v4 &b );
extern i64 i64_v4_length( const i64_v4 &v );
extern i64 i64_v4_length_sqr( const i64_v4 &v );
extern i64_v4 i64_v4_add( const i64_v4 &a, const i64_v4 &b );
extern i64_v4 i64_v4_sub( const i64_v4 &a, const i64_v4 &b );
extern i64_v4 i64_v4_divide( const i64_v4 &a, const i64_v4 &b );
extern i64_v4 i64_v4_divide( const i64_v4 &v, double s );
extern i64_v4 i64_v4_multiply( const i64_v4 &v, double s );
extern i64_v4 i64_v4_multiply( const i64_v4 &a, const i64_v4 &b );
extern i64_v4 i64_v4_multiply( const i64_v4 &v, const double_m44 &m );
extern i64_v4 i64_v4_multiply( const i64_v4 &v, const float_m44 &m );
extern i64_v4 i64_v4_cross( const i64_v4 &a, const i64_v4 &b );
extern i64_v4 i64_v4_normalize( const i64_v4 &v );
extern i64_v4 i64_v4_project( const i64_v4 &v, const i64_v4 &onto );
extern i64_v4 i64_v4_rotate_x( const i64_v4 &v, float angle, const i64_v4 &origin = { } );
extern i64_v4 i64_v4_rotate_y( const i64_v4 &v, float angle, const i64_v4 &origin = { } );
extern i64_v4 i64_v4_rotate_z( const i64_v4 &v, float angle, const i64_v4 &origin = { } );
extern i64_v4 i64_v4_reflect( const i64_v4 &v, const i64_v4 &normal );
extern i64_v4 i64_v4_lerp( const i64_v4 &a, const i64_v4 &b, float amount );
extern i64 i64_v4_distance( const i64_v4 &a, const i64_v4 &b );
extern i64 i64_v4_distance_sqr( const i64_v4 &a, const i64_v4 &b );

extern bool u64_v4_equal( const u64_v4 &a, const u64_v4 &b );
extern u64 u64_v4_dot( const u64_v4 &a, const u64_v4 &b );
extern u64 u64_v4_length( const u64_v4 &v );
extern u64 u64_v4_length_sqr( const u64_v4 &v );
extern u64_v4 u64_v4_add( const u64_v4 &a, const u64_v4 &b );
extern u64_v4 u64_v4_sub( const u64_v4 &a, const u64_v4 &b );
extern u64_v4 u64_v4_divide( const u64_v4 &a, const u64_v4 &b );
extern u64_v4 u64_v4_divide( const u64_v4 &v, double s );
extern u64_v4 u64_v4_multiply( const u64_v4 &v, double s );
extern u64_v4 u64_v4_multiply( const u64_v4 &a, const u64_v4 &b );
extern u64_v4 u64_v4_multiply( const u64_v4 &v, const double_m44 &m );
extern u64_v4 u64_v4_multiply( const u64_v4 &v, const float_m44 &m );
extern u64_v4 u64_v4_cross( const u64_v4 &a, const u64_v4 &b );
extern u64_v4 u64_v4_normalize( const u64_v4 &v );
extern u64_v4 u64_v4_project( const u64_v4 &v, const u64_v4 &onto );
extern u64_v4 u64_v4_rotate_x( const u64_v4 &v, float angle, const u64_v4 &origin = { } );
extern u64_v4 u64_v4_rotate_y( const u64_v4 &v, float angle, const u64_v4 &origin = { } );
extern u64_v4 u64_v4_rotate_z( const u64_v4 &v, float angle, const u64_v4 &origin = { } );
extern u64_v4 u64_v4_reflect( const u64_v4 &v, const u64_v4 &normal );
extern u64_v4 u64_v4_lerp( const u64_v4 &a, const u64_v4 &b, float amount );
extern u64 u64_v4_distance( const u64_v4 &a, const u64_v4 &b );
extern u64 u64_v4_distance_sqr( const u64_v4 &a, const u64_v4 &b );

extern bool float_v4_equal( const float_v4 &a, const float_v4 &b );
extern float float_v4_dot( const float_v4 &a, const float_v4 &b );
extern float float_v4_length( const float_v4 &v );
extern float float_v4_length_sqr( const float_v4 &v );
extern float_v4 float_v4_add( const float_v4 &a, const float_v4 &b );
extern float_v4 float_v4_sub( const float_v4 &a, const float_v4 &b );
extern float_v4 float_v4_divide( const float_v4 &a, const float_v4 &b );
extern float_v4 float_v4_divide( const float_v4 &v, double s );
extern float_v4 float_v4_multiply( const float_v4 &v, double s );
extern float_v4 float_v4_multiply( const float_v4 &a, const float_v4 &b );
extern float_v4 float_v4_multiply( const float_v4 &v, const double_m44 &m );
extern float_v4 float_v4_multiply( const float_v4 &v, const float_m44 &m );
extern float_v4 float_v4_cross( const float_v4 &a, const float_v4 &b );
extern float_v4 float_v4_normalize( const float_v4 &v );
extern float_v4 float_v4_project( const float_v4 &v, const float_v4 &onto );
extern float_v4 float_v4_rotate_x( const float_v4 &v, float angle, const float_v4 &origin = { } );
extern float_v4 float_v4_rotate_y( const float_v4 &v, float angle, const float_v4 &origin = { } );
extern float_v4 float_v4_rotate_z( const float_v4 &v, float angle, const float_v4 &origin = { } );
extern float_v4 float_v4_reflect( const float_v4 &v, const float_v4 &normal );
extern float_v4 float_v4_lerp( const float_v4 &a, const float_v4 &b, float amount );
extern float float_v4_distance( const float_v4 &a, const float_v4 &b );
extern float float_v4_distance_sqr( const float_v4 &a, const float_v4 &b );

extern bool double_v4_equal( const double_v4 &a, const double_v4 &b );
extern double double_v4_dot( const double_v4 &a, const double_v4 &b );
extern double double_v4_length( const double_v4 &v );
extern double double_v4_length_sqr( const double_v4 &v );
extern double_v4 double_v4_add( const double_v4 &a, const double_v4 &b );
extern double_v4 double_v4_sub( const double_v4 &a, const double_v4 &b );
extern double_v4 double_v4_divide( const double_v4 &a, const double_v4 &b );
extern double_v4 double_v4_divide( const double_v4 &v, double s );
extern double_v4 double_v4_multiply( const double_v4 &v, double s );
extern double_v4 double_v4_multiply( const double_v4 &a, const double_v4 &b );
extern double_v4 double_v4_multiply( const double_v4 &v, const double_m44 &m );
extern double_v4 double_v4_multiply( const double_v4 &v, const float_m44 &m );
extern double_v4 double_v4_cross( const double_v4 &a, const double_v4 &b );
extern double_v4 double_v4_normalize( const double_v4 &v );
extern double_v4 double_v4_project( const double_v4 &v, const double_v4 &onto );
extern double_v4 double_v4_rotate_x( const double_v4 &v, float angle, const double_v4 &origin = { } );
extern double_v4 double_v4_rotate_y( const double_v4 &v, float angle, const double_v4 &origin = { } );
extern double_v4 double_v4_rotate_z( const double_v4 &v, float angle, const double_v4 &origin = { } );
extern double_v4 double_v4_reflect( const double_v4 &v, const double_v4 &normal );
extern double_v4 double_v4_lerp( const double_v4 &a, const double_v4 &b, float amount );
extern double double_v4_distance( const double_v4 &a, const double_v4 &b );
extern double double_v4_distance_sqr( const double_v4 &a, const double_v4 &b );

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2D Rays

struct float_r2
{
	float_r2( const float_v2 &origin, const float_v2 &vector ) :
		origin { origin }, vector { vector } { };
	float_r2( float oX, float oY, float vX, float vY ) :
		origin { float_v2 { oX, oY } }, vector { float_v2 { vX, vY } } { };

	struct hit
	{
		float distance = 0.0f; // negative if no hit
		float_v2 normal = float_v2 { 0.0f, 0.0f };
	};

	float_v2 origin;
	float_v2 vector;
};


struct double_r2
{
	double_r2( const double_v2 &origin, const double_v2 &vector ) :
		origin { origin }, vector { vector } { };
	double_r2( double oX, double oY, double vX, double vY ) :
		origin { double_v2 { oX, oY } }, vector { double_v2 { vX, vY } } { };

	struct hit
	{
		double distance = 0.0; // negative if no hit
		double_v2 normal = double_v2 { 0.0, 0.0 };
	};

	double_v2 origin;
	double_v2 vector;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3D Rays

struct float_r3
{
	float_r3() :
		origin { float_v3 { 0.0f, 0.0f, 0.0f } }, vector { float_v3 { 1.0f, 0.0f, 0.0f } } { }
	float_r3( const float_v3 &origin, const float_v3 &vector ) :
		origin { origin }, vector { vector } { };
	float_r3( float oX, float oY, float oZ, float vX, float vY, float vZ ) :
		origin { float_v3 { oX, oY, oZ } }, vector { float_v3 { vX, vY, vZ } } { };

	struct hit
	{
		float distance = 0.0f;
		float baryU = 0.0f;
		float baryV = 0.0f;
		usize triangleID = USIZE_MAX;
	};

	float_v3 origin = float_v3 { 0.0f, 0.0f, 0.0f };
	float_v3 vector = float_v3 { 1.0f, 0.0f, 0.0f };
};


struct double_r3
{
	double_r3() :
		origin { double_v3 { 0.0, 0.0, 0.0 } }, vector { double_v3 { 1.0, 0.0, 0.0 } } { }
	double_r3( const double_v3 &origin, const double_v3 &vector ) :
		origin { origin }, vector { vector } { };
	double_r3( double oX, double oY, double oZ, double vX, double vY, double vZ ) :
		origin { double_v3 { oX, oY, oZ } }, vector { double_v3 { vX, vY, vZ } } { };

	struct hit
	{
		double distance = 0.0;
		double baryU = 0.0;
		double baryV = 0.0;
		usize triangleID = 0;
	};

	double_v3 origin;
	double_v3 vector;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern double_v4 quaternion_from_axis_radians( const double_v3 &axis, double radians );
extern double_v4 quaternion_normalize( const double_v4 &quat );
extern double_v4 quaternion_multiply( const double_v4 &a, const double_v4 &b );
extern double_v3 quaternion_rotate( const double_v4 &quat, const double_v3 &v );
extern double_v4 quaternion_from_forward_up( const double_v3 &forward, const double_v3 &up );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline float rsqrtf( float x )
{
#if defined( __clang__ ) || defined( __GNUC__ )
	return 1.0f / __builtin_sqrtf( x );
#else
	return 1.0f / sqrtf(x);
#endif
}

inline double rsqrt( double x )
{
#if defined( __clang__ ) || defined( __GNUC__ )
	return 1.0 / __builtin_sqrt( x );
#else
	return 1.0 / sqrt( x );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static constexpr int FLOAT_GRAPH_COUNT = 32;


struct RadialFloatNode
{
	RadialFloatNode() { };
	RadialFloatNode( float value, float time ) : value { value }, time { time } { };
	float value = 0.0f;
	float time = 0.0f;
};


class RadialFloatGraph
{
public:
	int add_node( RadialFloatNode node );
	void remove_node( int index );
	float get_value( float time ) const;

public:
	RadialFloatNode nodes[FLOAT_GRAPH_COUNT] = { };
	int count = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////