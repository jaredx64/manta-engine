#pragma once

#include <core/types.hpp>
#include <core/math.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Vector2D
{
	using VectorType = Vector2D<T>;
	T x, y;

	Vector2D( T x = 0, T y = 0 ) : x{ x }, y{ y } { }
	Vector2D( const VectorType &v ) : x{ v.x }, y{ v.y } { }
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

    VectorType &multiply( const Matrix &matrix )
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

		x = static_cast<T>( r[0] / r[3] );
		y = static_cast<T>( r[1] / r[3] );
		return *this;
    }

	VectorType &divide( const VectorType &v )
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	VectorType &divide( const double s )
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

	VectorType &rotate( const double angle, const VectorType &origin = { } )
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

	VectorType operator+( const VectorType &v ) const { VectorType r { *this }; r.add( v );      return r; }
	VectorType operator-( const VectorType &v ) const { VectorType r { *this }; r.sub( v );      return r; }
	VectorType operator*( const VectorType &v ) const { VectorType r { *this }; r.multiply( v ); return r; }
	VectorType operator*( const double s )      const { VectorType r { *this }; r.multiply( s ); return r; }
	VectorType operator/( const VectorType &v ) const { VectorType r { *this }; r.divide( v );   return r; }
	VectorType operator/( const double s )      const { VectorType r { *this }; r.divide( s );   return r; }

	VectorType &operator+=( const VectorType &v ) { this->add( v );      return *this; }
	VectorType &operator-=( const VectorType &v ) { this->sub( v );      return *this; }
	VectorType &operator*=( const VectorType &v ) { this->multiply( v ); return *this; }
	VectorType &operator*=( const double s )      { this->multiply( s ); return *this; }
	VectorType &operator/=( const VectorType &v ) { this->divide( v );   return *this; }
	VectorType &operator/=( const double s )      { this->divide( s );   return *this; }

	bool operator==( const VectorType &v ) const { return x == v.x && y == v.y; }
};


#define VECTOR_TYPE_2D( Vec, T )                                                                             \
	using Vec = Vector2D<T>;                                                                                 \
	inline bool Vec##_equal( const Vec &a, const Vec &b )      { return a == b; }                            \
	inline    T Vec##_dot( const Vec &a, const Vec &b )        { return a.dot( b ); }                        \
	inline    T Vec##_length( const Vec &v )                   { return v.length(); }                        \
	inline    T Vec##_length_sqr( const Vec &v )               { return v.length_sqr(); }                    \
	inline  Vec Vec##_add( const Vec &a, const Vec &b )        { Vec r { a }; r.add( b ); return r; }        \
	inline  Vec Vec##_sub( const Vec &a, const Vec &b )        { Vec r { a }; r.sub( b ); return r; }        \
	inline  Vec Vec##_multiply( const Vec &a, const Vec &b )   { Vec r { a }; r.multiply( b ); return r; }   \
	inline  Vec Vec##_divide( const Vec &a, const Vec &b )     { Vec r { a }; r.divide( b ); return r; }     \
	inline  Vec Vec##_multiply( const Vec &v, const double s ) { Vec r { v }; r.multiply( s ); return r; }   \
	inline  Vec Vec##_divide( const Vec &v, const double s )   { Vec r { v }; r.divide( s ); return r; }     \
	inline    T Vec##_cross( const Vec &a, const Vec &b )      { return a.cross( b ); }                      \
	inline  Vec Vec##_normalize( const Vec &v )                { Vec r { v }; r.normalize(); return r; }     \
	inline  Vec Vec##_project( const Vec &v, const Vec &onto ) { Vec r { v }; r.project( onto ); return r; } \
	inline  Vec Vec##_rotate( const Vec &v, const float angle, const Vec &origin = { } )                     \
		{ Vec r { v }; r.rotate( angle, origin ); return r; }                                                \
	inline  Vec Vec##_reflect( const Vec &v, const Vec &normal )                                             \
		{ Vec r { v }; r.reflect( normal ); return r; }                                                      \
	inline Vec Vec##_lerp( const Vec &a, const Vec &b, const float amount )                                  \
		{ Vec r { a }; r.lerp( b, amount ); return r; }                                                      \
	inline   T Vec##_distance( const Vec &a, const Vec &b )                                                  \
		{ Vec r { b.x - a.x, b.y - a.y }; return r.length(); }                                               \
	inline   T Vec##_distance_sqr( const Vec &a, const Vec &b )                                              \
		{ Vec r { b.x - a.x, b.y - a.y }; return r.length_sqr(); }


