#pragma once

#include <core/types.hpp>

#include <vendor/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpful Constants & Conversion Utils

#define PI ( 3.14159265358979 )
#define DEG2RAD ( PI / 180.0 )
#define RAD2DEG ( 180.0 / PI )

#define PI_F ( 3.14159265358979f )
#define DEG2RAD_F ( PI_F / 180.0f )
#define RAD2DEG_F ( 180.0f / PI_F )

#define ALIGN_16( value ) ( ( static_cast<int>( value ) >> 4 ) << 4 )
#define ALIGN_16_CEIL( value ) ( ( ( static_cast<int>( value ) >> 4 ) + 1 ) << 4 )
#define ALIGN_POWER( value, power ) ( ( static_cast<int>( value ) >> power ) << power )
#define ALIGN_POWER_CEIL( value, power ) ( ( ( static_cast<int>( value ) >> power ) + 1 ) << power )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations

template <typename T> struct Vector2D;
#define VECTOR_TYPE_2D_FORWARD_DECL( Vec, T ) using Vec = Vector2D<T>;
VECTOR_TYPE_2D_FORWARD_DECL( u8_v2, u8 )
VECTOR_TYPE_2D_FORWARD_DECL( i16_v2, i16 )
VECTOR_TYPE_2D_FORWARD_DECL( u16_v2, u16 )
VECTOR_TYPE_2D_FORWARD_DECL( int_v2, i32 )
VECTOR_TYPE_2D_FORWARD_DECL( u32_v2, u32 )
VECTOR_TYPE_2D_FORWARD_DECL( i64_v2, i64 )
VECTOR_TYPE_2D_FORWARD_DECL( u64_v2, u64 )
VECTOR_TYPE_2D_FORWARD_DECL( float_v2, float )
VECTOR_TYPE_2D_FORWARD_DECL( double_v2, double )

template <typename T> struct Vector3D;
#define VECTOR_TYPE_3D_FORWARD_DECL( Vec, T ) using Vec = Vector3D<T>;
VECTOR_TYPE_3D_FORWARD_DECL( u8_v3, u8 )
VECTOR_TYPE_3D_FORWARD_DECL( i16_v3, i16 )
VECTOR_TYPE_3D_FORWARD_DECL( u16_v3, u16 )
VECTOR_TYPE_3D_FORWARD_DECL( int_v3, i32 )
VECTOR_TYPE_3D_FORWARD_DECL( u32_v3, u32 )
VECTOR_TYPE_3D_FORWARD_DECL( i64_v3, i64 )
VECTOR_TYPE_3D_FORWARD_DECL( u64_v3, u64 )
VECTOR_TYPE_3D_FORWARD_DECL( float_v3, float )
VECTOR_TYPE_3D_FORWARD_DECL( double_v3, double )

template <typename T> struct Vector4D;
#define VECTOR_TYPE_4D_FORWARD_DECL( Vec, T ) using Vec = Vector4D<T>;
VECTOR_TYPE_4D_FORWARD_DECL( u8_v4, u8 )
VECTOR_TYPE_4D_FORWARD_DECL( i16_v4, i16 )
VECTOR_TYPE_4D_FORWARD_DECL( u16_v4, u16 )
VECTOR_TYPE_4D_FORWARD_DECL( int_v4, i32 )
VECTOR_TYPE_4D_FORWARD_DECL( u32_v4, u32 )
VECTOR_TYPE_4D_FORWARD_DECL( i64_v4, i64 )
VECTOR_TYPE_4D_FORWARD_DECL( u64_v4, u64 )
VECTOR_TYPE_4D_FORWARD_DECL( float_v4, float )
VECTOR_TYPE_4D_FORWARD_DECL( double_v4, double )

struct float_r2;
struct double_r2;
struct float_r3;
struct double_r3;
struct float_r4;
struct double_r4;

struct float_m44;
struct double_m44;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern u32 ceilpow2( u32 v );
extern u64 ceilpow2( u64 v );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !!USE_CUSTOM_C_HEADERS
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

template <typename T>
struct Vector2D
{
	using VectorType = Vector2D<T>;
	T x, y;

