#pragma once

#include <core/types.hpp>
#include <core/math.hpp>
#include <core/color.hpp>

#include <manta/assets.hpp>
#include <manta/fonts.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( Align, u8 )
{
	Align_Left = 0,
	Align_Top = 0,
	Align_Center = 1,
	Align_Middle = 1,
	Align_Right = 2,
	Align_Bottom = 2,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_set_halign( const Align align );
extern void draw_set_valign( const Align align );

extern Align draw_get_halign();
extern Align draw_get_valign();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_quad( float x1, float y1, float x2, float y2, Color color = c_white, float depth = 0.0f );


extern void draw_quad_color( float x1, float y1, float x2, float y2,
	Color c1, Color c2, Color c3, Color c4, float depth = 0.0f );


extern void draw_quad_uv( float x1, float y1, float  x2, float y2, u16 u1, u16 v1, u16 u2, u16 v2,
	Color color = c_white, float depth = 0.0f );


extern void draw_quad_uv( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	float u1, float v1, float u2, float v2, Color color = c_white, float depth = 0.0f );


extern void draw_quad_uv( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float  y4,
	u16 u1, u16 v1, u16 u2, u16 v2, Color color = c_white, float depth = 0.0f );


extern void draw_quad_uv_color( float x1, float y1, float x2, float y2, u16 u1, u16 v1, u16 u2, u16 v2,
	Color c1, Color c2, Color c3, Color c4, float depth = 0.0f );


extern void draw_quad_uv_color( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	float u1, float v1, float u2, float v2, Color c1, Color c2, Color c3, Color c4, float depth = 0.0f );


extern void draw_quad_uv_color( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	u16 u1, u16 v1, u16 u2, u16 v2, Color c1, Color c2, Color c3, Color c4, float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline u16 sprite_subimg_mod( Sprite sprite, u16 subimg )
{
	Assert( sprite < CoreAssets::spriteCount );
	return subimg % Assets::sprite( sprite ).count;
}


extern void draw_sprite( Sprite sprite, u16 subimg, float x, float y, float xscale = 1.0f, float yscale = 1.0f,
	Color color = c_white, float depth = 0.0f );


extern void draw_sprite_part( Sprite sprite, u16 subimg, float x, float y, float u1, float v1, float u2, float v2,
	float xscale = 1.0f, float yscale = 1.0f, Color color = c_white, float depth = 0.0f );


extern void draw_sprite_part_quad( Sprite sprite, u16 subimg, float x1, float y1, float x2, float y2,
	float u1, float v1, float u2, float v2, Color color = c_white, float depth = 0.0f );


extern void draw_sprite_angle( Sprite sprite, u16 subimg, float x, float y, float angle,
	float xscale = 1.0f, float yscale = 1.0f, Color color = c_white, float depth = 0.0f );


extern void draw_sprite_fast( Sprite sprite, u16 subimg, float x, float y, const Color color = c_white );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_render_target( const class GfxRenderTarget &renderTarget, float x, float y,
	float xscale = 1.0f, float yscale = 1.0f, Color color = c_white, float depth = 0.0f );


extern void draw_render_target_depth( const class GfxRenderTarget &renderTarget, float x, float y,
	float xscale = 1.0f, float yscale = 1.0f, Color color = c_white, float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_rectangle( float x1, float y1, float x2, float y2,
	Color color = c_white, bool outline = false, float depth = 0.0f );


extern void draw_rectangle_angle( float x1, float y1, float x2, float y2, float angle,
	Color color = c_white, bool outline = false, float depth = 0.0f );


extern void draw_rectangle_gradient( float x1, float y1, float x2, float y2, Color c1, Color c2, Color c3, Color c4,
	bool outline = false, float depth = 0.0f );


extern void draw_rectangle_gradient_angle( float x1, float y1, float x2, float y2,
	float angle, Color c1, Color c2, Color c3, Color c4, bool outline = false, float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_line( float x1, float y1, float x2, float y2,
	Color color = c_white, float thickness = 1.0f, float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_circle_gradient( float x, float y, float radius, Color c1 = c_white, Color c2 = c_white,
	u32 resolution = 20, float depth = 0.0f );


inline void draw_circle( float x, float y, float radius, Color color = c_white,
	u32 resolution = 20, float depth = 0.0f )
{
	draw_circle_gradient( x, y, radius, color, color, resolution, depth );
}


extern void draw_circle_outline_gradient( float x, float y, float radius, float thickness = 1.0f,
	Color c1 = c_white,  Color c2 = c_white,
	u32 resolution = 20, float depth = 0.0f );


inline void draw_circle_outline( float x, float y, float radius, float thickness = 1.0f,
	Color color = c_white, u32 resolution = 20, float depth = 0.0f )
{
	draw_circle_outline_gradient( x, y, radius, thickness, color, color, resolution, depth );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_text( Font font, u16 size, float x, float y, Color color, const char *string );
extern void draw_text_f( Font font, u16 size, float x, float y, Color color, const char *format, ... );


extern int_v2 text_dimensions( Font font, u16 size, const char *string );
extern int_v2 text_dimensions_f( Font font, u16 size, const char *format, ... );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////