#pragma once

#include <core/types.hpp>

#include <manta/assets.hpp>
#include <manta/color.hpp>
#include <manta/fonts.hpp>
#include <manta/math.hpp>

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

void draw_quad( const float x1, const float y1, const float x2, const float y2,
                const Color color = c_white, const float depth = 0.0f );

void draw_quad_color( const float x1, const float y1, const float x2, const float y2,
                      const Color c1, const Color c2, const Color c3, const Color c4, const float depth = 0.0f );

/*
void draw_quad_uv( const float x1, const float y1, const float x2, const float y2,
                   const float u1, const float v1, const float u2, const float v2,
                   const Color color = c_white, const float depth = 0.0f );*/


void draw_quad_uv( const float x1, const float y1, const float  x2, const float y2,
                   const u16 u1, const u16 v1, const u16 u2, const u16 v2,
                   const Color color = c_white, const float depth = 0.0f );


void draw_quad_uv( const float x1, const float y1, const float x2, const float y2,
                   const float x3, const float y3, const float x4, const float y4,
                   const float u1, const float v1, const float u2, const float v2,
                   const Color color = c_white, const float depth = 0.0f );


void draw_quad_uv( const float x1, const float y1, const float x2, const float y2,
                   const float x3, const float y3, const float x4, const float  y4,
                   const u16 u1, const u16 v1, const u16 u2, const u16 v2,
                   const Color color = c_white, const float depth = 0.0f );


/*void draw_quad_uv_color( const float x1, const float y1, const float x2, const float y2,
                         const float u1, const float v1, const float u2, const float v2,
                         const Color c1, const Color c2, const Color c3, const Color c4, const float depth = 0.0f );
*/

void draw_quad_uv_color( const float x1, const float y1, const float x2, const float y2,
                         const u16 u1, const u16 v1, const u16 u2, const u16 v2,
                         const Color c1, const Color c2, const Color c3, const Color c4, const float depth = 0.0f );


void draw_quad_uv_color( const float x1, const float y1, const float x2, const float y2,
                         const float x3, const float y3, const float x4, const float y4,
                         const float u1, const float v1, const float u2, const float v2,
                         const Color c1, const Color c2, const Color c3, const Color c4, const float depth = 0.0f );


void draw_quad_uv_color( const float x1, const float y1, const float x2, const float y2,
                         const float x3, const float y3, const float x4, const float y4,
                         const u16 u1, const u16 v1, const u16 u2, const u16 v2,
                         const Color c1, const Color c2, const Color c3, const Color c4, const float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline u16 sprite_subimg_mod( const u32 sprite, const u16 subimg )
{
	Assert( sprite < Assets::spritesCount );
	return subimg % Assets::sprites[sprite].count;
}

void draw_sprite( const u32 sprite, const u16 index, float x, float y,
                  const float xscale = 1.0f, const float yscale = 1.0f,
                  const Color color = c_white, const float depth = 0.0f );


void draw_sprite_part( const u32 sprite, const u16 index, float x, float y,
                       const float u1, const float v1, const float u2, const float v2,
                       const float xscale = 1.0f, const float yscale = 1.0f,
                       const Color color = c_white, const float depth = 0.0f );


void draw_sprite_angle( const u32 sprite, const u16 index, float x, float y, const float angle,
                        const float xscale = 1.0f, const float yscale = 1.0f,
                        const Color color = c_white, const float depth = 0.0f );


void draw_sprite_fast( const u32 sprite, const u16 index, float x, float y, const Color color = c_white );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_render_target_2d( const struct GfxRenderTarget2D &renderTarget, float x, float y,
                                   const float xscale = 1.0f, const float yscale = 1.0f,
                                   const Color color = c_white, const float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_rectangle( const float x1, const float y1, const float x2, const float y2,
                     const Color color = c_white, const bool outline = false, const float depth = 0.0f );


void draw_rectangle_angle( const float x1, const float y1, const float x2, const float y2, const float angle,
                           const Color color = c_white, const bool outline = false, const float depth = 0.0f );


void draw_rectangle_gradient( const float x1, const float y1, const float x2, const float y2,
                              const Color c1, const Color c2, const Color c3, const Color c4,
                              const bool outline = false, const float depth = 0.0f );


void draw_rectangle_gradient_angle( const float x1, const float y1, const float x2, const float y2, const float angle,
                                    const Color c1, const Color c2, const Color c3, const Color c4,
                                    const bool outline = false, const float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_line( const float x1, const float y1, const float x2, const float y2,
                const Color color = c_white, const float thickness = 1.0f, const float depth = 0.0f );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void draw_circle_gradient( float x, float y, float radius, Color c1 = c_white, Color c2 = c_white,
                                  u32 resolution = 20, const float depth = 0.0f );

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

extern void draw_text( const Font font, const u16 size, const float x, const float y,
                       Color color, const char *string );
extern void draw_text_f( const Font font, const u16 size, const float x, const float y,
                         Color color, const char *format, ... );

extern intv2 text_dimensions( const Font font, const u16 size, const char *string );
extern intv2 text_dimensions_f( const Font font, const u16 size, const char *format, ... );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////