	Vector2D( T x = 0, T y = 0 ) : x { x }, y { y } { }
	Vector2D( const VectorType &v ) : x { v.x }, y { v.y } { }
	VectorType &operator=( const VectorType &v ) { x = v.x; y = v.y; return *this; }

	/* Automatically cast vector types (if valid component cast exists) */
	template <typename S> Vector2D( Vector2D<S> &v ) :
		x{ static_cast<T>( v.x ) }, y{ static_cast<T>( v.y ) } { }
	template <typename S> Vector2D( S x, S y ) :
		x{ static_cast<T>( x ) }, y{ static_cast<T>( y ) } { }

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


#define VECTOR_TYPE_2D( Vec, T )                                                                                  \
	inline bool Vec##_equal( const Vec &a, const Vec &b ) { return a == b; }                                      \
	inline    T Vec##_dot( const Vec &a, const Vec &b ) { return a.dot( b ); }                                    \
	inline    T Vec##_length( const Vec &v ) { return v.length(); }                                               \
	inline    T Vec##_length_sqr( const Vec &v ) { return v.length_sqr(); }                                       \
	inline  Vec Vec##_add( const Vec &a, const Vec &b ) { Vec r { a }; r.add( b ); return r; }                    \
	inline  Vec Vec##_sub( const Vec &a, const Vec &b ) { Vec r { a }; r.sub( b ); return r; }                    \
	inline  Vec Vec##_divide( const Vec &a, const Vec &b ) { Vec r { a }; r.divide( b ); return r; }              \
	inline  Vec Vec##_divide( const Vec &v, double s ) { Vec r { v }; r.divide( s ); return r; }                  \
	inline  Vec Vec##_multiply( const Vec &v, double s ) { Vec r { v }; r.multiply( s ); return r; }              \
	inline  Vec Vec##_multiply( const Vec &a, const Vec &b ) { Vec r { a }; r.multiply( b ); return r; }          \
	inline  Vec Vec##_multiply( const Vec &v, const double_m44 &m ) { Vec r { v }; r.multiply( m ); return r; }   \
	inline  Vec Vec##_multiply( const Vec &v, const float_m44 &m ) { Vec r { v }; r.multiply( m ); return r; }    \
	inline    T Vec##_cross( const Vec &a, const Vec &b ) { return a.cross( b ); }                                \
	inline  Vec Vec##_normalize( const Vec &v ) { Vec r { v }; r.normalize(); return r; }                         \
	inline  Vec Vec##_project( const Vec &v, const Vec &onto ) { Vec r { v }; r.project( onto ); return r; }      \
	inline  Vec Vec##_rotate( const Vec &v, float angle, const Vec &origin = { } )                                \
		{ Vec r { v }; r.rotate( angle, origin ); return r; }                                                     \
	inline  Vec Vec##_reflect( const Vec &v, const Vec &normal )                                                  \
		{ Vec r { v }; r.reflect( normal ); return r; }                                                           \
	inline Vec Vec##_lerp( const Vec &a, const Vec &b, float amount )                                             \
		{ Vec r { a }; r.lerp( b, amount ); return r; }                                                           \
	inline   T Vec##_distance( const Vec &a, const Vec &b )                                                       \
		{ Vec r { b.x - a.x, b.y - a.y }; return r.length(); }                                                    \
	inline   T Vec##_distance_sqr( const Vec &a, const Vec &b )                                                   \
		{ Vec r { b.x - a.x, b.y - a.y }; return r.length_sqr(); }


VECTOR_TYPE_2D( u8_v2, u8 )
VECTOR_TYPE_2D( i16_v2, i16 )
VECTOR_TYPE_2D( u16_v2, u16 )
VECTOR_TYPE_2D( int_v2, i32 )
VECTOR_TYPE_2D( u32_v2, u32 )
VECTOR_TYPE_2D( i64_v2, i64 )
VECTOR_TYPE_2D( u64_v2, u64 )
VECTOR_TYPE_2D( float_v2, float )
VECTOR_TYPE_2D( double_v2, double )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3D Vector

template <typename T>
struct Vector3D
{
	using VectorType = Vector3D<T>;
	T x, y, z;

