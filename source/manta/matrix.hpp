#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct float_m44;
struct double_m44;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool float_m44_equal( const float_m44 &a, const float_m44 &b );

extern float_m44 float_m44_transpose( const float_m44 &m );

extern float_m44 float_m44_inverse( const float_m44 &m );

extern float_m44 float_m44_inverse_row_major( const float_m44 &m );

extern float_m44 float_m44_multiply( const float_m44 &a, const float_m44 &b );

extern float_m44 float_m44_multiply_scalar( const float_m44 &a, const float scalar );

extern float_m44 float_m44_add( const float_m44 &a, const float_m44 &b );

extern float_m44 float_m44_sub( const float_m44 &a, const float_m44 &b );

extern float_m44 float_m44_build_zeros();

extern float_m44 float_m44_build_identity();

extern float_m44 float_m44_build_scaling( const float xscale, const float yscale, const float zscale );

extern float_m44 float_m44_build_translation( const float xtrans, const float ytrans, const float ztrans );

extern float_m44 float_m44_build_rotation_x( const float rad );

extern float_m44 float_m44_build_rotation_y( const float rad );

extern float_m44 float_m44_build_rotation_z( const float rad );

extern float_m44 float_m44_build_orthographic( float left, float right, float top, float bottom, float znear, float zfar );

extern float_m44 float_m44_build_perspective( const float fov, const float aspect, const float znear, const float zfar );

extern float_m44 float_m44_build_lookat( const float x, const float y, const float z,
	const float xto, const float yto, const float zto,
	const float xup, const float yup, const float zup );

extern float_m44 float_m44_build_ndc( const float width, const float height );

extern float_m44 float_m44_from_double_m44( const double_m44 &m );

struct float_m44
{
	float data[16];

	float_m44();
	float_m44( const float_m44 &m );
	float_m44( float_m44 &&m );
	float_m44( const double_m44 &m );
	float_m44( double_m44 &&m );
	float_m44 &operator=( const float_m44 &m );
	float_m44 &operator=( float_m44 &&m );

	explicit operator double_m44() const;

	float &operator[]( const int index ) { return data[index]; }
	float operator[]( const int index ) const { return data[index]; }
	bool operator==( const float_m44 &m ) { return float_m44_equal( *this, m ); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool double_m44_equal( const double_m44 &a, const double_m44 &b );

extern double_m44 double_m44_transpose( const double_m44 &m );

extern double_m44 double_m44_inverse( const double_m44 &m );

extern double_m44 double_m44_multiply( const double_m44 &a, const double_m44 &b );

extern double_m44 double_m44_multiply_scalar( const double_m44 &a, const double scalar );

extern double_m44 double_m44_add( const double_m44 &a, const double_m44 &b );

extern double_m44 double_m44_sub( const double_m44 &a, const double_m44 &b );

extern double_m44 double_m44_build_zeros();

extern double_m44 double_m44_build_identity();

extern double_m44 double_m44_build_scaling( const double xscale, const double yscale, const double zscale );

extern double_m44 double_m44_build_translation( const double xtrans, const double ytrans, const double ztrans );

extern double_m44 double_m44_build_rotation_x( const double rad );

extern double_m44 double_m44_build_rotation_y( const double rad );

extern double_m44 double_m44_build_rotation_z( const double rad );

extern double_m44 double_m44_build_orthographic( double left, double right, double top, double bottom,
	double znear, double zfar );

extern double_m44 double_m44_build_perspective( const double fov, const double aspect,
	const double znear, const double zfar );

extern double_m44 double_m44_build_lookat( const double x, const double y, const double z,
	const double xto, const double yto, const double zto,
	const double xup, const double yup, const double zup );

extern double_m44 double_m44_build_ndc( const double width, const double height );

extern double_m44 double_m44_from_float_m44( const double_m44 &m );

struct double_m44
{
	double data[16];

	double_m44();
	double_m44( const float_m44 &m );
	double_m44( float_m44 &&m );
	double_m44( const double_m44 &m );
	double_m44( double_m44 &&m );
	double_m44 &operator=( const double_m44 &m );
	double_m44 &operator=( double_m44 &&m );

	explicit operator float_m44() const;

	double &operator[]( const int index ) { return data[index]; }
	double operator[]( const int index ) const { return data[index]; }
	bool operator==( const double_m44 &m ) { return double_m44_equal( *this, m ); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////