VECTOR_TYPE_2D( i8v2, i8 )
VECTOR_TYPE_2D( u8v2, u8 )
VECTOR_TYPE_2D( i16v2, i16 )
VECTOR_TYPE_2D( u16v2, u16 )
VECTOR_TYPE_2D( intv2, i32 )
VECTOR_TYPE_2D( u32v2, u32 )
VECTOR_TYPE_2D( i64v2, i64 )
VECTOR_TYPE_2D( u64v2, u64 )
VECTOR_TYPE_2D( boolv2, bool )
VECTOR_TYPE_2D( floatv2, float )
VECTOR_TYPE_2D( doublev2, double )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Vector3D
{
	using VectorType = Vector3D<T>;
	T x, y, z;

	Vector3D( T x = 0, T y = 0, T z = 0 ) : x{ x }, y{ y }, z{ z } { }
	Vector3D( const VectorType &v ) : x{ v.x }, y{ v.y }, z{ v.z } { }
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

	VectorType &multiply( const double s )
	{
		x = static_cast<T>( x * s );
		y = static_cast<T>( y * s );
		z = static_cast<T>( z * s );
		return *this;
	}

	VectorType &multiply( const Matrix &matrix )
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

		x = static_cast<T>( r[0] / r[3] );
		y = static_cast<T>( r[1] / r[3] );
		z = static_cast<T>( r[2] / r[3] );
		return *this;
	}

	VectorType &divide( const VectorType &v )
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	VectorType &divide( const double s )
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

	VectorType &rotate_x( const double angle, const VectorType &origin = {} )
	{
		const double tY = static_cast<double>( y ) - origin.y;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		y = static_cast<T>( tY * cT - tZ * sT + origin.y );
		z = static_cast<T>( tY * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_y( const double angle, const VectorType &origin = {} )
	{
		const double tX = static_cast<double>( x )- origin.x;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		x = static_cast<T>( tX * cT + tZ * sT + origin.x );
		z = static_cast<T>( -tX * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_z( const double angle, const VectorType &origin = {} )
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

	VectorType &lerp( const VectorType &v, const double amount )
	{
		x = static_cast<T>( x + ( v.x - x ) * amount );
		y = static_cast<T>( y + ( v.y - y ) * amount );
		z = static_cast<T>( z + ( v.z - z ) * amount );
		return *this;
	}

	VectorType operator+( const VectorType &v ) const { VectorType r { *this }; r.add( v );      return r; }
	VectorType operator-( const VectorType &v ) const { VectorType r { *this }; r.sub( v );      return r; }
	VectorType operator*( const VectorType &v ) const { VectorType r { *this }; r.multiply( v ); return r; }
	VectorType operator*( const double s )      const { VectorType r { *this }; r.multiply( s ); return r; }
	VectorType operator/( const VectorType &v ) const { VectorType r { *this }; r.divide( v );   return r; }
	VectorType operator/( const double s )      const { VectorType r { *this }; r.divide( s );   return r; }

	VectorType &operator+=( const VectorType &v ) { this->add( v );      return *this; }
	VectorType &operator-=( const VectorType &v ) { this->sub( v );      return *this; }
	VectorType &operator*=( const VectorType &v ) { this->multiply( v ); return *this; }
	VectorType &operator*=( const double s )      { this->multiply( s ); return *this; }
	VectorType &operator/=( const VectorType &v ) { this->divide( v );   return *this; }
	VectorType &operator/=( const double s )      { this->divide( s );   return *this; }

	bool operator==( const VectorType &v ) const { return x == v.x && y == v.y && z == v.z; }
};


#define VECTOR_TYPE_3D( Vec, T )                                                                             \
	using Vec = Vector3D<T>;                                                                                 \
	inline bool Vec##_equal( const Vec &a, const Vec &b )      { return a == b; }                            \
	inline    T Vec##_dot( const Vec &a, const Vec &b )        { return a.dot( b ); }                        \
	inline    T Vec##_length( const Vec &v )                   { return v.length(); }                        \
	inline    T Vec##_length_sqr( const Vec &v )               { return v.length_sqr(); }                    \
	inline  Vec Vec##_add( const Vec &a, const Vec &b )        { Vec r { a }; r.add( b ); return r; }        \
	inline  Vec Vec##_sub( const Vec &a, const Vec &b )        { Vec r { a }; r.sub( b ); return r; }        \
	inline  Vec Vec##_multiply( const Vec &a, const Vec &b )   { Vec r { a }; r.multiply( b ); return r; }   \
	inline  Vec Vec##_divide( const Vec &a, const Vec &b )     { Vec r { a }; r.divide( b ); return r; }     \
	inline  Vec Vec##_multiply( const Vec &v, const double s ) { Vec r { v }; r.multiply( s ); return r; }   \
	inline  Vec Vec##_divide( const Vec &v, const double s )   { Vec r { v }; r.divide( s ); return r; }     \
	inline  Vec Vec##_cross( const Vec &a, const Vec &b )      { Vec r { a }; r.cross( b ); return r; }      \
	inline  Vec Vec##_normalize( const Vec &v )                { Vec r { v }; r.normalize(); return r; }     \
	inline  Vec Vec##_project( const Vec &v, const Vec &onto ) { Vec r { v }; r.project( onto ); return r; } \
	inline  Vec Vec##_rotate_x( const Vec &v, const float angle, const Vec &origin = { } )                   \
		{ Vec r { v }; r.rotate_x( angle, origin ); return r; }                                              \
	inline  Vec Vec##_rotate_y( const Vec &v, const float angle, const Vec &origin = { } )                   \
		{ Vec r { v }; r.rotate_y( angle, origin ); return r; }                                              \
	inline  Vec Vec##_rotate_z( const Vec &v, const float angle, const Vec &origin = { } )                   \
		{ Vec r { v }; r.rotate_z( angle, origin ); return r; }                                              \
	inline  Vec Vec##_reflect( const Vec &v, const Vec &normal )                                             \
		{ Vec r { v }; r.reflect( normal ); return r; }                                                      \
	inline Vec Vec##_lerp( const Vec &a, const Vec &b, const float amount )                                  \
		{ Vec r { a }; r.lerp( b, amount ); return r; }                                                      \
	inline   T Vec##_distance( const Vec &a, const Vec &b )                                                  \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z }; return r.length(); }                                    \
	inline   T Vec##_distance_sqr( const Vec &a, const Vec &b )                                              \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z }; return r.length_sqr(); }


VECTOR_TYPE_3D( i8v3, i8 )
VECTOR_TYPE_3D( u8v3, u8 )
VECTOR_TYPE_3D( i16v3, i16 )
VECTOR_TYPE_3D( u16v3, u16 )
VECTOR_TYPE_3D( intv3, i32 )
VECTOR_TYPE_3D( u32v3, u32 )
VECTOR_TYPE_3D( i64v3, i64 )
VECTOR_TYPE_3D( u64v3, u64 )
VECTOR_TYPE_3D( boolv3, bool );
VECTOR_TYPE_3D( floatv3, float )
VECTOR_TYPE_3D( doublev3, double )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Vector4D
{
	using VectorType = Vector4D<T>;
	T x, y, z, w;

	Vector4D( T x = 0, T y = 0, T z = 0, T w = 0 ) : x{ x }, y{ y }, z{ z }, w{ w } { }
	Vector4D( const VectorType &v ) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } { }
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

	VectorType &multiply( const double s )
	{
		x = static_cast<T>( x * s );
		y = static_cast<T>( y * s );
		z = static_cast<T>( z * s );
		w = static_cast<T>( w * s );
		return *this;
	}

	VectorType &multiply( const Matrix &matrix )
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

		x = static_cast<T>( r[0] / r[3] );
		y = static_cast<T>( r[1] / r[3] );
		z = static_cast<T>( r[2] / r[3] );
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

	VectorType &divide( const double s )
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

	VectorType &rotate_x( const double angle, const VectorType &origin = {} )
	{
		const double tY = static_cast<double>( y ) - origin.y;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		y = static_cast<T>( tY * cT - tZ * sT + origin.y );
		z = static_cast<T>( tY * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_y( const double angle, const VectorType &origin = {} )
	{
		const double tX = static_cast<double>( x )- origin.x;
		const double tZ = static_cast<double>( z ) - origin.z;
		const double cT = cos( angle * DEG2RAD );
		const double sT = sin( angle * DEG2RAD );
		x = static_cast<T>( tX * cT + tZ * sT + origin.x );
		z = static_cast<T>( -tX * sT + tZ * cT + origin.z );
		return *this;
	}

	VectorType &rotate_z( const double angle, const VectorType &origin = {} )
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

	VectorType &lerp( const VectorType &v, const double amount )
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
	VectorType operator*( const double s )      const { VectorType r { *this }; r.multiply( s ); return r; }
	VectorType operator/( const VectorType &v ) const { VectorType r { *this }; r.divide( v );   return r; }
	VectorType operator/( const double s )      const { VectorType r { *this }; r.divide( s );   return r; }

	VectorType &operator+=( const VectorType &v ) { this->add( v );      return *this; }
	VectorType &operator-=( const VectorType &v ) { this->sub( v );      return *this; }
	VectorType &operator*=( const VectorType &v ) { this->multiply( v ); return *this; }
	VectorType &operator*=( const double s )      { this->multiply( s ); return *this; }
	VectorType &operator/=( const VectorType &v ) { this->divide( v );   return *this; }
	VectorType &operator/=( const double s )      { this->divide( s );   return *this; }

	bool operator==( const VectorType &v ) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
};


#define VECTOR_TYPE_4D( Vec, T )                                                                             \
	using Vec = Vector4D<T>;                                                                                 \
	inline bool Vec##_equal( const Vec &a, const Vec &b )      { return a == b; }                            \
	inline    T Vec##_dot( const Vec &a, const Vec &b )        { return a.dot( b ); }                        \
	inline    T Vec##_length( const Vec &v )                   { return v.length(); }                        \
	inline    T Vec##_length_sqr( const Vec &v )               { return v.length_sqr(); }                    \
	inline  Vec Vec##_add( const Vec &a, const Vec &b )        { Vec r { a }; r.add( b ); return r; }        \
	inline  Vec Vec##_sub( const Vec &a, const Vec &b )        { Vec r { a }; r.sub( b ); return r; }        \
	inline  Vec Vec##_multiply( const Vec &a, const Vec &b )   { Vec r { a }; r.multiply( b ); return r; }   \
	inline  Vec Vec##_divide( const Vec &a, const Vec &b )     { Vec r { a }; r.divide( b ); return r; }     \
	inline  Vec Vec##_multiply( const Vec &v, const double s ) { Vec r { v }; r.multiply( s ); return r; }   \
	inline  Vec Vec##_divide( const Vec &v, const double s )   { Vec r { v }; r.divide( s ); return r; }     \
	inline  Vec Vec##_cross( const Vec &a, const Vec &b )      { Vec r { a }; r.cross( b ); return r; }      \
	inline  Vec Vec##_normalize( const Vec &v )                { Vec r { v }; r.normalize(); return r; }     \
	inline  Vec Vec##_project( const Vec &v, const Vec &onto ) { Vec r { v }; r.project( onto ); return r; } \
	inline  Vec Vec##_rotate_x( const Vec &v, const float angle, const Vec &origin = { } )                   \
		{ Vec r { v }; r.rotate_x( angle, origin ); return r; }                                              \
	inline  Vec Vec##_rotate_y( const Vec &v, const float angle, const Vec &origin = { } )                   \
		{ Vec r { v }; r.rotate_y( angle, origin ); return r; }                                              \
	inline  Vec Vec##_rotate_z( const Vec &v, const float angle, const Vec &origin = { } )                   \
		{ Vec r { v }; r.rotate_z( angle, origin ); return r; }                                              \
	inline  Vec Vec##_reflect( const Vec &v, const Vec &normal )                                             \
		{ Vec r { v }; r.reflect( normal ); return r; }                                                      \
	inline Vec Vec##_lerp( const Vec &a, const Vec &b, const float amount )                                  \
		{ Vec r { a }; r.lerp( b, amount ); return r; }                                                      \
	inline   T Vec##_distance( const Vec &a, const Vec &b )                                                  \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z, b.w - a.w }; return r.length(); }                         \
	inline   T Vec##_distance_sqr( const Vec &a, const Vec &b )                                              \
		{ Vec r { b.x - a.x, b.y - a.y, b.z - a.z, b.w - a.w }; return r.length_sqr(); }


VECTOR_TYPE_4D( i8v4, i8 )
VECTOR_TYPE_4D( u8v4, u8 )
VECTOR_TYPE_4D( i16v4, i16 )
VECTOR_TYPE_4D( u16v4, u16 )
VECTOR_TYPE_4D( intv4, i32 )
VECTOR_TYPE_4D( u32v4, u32 )
VECTOR_TYPE_4D( i64v4, i64 )
VECTOR_TYPE_4D( u64v4, u64 )
VECTOR_TYPE_4D( boolv4, bool )
VECTOR_TYPE_4D( floatv4, float )
VECTOR_TYPE_4D( doublev4, double )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline floatv2 cubic_bezier( floatv2 root0, floatv2 root1, floatv2 handle0, floatv2 handle1, float t )
{
	const float tSqr = t * t;
	const float tInv = 1.0f - t;
	const float tInvSqr = tInv * tInv;
	const float tCube = tSqr * t;
	const float tInvCube = tInvSqr * tInv;

	return floatv2
		{
			tInvCube * root0.x + 3.0f * tInvSqr * t * handle0.x + 3.0f * tInv * tSqr * handle1.x + tCube * root1.x,
			tInvCube * root0.y + 3.0f * tInvSqr * t * handle0.y + 3.0f * tInv * tSqr * handle1.y + tCube * root1.y,
		};
}


inline doublev2 cubic_bezier( doublev2 root0, doublev2 root1, doublev2 handle0, doublev2 handle1, double t )
{
	const double tSqr = t * t;
	const double tInv = 1.0 - t;
	const double tInvSqr = tInv * tInv;
	const double tCube = tSqr * t;
	const double tInvCube = tInvSqr * tInv;

	return doublev2
		{
			tInvCube * root0.x + 3.0 * tInvSqr * t * handle0.x + 3.0 * tInv * tSqr * handle1.x + tCube * root1.x,
			tInvCube * root0.y + 3.0 * tInvSqr * t * handle0.y + 3.0 * tInv * tSqr * handle1.y + tCube * root1.y,
		};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////