	Vector3D( T x = 0, T y = 0, T z = 0 ) : x { x }, y { y }, z { z } { }
	Vector3D( const VectorType &v ) : x { v.x }, y { v.y }, z { v.z } { }
	VectorType &operator=( const VectorType &v ) { x = v.x; y = v.y; z = v.z; return *this; }

	/* Automatically cast vector types (if valid component cast exists) */
	template <typename S> Vector3D( Vector3D<S> &v ) :
		x{ static_cast<T>( v.x ) }, y{ static_cast<T>( v.y ) }, z{ static_cast<T>( v.z ) } { }
	template <typename S> Vector3D( S x, S y, S z ) :
		x{ static_cast<T>( x ) }, y{ static_cast<T>( y ) }, z{ static_cast<T>( z ) } { }

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


#define VECTOR_TYPE_3D( Vec, T )                                                                                \
	inline bool Vec##_equal( const Vec &a, const Vec &b ) { return a == b; }                                    \
	inline    T Vec##_dot( const Vec &a, const Vec &b ) { return a.dot( b ); }                                  \
	inline    T Vec##_length( const Vec &v ) { return v.length(); }                                             \
	inline    T Vec##_length_sqr( const Vec &v ) { return v.length_sqr(); }                                     \
	inline  Vec Vec##_add( const Vec &a, const Vec &b ) { Vec r { a }; r.add( b ); return r; }                  \
	inline  Vec Vec##_sub( const Vec &a, const Vec &b ) { Vec r { a }; r.sub( b ); return r; }                  \
	inline  Vec Vec##_divide( const Vec &a, const Vec &b ) { Vec r { a }; r.divide( b ); return r; }            \
	inline  Vec Vec##_divide( const Vec &v, double s ) { Vec r { v }; r.divide( s ); return r; }                \
	inline  Vec Vec##_multiply( const Vec &v, double s ) { Vec r { v }; r.multiply( s ); return r; }            \
	inline  Vec Vec##_multiply( const Vec &a, const Vec &b ) { Vec r { a }; r.multiply( b ); return r; }        \
	inline  Vec Vec##_multiply( const Vec &v, const double_m44 &m ) { Vec r { v }; r.multiply( m ); return r; } \
	inline  Vec Vec##_multiply( const Vec &v, const float_m44 &m ) { Vec r { v }; r.multiply( m ); return r; }  \
	inline  Vec Vec##_cross( const Vec &a, const Vec &b ) { Vec r { a }; r.cross( b ); return r; }              \
	inline  Vec Vec##_normalize( const Vec &v ) { Vec r { v }; r.normalize(); return r; }                       \
	inline  Vec Vec##_project( const Vec &v, const Vec &onto ) { Vec r { v }; r.project( onto ); return r; }    \
	inline  Vec Vec##_rotate_x( const Vec &v, float angle, const Vec &origin = { } )                            \
		{ Vec r { v }; r.rotate_x( angle, origin ); return r; }                                                 \
	inline  Vec Vec##_rotate_y( const Vec &v, float angle, const Vec &origin = { } )                            \
		{ Vec r { v }; r.rotate_y( angle, origin ); return r; }                                                 \
	inline  Vec Vec##_rotate_z( const Vec &v, float angle, const Vec &origin = { } )                            \
		{ Vec r { v }; r.rotate_z( angle, origin ); return r; }                                                 \
	inline  Vec Vec##_reflect( const Vec &v, const Vec &normal )                                                \
		{ Vec r { v }; r.reflect( normal ); return r; }                                                         \
	inline Vec Vec##_lerp( const Vec &a, const Vec &b, float amount )                                           \
		{ Vec r { a }; r.lerp( b, amount ); return r; }                                                         \
	inline   T Vec##_distance( const Vec &a, const Vec &b )                                                     \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z }; return r.length(); }                                       \
	inline   T Vec##_distance_sqr( const Vec &a, const Vec &b )                                                 \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z }; return r.length_sqr(); }


VECTOR_TYPE_3D( u8_v3, u8 )
VECTOR_TYPE_3D( i16_v3, i16 )
VECTOR_TYPE_3D( u16_v3, u16 )
VECTOR_TYPE_3D( int_v3, i32 )
VECTOR_TYPE_3D( u32_v3, u32 )
VECTOR_TYPE_3D( i64_v3, i64 )
VECTOR_TYPE_3D( u64_v3, u64 )
VECTOR_TYPE_3D( float_v3, float )
VECTOR_TYPE_3D( double_v3, double )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 4D Vector

template <typename T>
struct Vector4D
{
	using VectorType = Vector4D<T>;
	T x, y, z, w;

	Vector4D( T x = 0, T y = 0, T z = 0, T w = 0 ) : x { x }, y { y }, z { z }, w { w } { }
	Vector4D( const VectorType &v ) : x { v.x }, y { v.y }, z { v.z }, w { v.w } { }
	VectorType &operator=( const VectorType &v ) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

	/* Automatically cast vector types (if valid component cast exists) */
	template <typename S> Vector4D( Vector4D<S> &v ) :
		x{ static_cast<T>( v.x ) }, y{ static_cast<T>( v.y ) }, z{ static_cast<T>( v.z ) }, w{ static_cast<T>( v.w ) } { }
	template <typename S> Vector4D( S x, S y, S z, S w ) :
		x{ static_cast<T>( x ) }, y{ static_cast<T>( y ) }, z{ static_cast<T>( z ) }, w{ static_cast<T>( w ) } { }

	template <typename S> VectorType &operator=( const Vector3D<S> &v )
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
		// TODO: cross Vector4D
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


#define VECTOR_TYPE_4D( Vec, T )                                                                                \
	inline bool Vec##_equal( const Vec &a, const Vec &b ) { return a == b; }                                    \
	inline    T Vec##_dot( const Vec &a, const Vec &b ) { return a.dot( b ); }                                  \
	inline    T Vec##_length( const Vec &v ) { return v.length(); }                                             \
	inline    T Vec##_length_sqr( const Vec &v ) { return v.length_sqr(); }                                     \
	inline  Vec Vec##_add( const Vec &a, const Vec &b ) { Vec r { a }; r.add( b ); return r; }                  \
	inline  Vec Vec##_sub( const Vec &a, const Vec &b ) { Vec r { a }; r.sub( b ); return r; }                  \
	inline  Vec Vec##_divide( const Vec &a, const Vec &b ) { Vec r { a }; r.divide( b ); return r; }            \
	inline  Vec Vec##_divide( const Vec &v, double s ) { Vec r { v }; r.divide( s ); return r; }                \
	inline  Vec Vec##_multiply( const Vec &v, double s ) { Vec r { v }; r.multiply( s ); return r; }            \
	inline  Vec Vec##_multiply( const Vec &a, const Vec &b ) { Vec r { a }; r.multiply( b ); return r; }        \
	inline  Vec Vec##_multiply( const Vec &v, const double_m44 &m ) { Vec r { v }; r.multiply( m ); return r; } \
	inline  Vec Vec##_multiply( const Vec &v, const float_m44 &m ) { Vec r { v }; r.multiply( m ); return r; }  \
	inline  Vec Vec##_cross( const Vec &a, const Vec &b ) { Vec r { a }; r.cross( b ); return r; }              \
	inline  Vec Vec##_normalize( const Vec &v ) { Vec r { v }; r.normalize(); return r; }                       \
	inline  Vec Vec##_project( const Vec &v, const Vec &onto ) { Vec r { v }; r.project( onto ); return r; }    \
	inline  Vec Vec##_rotate_x( const Vec &v, float angle, const Vec &origin = { } )                            \
		{ Vec r { v }; r.rotate_x( angle, origin ); return r; }                                                 \
	inline  Vec Vec##_rotate_y( const Vec &v, float angle, const Vec &origin = { } )                            \
		{ Vec r { v }; r.rotate_y( angle, origin ); return r; }                                                 \
	inline  Vec Vec##_rotate_z( const Vec &v, float angle, const Vec &origin = { } )                            \
		{ Vec r { v }; r.rotate_z( angle, origin ); return r; }                                                 \
	inline  Vec Vec##_reflect( const Vec &v, const Vec &normal )                                                \
		{ Vec r { v }; r.reflect( normal ); return r; }                                                         \
	inline Vec Vec##_lerp( const Vec &a, const Vec &b, float amount )                                           \
		{ Vec r { a }; r.lerp( b, amount ); return r; }                                                         \
	inline   T Vec##_distance( const Vec &a, const Vec &b )                                                     \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z, b.w - a.w }; return r.length(); }                            \
	inline   T Vec##_distance_sqr( const Vec &a, const Vec &b )                                                 \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z, b.w - a.w }; return r.length_sqr(); }


VECTOR_TYPE_4D( u8_v4, u8 )
VECTOR_TYPE_4D( i16_v4, i16 )
VECTOR_TYPE_4D( u16_v4, u16 )
VECTOR_TYPE_4D( int_v4, i32 )
VECTOR_TYPE_4D( u32_v4, u32 )
VECTOR_TYPE_4D( i64_v4, i64 )
VECTOR_TYPE_4D( u64_v4, u64 )
VECTOR_TYPE_4D( float_v4, float )
VECTOR_TYPE_4D( double_v4, double )

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

inline double_v4 quaternion_from_axis_radians( const double_v3 &axis, double radians )
{
	const double s = sin( radians * 0.5 );
	return double_v4 { axis.x * s, axis.y * s, axis.z * s, cos( radians * 0.5 ) };
}


inline double_v4 quaternion_normalize( const double_v4 &quat )
{
	double nInv = 1.0 / quat.length();
	return double_v4 { quat.x * nInv, quat.y * nInv, quat.z * nInv, quat.w * nInv };
}


inline double_v4 quaternion_multiply( const double_v4 &a, const double_v4 &b )
{
	return double_v4
		{
			a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
			a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
			a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
			a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
		};
}


inline double_v3 quaternion_rotate( const double_v4 &quat, const double_v3 &v )
{
	// rotate v by q: v' = q * (0,v) * q^-1
	const double_v3 qv = double_v3 { quat.x, quat.y, quat.z };
	const double_v3 t = double_v3_cross( qv, v ) * 2.0;
	const double_v3 result = v + t * quat.w + double_v3_cross( qv, t );
	return result;
}


inline double_v4 quaternion_from_forward_up( const double_v3 &forward, const double_v3 &up )
{
	const double_v3 f = double_v3_normalize( forward );
	const double_v3 r = double_v3_normalize( double_v3_cross( up, f ) );
	const double_v3 u = double_v3_cross( f, r );

	// +X = forward, +Y = right, +Z = up
	double_m44 m = double_m44_build_zeros();
	m[0] = f.x;
	m[1] = r.x;
	m[2] = u.x;
	m[4] = f.y;
	m[5] = r.y;
	m[6] = u.y;
	m[8] = f.z;
	m[9] = r.z;
	m[10]= u.z;
	m[15] = 1.0;

	// Convert Euler 3x3 rotation matrix to double_v4 Quaternion orientation
	double_v4 q;
	double trace = m[0] + m[5] + m[10];

	if( trace > 0.0 )
	{
		const double s = 0.5 / sqrt( trace + 1.0 );
		q.x = ( m[9] - m[6] ) * s;
		q.y = ( m[2] - m[8] ) * s;
		q.z = ( m[4] - m[1] ) * s;
		q.w = 0.25 / s;
	}
	else if( ( m[0] > m[5] ) && ( m[0] > m[10] ) )
	{
		const double s = 2.0 * sqrt( 1.0 + m[0] - m[5] - m[10] );
		q.x = 0.25 * s;
		q.y = ( m[1] + m[4] ) / s;
		q.z = ( m[2] + m[8] ) / s;
		q.w = ( m[9] - m[6] ) / s;
	}
	else if( m[5] > m[10] )
	{
		const double s = 2.0 * sqrt( 1.0 + m[5] - m[0] - m[10] );
		q.x = ( m[1] + m[4] ) / s;
		q.y = 0.25 * s;
		q.z = ( m[6] + m[9] ) / s;
		q.w = ( m[2] - m[8] ) / s;
	}
	else
	{
		const double s = 2.0 * sqrt( 1.0 + m[10] - m[0] - m[5] );
		q.x = ( m[2] + m[8] ) / s;
		q.y = ( m[6] + m[9] ) / s;
		q.z = 0.25 * s;
		q.w = ( m[4] - m[1] ) / s;
	}

	return quaternion_normalize( q );
}

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