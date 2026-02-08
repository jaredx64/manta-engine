#include <manta/draw.hpp>

#include <vendor/vendor.hpp>
#include <vendor/stdarg.hpp>
#include <vendor/stdio.hpp>

#include <core/utf8.hpp>
#include <core/math.hpp>

#include <manta/gfx.hpp>
#include <manta/fonts.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const Assets::SpriteEntry &spriteEntryNull = Assets::sprite( Sprite::SPRITE_DEFAULT );
static const Assets::GlyphEntry &glyphEntryNull = Assets::glyph( spriteEntryNull.glyph );
static const GfxTexture *const textureNull = &CoreGfx::textures[spriteEntryNull.texture];

static Align halign = Align_Left;
static Align valign = Align_Top;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_set_halign( const Align align )
{
	halign = align;
}


void draw_set_valign( const Align align )
{
	valign = align;
}


Align draw_get_halign()
{
	return halign;
}


Align draw_get_valign()
{
	return valign;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_quad( float x1, float y1, float x2, float y2, Color color, float depth )
{
#if GRAPHICS_ENABLED
	Gfx::quad_batch_write( x1, y1, x2, y2, 0, 0, 0xFFFF, 0xFFFF, color, nullptr, depth );
#endif
}


void draw_quad_color( float x1, float y1, float x2, float y2,
	Color c1, Color c2, Color c3, Color c4, float depth )
{
#if GRAPHICS_ENABLED
	Gfx::quad_batch_write( x1, y1, x2, y2, 0, 0, 0xFFFF, 0xFFFF, c1, c2, c3, c4, nullptr, depth );
#endif
}


void draw_quad_uv( const float x1, const float y1, const float x2, const float y2,
	const float u1, const float v1, const float u2, const float v2, const Color color, const float depth )
{
#if GRAPHICS_ENABLED
	const u16 U1 = static_cast<u16>( u1 * 65535.0 );
	const u16 V1 = static_cast<u16>( v1 * 65535.0 );
	const u16 U2 = static_cast<u16>( u2 * 65535.0 );
	const u16 V2 = static_cast<u16>( v2 * 65535.0 );
	Gfx::quad_batch_write( x1, y1, x2, y2, U1, V1, U2, V2, color, nullptr, depth );
#endif
}


void draw_quad_uv( float x1, float y1, float x2, float y2, u16 u1, u16 v1, u16 u2, u16 v2,
	Color color, float depth )
{
#if GRAPHICS_ENABLED
	Gfx::quad_batch_write( x1, y1, x2, y2, u1, v1, u2, v2, color, nullptr, depth );
#endif
}


void draw_quad_uv( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	float u1, float v1, float u2, float v2, Color color, float depth )
{
#if GRAPHICS_ENABLED
	const u16 U1 = static_cast<u16>( u1 * 65535.0 );
	const u16 V1 = static_cast<u16>( v1 * 65535.0 );
	const u16 U2 = static_cast<u16>( u2 * 65535.0 );
	const u16 V2 = static_cast<u16>( v2 * 65535.0 );
	Gfx::quad_batch_write( x1, y1, x2, y2, x3, y3, x4, y4, U1, V1, U2, V2, color, nullptr, depth );
#endif
}


void draw_quad_uv( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	u16 u1, u16 v1, u16 u2, u16 v2, Color color, float depth )
{
#if GRAPHICS_ENABLED
	Gfx::quad_batch_write( x1, y1, x2, y2, x3, y3, x4, y4, u1, v1, u2, v2, color, nullptr, depth );
#endif
}


void draw_quad_uv_color( float x1, float y1, float x2, float y2, u16 u1, u16 v1, u16 u2, u16 v2,
	Color c1, Color c2, Color c3, Color c4, float depth )
{
#if GRAPHICS_ENABLED
	Gfx::quad_batch_write( x1, y1, x2, y2, u1, v1, u2, v2, c1, c2, c3, c4, nullptr, depth );
#endif
}


void draw_quad_uv_color( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	float u1, float v1, float u2, float v2, Color c1, Color c2, Color c3, Color c4, float depth )
{
#if GRAPHICS_ENABLED
	const u16 U1 = static_cast<u16>( u1 * 65535.0 );
	const u16 V1 = static_cast<u16>( v1 * 65535.0 );
	const u16 U2 = static_cast<u16>( u2 * 65535.0 );
	const u16 V2 = static_cast<u16>( v2 * 65535.0 );
	Gfx::quad_batch_write( x1, y1, x2, y2, x3, y3, x4, y4, U1, V1, U2, V2, c1, c2, c3, c4, nullptr, depth );
#endif
}


void draw_quad_uv_color( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	u16 u1, u16 v1, u16 u2, u16 v2, Color c1, Color c2, Color c3, Color c4, float depth )
{
#if GRAPHICS_ENABLED
	Gfx::quad_batch_write( x1, y1, x2, y2, x3, y3, x4, y4, u1, v1, u2, v2, c1, c2, c3, c4, nullptr, depth );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_sprite( Sprite sprite, u16 subimg, float x, float y, float xscale, float yscale, Color color, float depth )
{
#if GRAPHICS_ENABLED
	const Assets::SpriteEntry &spriteEntry = Assets::sprite( sprite );
	const Assets::GlyphEntry &glyphEntry =
		Assets::glyph( spriteEntry.glyph + ( subimg % Assets::sprite( sprite ).count ) );
	GfxTexture *texture = &CoreGfx::textures[spriteEntry.texture];

	float width  = spriteEntry.width * xscale;
	float height = spriteEntry.height * yscale;

	x -= spriteEntry.xorigin * xscale;
	y -= spriteEntry.yorigin * yscale;

	Gfx::quad_batch_write( x, y, x + width, y + height,
		glyphEntry.u1, glyphEntry.v1, glyphEntry.u2, glyphEntry.v2, color, texture, depth );
#endif
}


void draw_sprite_part( Sprite sprite, u16 subimg, float x, float y, float u1, float v1, float u2, float v2,
	float xscale, float yscale, Color color, float depth )
{
#if GRAPHICS_ENABLED
	const Assets::SpriteEntry &spriteEntry = Assets::sprite( sprite );
	const Assets::GlyphEntry &glyphEntry =
		Assets::glyph( spriteEntry.glyph + ( subimg % Assets::sprite( sprite ).count ) );
	const GfxTexture *const texture = &CoreGfx::textures[spriteEntry.texture];

	float width  = spriteEntry.width * xscale;
	float height = spriteEntry.height * yscale;

	x -= spriteEntry.xorigin * xscale;
	y -= spriteEntry.yorigin * yscale;

	const u16 u = glyphEntry.u2 - glyphEntry.u1;
	const u16 v = glyphEntry.v2 - glyphEntry.v1;
	Gfx::quad_batch_write( x, y, x + width, y + height,
		glyphEntry.u1 + static_cast<u16>( u1 * u ), glyphEntry.v1 + static_cast<u16>( v1 * v ),
		glyphEntry.u1 + static_cast<u16>( u2 * v ), glyphEntry.v1 + static_cast<u16>( v2 * v ),
		color, texture, depth );
#endif
}


void draw_sprite_part_quad( Sprite sprite, u16 subimg, float x1, float y1, float x2, float y2,
	float u1, float v1, float u2, float v2, Color color, float depth )
{
#if GRAPHICS_ENABLED
	const Assets::SpriteEntry &spriteEntry = Assets::sprite( sprite );
	const Assets::GlyphEntry &glyphEntry =
		Assets::glyph( spriteEntry.glyph + ( subimg % Assets::sprite( sprite ).count ) );
	const GfxTexture *const texture = &CoreGfx::textures[spriteEntry.texture];

	const u16 u = glyphEntry.u2 - glyphEntry.u1;
	const u16 v = glyphEntry.v2 - glyphEntry.v1;
	Gfx::quad_batch_write( x1, y1, x2, y2,
		glyphEntry.u1 + static_cast<u16>( u1 * u ), glyphEntry.v1 + static_cast<u16>( v1 * v ),
		glyphEntry.u1 + static_cast<u16>( u2 * v ), glyphEntry.v1 + static_cast<u16>( v2 * v ),
		color, texture, depth );
#endif
}


void draw_sprite_angle( Sprite sprite, u16 subimg, float x, float y, float angle, float xscale, float yscale,
	Color color, float depth )
{
#if GRAPHICS_ENABLED
	const Assets::SpriteEntry &spriteEntry = Assets::sprite( sprite );
	const Assets::GlyphEntry &glyphEntry =
		Assets::glyph( spriteEntry.glyph + ( subimg % Assets::sprite( sprite ).count ) );
	const GfxTexture *const texture = &CoreGfx::textures[spriteEntry.texture];

	const float width = spriteEntry.width * xscale;
	const float height = spriteEntry.height * yscale;
	const float dx = spriteEntry.xorigin * xscale;
	const float dy = spriteEntry.yorigin * yscale;

#if 0
	const float s = sinf( angle * DEG2RAD_F );
	const float c = cosf( angle * DEG2RAD_F );
#else
	float s, c;
	fast_sinf_cosf( angle * DEG2RAD_F, s, c );
#endif

	const float x1 = x - ( dx ) * c + ( dy ) * s;
	const float y1 = y - ( dx ) * s - ( dy ) * c;
	const float x2 = x - ( dx - width ) * c + ( dy ) * s;
	const float y2 = y - ( dx - width ) * s - ( dy ) * c;
	const float x3 = x - ( dx ) * c + ( dy - height ) * s;
	const float y3 = y - ( dx ) * s - ( dy - height ) * c;
	const float x4 = x - ( dx - width ) * c + ( dy - height ) * s;
	const float y4 = y - ( dx - width ) * s - ( dy - height ) * c;

	Gfx::quad_batch_write( x1, y1, x2, y2, x3, y3, x4, y4,
		glyphEntry.u1, glyphEntry.v1, glyphEntry.u2, glyphEntry.v2, color, texture, depth );
#endif
}


void draw_sprite_fast( Sprite sprite, u16 subimg, float x, float y, Color color )
{
#if GRAPHICS_ENABLED
	const Assets::SpriteEntry &spriteEntry = Assets::sprite( sprite );
	const Assets::GlyphEntry &glyphEntry = Assets::glyph( spriteEntry.glyph + subimg );
	const GfxTexture *const texture = &CoreGfx::textures[spriteEntry.texture];

	const float width = spriteEntry.width;
	const float height = spriteEntry.height;

	Gfx::quad_batch_write( x, y, x + width, y + height,
		glyphEntry.u1, glyphEntry.v1, glyphEntry.u2, glyphEntry.v2, color, texture, 0.0f );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_render_target( const GfxRenderTarget &rt, float x, float y, float xscale, float yscale,
	Color color, float depth )
{
#if GRAPHICS_ENABLED
	const float x1 = x;
	const float x2 = x + rt.width * xscale;
	const float y1 = y;
	const float y2 = y + rt.height * yscale;

	const u16 u1 = 0;
	const u16 u2 = 0xFFFF;
	const u16 v1 = 0; // Render target memory is 'upside down' relative to default UV sampling
	const u16 v2 = 0xFFFF;

	Assert( rt.textureColor.is_initialized() );
	Gfx::quad_batch_write( x1, y1, x2, y2, u1, v1, u2, v2, color, &rt.textureColor, depth );
#endif
}


void draw_render_target_depth( const GfxRenderTarget &rt, float x, float y, float xscale,  float yscale,
	Color color, float depth )
{
#if GRAPHICS_ENABLED
	const float x1 = x;
	const float x2 = x + rt.width * xscale;
	const float y1 = y;
	const float y2 = y + rt.height * yscale;

	const u16 u1 = 0;
	const u16 u2 = 0xFFFF;
	const u16 v1 = 0xFFFF; // NOTE: Render target memory is 'upside down' relative to default UV sampling
	const u16 v2 = 0;

	Assert( rt.textureDepth.is_initialized() );
	Gfx::quad_batch_write( x1, y1, x2, y2, u1, v1, u2, v2, color, &rt.textureDepth, depth );
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_rectangle( float x1, float y1, float x2, float y2, Color color, bool outline, float depth )
{
#if GRAPHICS_ENABLED
	if( outline )
	{
		const float_v2 q1[] = { { x1, y1 }, { x1 + 1.0f, y2 } }; // Left
		const float_v2 q2[] = { { x2 - 1.0f, y1 }, { x2, y2 } }; // Right
		const float_v2 q3[] = { { x1 + 1.0f, y1 }, { x2 - 1.0f, y1 + 1.0f } }; // Top
		const float_v2 q4[] = { { x1, y2 - 1.0f }, { x2, y2 } }; // Bottom

		draw_rectangle( q1[0].x, q1[0].y, q1[1].x, q1[1].y, color, false, depth );
		draw_rectangle( q2[0].x, q2[0].y, q2[1].x, q2[1].y, color, false, depth );
		draw_rectangle( q3[0].x, q3[0].y, q3[1].x, q3[1].y, color, false, depth );
		draw_rectangle( q4[0].x, q4[0].y, q4[1].x, q4[1].y, color, false, depth );
	}
	else
	{
		const Assets::GlyphEntry &g = glyphEntryNull;
		Gfx::quad_batch_write( x1, y1, x2, y2, g.u1, g.v1, g.u2, g.v2, color, textureNull, depth );
	}
#endif
}


void draw_rectangle_angle( float x1, float y1, float x2, float y2, float angle,
	Color color, bool outline, float depth )
{
#if GRAPHICS_ENABLED
	if( outline )
	{
		if( angle != 0.0f )
		{
			const float_v2 x2y1 = float_v2( x2, y1 ).rotate( angle, { x1, y1 } );
			const float_v2 x1y2 = float_v2( x1, y2 ).rotate( angle, { x1, y1 } );

			const float_v2 q1[] = { { x1, y1 }, { x1 + 1.0f, y2 } }; // Left
			const float_v2 q2[] = { { x2y1.x, x2y1.y }, { x2y1.x - 1.0f, x2y1.y + ( y2 - y1 ) } }; // Right
			const float_v2 q3[] = { { x1, y1 }, { x2, y1 + 1.0f } }; // Top
			const float_v2 q4[] = { { x1y2.x, x1y2.y }, { x1y2.x + ( x2 - x1 ), x1y2.y - 1.0f } }; // Bottom

			draw_rectangle_angle( q1[0].x, q1[0].y, q1[1].x, q1[1].y, angle, color, false, depth );
			draw_rectangle_angle( q2[0].x, q2[0].y, q2[1].x, q2[1].y, angle, color, false, depth );
			draw_rectangle_angle( q3[0].x, q3[0].y, q3[1].x, q3[1].y, angle, color, false, depth );
			draw_rectangle_angle( q4[0].x, q4[0].y, q4[1].x, q4[1].y, angle, color, false, depth );
		}
		else
		{
			const float_v2 q1[] = { { x1, y1 }, { x1 + 1.0f, y2 } }; // Left
			const float_v2 q2[] = { { x2 - 1.0f, y1 }, { x2, y2 } }; // Right
			const float_v2 q3[] = { { x1 + 1.0f, y1 }, { x2 - 1.0f, y1 + 1.0f } }; // Top
			const float_v2 q4[] = { { x1, y2 - 1.0f }, { x2, y2 } }; // Bottom

			draw_rectangle( q1[0].x, q1[0].y, q1[1].x, q1[1].y, color, false, depth );
			draw_rectangle( q2[0].x, q2[0].y, q2[1].x, q2[1].y, color, false, depth );
			draw_rectangle( q3[0].x, q3[0].y, q3[1].x, q3[1].y, color, false, depth );
			draw_rectangle( q4[0].x, q4[0].y, q4[1].x, q4[1].y, color, false, depth );
		}
	}
	else
	{
		const Assets::GlyphEntry &g = glyphEntryNull;

		if( angle != 0.0f )
		{
			const float_v2 x2y1 = float_v2( x2, y1 ).rotate( angle, { x1, y1 } );
			const float_v2 x1y2 = float_v2( x1, y2 ).rotate( angle, { x1, y1 } );
			const float_v2 x2y2 = float_v2( x2, y2 ).rotate( angle, { x1, y1 } );
			Gfx::quad_batch_write( x1, y1, x2y1.x, x2y1.y, x1y2.x, x1y2.y, x2y2.x, x2y2.y,
			                       g.u1, g.v1, g.u2, g.v2, color, textureNull, depth );
		}
		else
		{
			Gfx::quad_batch_write( x1, y1, x2, y2, g.u1, g.v1, g.u2, g.v2, color, textureNull, depth );
		}
	}
#endif
}


void draw_rectangle_gradient( float x1, float y1, float x2, float y2, Color c1, Color c2, Color c3, Color c4,
	bool outline, float depth )
{
#if GRAPHICS_ENABLED
	if( outline )
	{
		const float_v2 q1[] = { { x1, y1 }, { x1 + 1.0f, y2 } }; // Left
		const float_v2 q2[] = { { x2 - 1.0f, y1 }, { x2, y2 } }; // Right
		const float_v2 q3[] = { { x1 + 1.0f, y1 }, { x2 - 1.0f, y1 + 1.0f } }; // Top
		const float_v2 q4[] = { { x1, y2 - 1.0f }, { x2, y2 } }; // Bottom

		draw_rectangle_gradient( q1[0].x, q1[0].y, q1[1].x, q1[1].y, c1, c1, c3, c3, false, depth );
		draw_rectangle_gradient( q2[0].x, q2[0].y, q2[1].x, q2[1].y, c2, c2, c4, c4, false, depth );
		draw_rectangle_gradient( q3[0].x, q3[0].y, q3[1].x, q3[1].y, c1, c2, c1, c2, false, depth );
		draw_rectangle_gradient( q4[0].x, q4[0].y, q4[1].x, q4[1].y, c3, c4, c3, c4, false, depth );
	}
	else
	{
		const Assets::GlyphEntry &g = glyphEntryNull;
		Gfx::quad_batch_write( x1, y1, x2, y2, g.u1, g.v1, g.u2, g.v2, c1, c2, c3, c4, textureNull, depth );
	}
#endif
}


void draw_rectangle_gradient_angle( float x1, float y1, float x2, float y2, float angle,
	Color c1, Color c2, Color c3, Color c4, bool outline, float depth )
{
#if GRAPHICS_ENABLED
	if( outline )
	{
		if( angle != 0.0f )
		{
			const float_v2 x2y1 = float_v2( x2, y1 ).rotate( angle, { x1, y1 } );
			const float_v2 x1y2 = float_v2( x1, y2 ).rotate( angle, { x1, y1 } );

			const float_v2 q1[] = { { x1, y1 }, { x1 + 1.0f, y2 } }; // Left
			const float_v2 q2[] = { { x2y1.x, x2y1.y }, { x2y1.x - 1.0f, x2y1.y + ( y2 - y1 ) } }; // Reft
			const float_v2 q3[] = { { x1, y1 }, { x2, y1 + 1.0f } }; // Top
			const float_v2 q4[] = { { x1y2.x, x1y2.y }, { x1y2.x + ( x2 - x1 ), x1y2.y - 1.0f } }; // Bottom

			draw_rectangle_gradient_angle( q1[0].x, q1[0].y, q1[1].x, q1[1].y, angle, c1, c1, c3, c3, false, depth );
			draw_rectangle_gradient_angle( q2[0].x, q2[0].y, q2[1].x, q2[1].y, angle, c2, c2, c4, c4, false, depth );
			draw_rectangle_gradient_angle( q3[0].x, q3[0].y, q3[1].x, q3[1].y, angle, c1, c2, c1, c2, false, depth );
			draw_rectangle_gradient_angle( q4[0].x, q4[0].y, q4[1].x, q4[1].y, angle, c3, c4, c3, c4, false, depth );
		}
		else
		{
			const float_v2 q1[] = { { x1, y1 }, { x1 + 1.0f, y2 } }; // Left
			const float_v2 q2[] = { { x2 - 1.0f, y1 }, { x2, y2 } }; // Right
			const float_v2 q3[] = { { x1 + 1.0f, y1 }, { x2 - 1.0f, y1 + 1.0f } }; // Top
			const float_v2 q4[] = { { x1, y2 - 1.0f }, { x2, y2 } }; // Bottom

			draw_rectangle_gradient( q1[0].x, q1[0].y, q1[1].x, q1[1].y, c1, c1, c3, c3, false, depth );
			draw_rectangle_gradient( q2[0].x, q2[0].y, q2[1].x, q2[1].y, c2, c2, c4, c4, false, depth );
			draw_rectangle_gradient( q3[0].x, q3[0].y, q3[1].x, q3[1].y, c1, c2, c1, c2, false, depth );
			draw_rectangle_gradient( q4[0].x, q4[0].y, q4[1].x, q4[1].y, c3, c4, c3, c4, false, depth );
		}
	}
	else
	{
		const Assets::GlyphEntry &g = glyphEntryNull;

		if( angle != 0.0f )
		{
			const float_v2 x2y1 = float_v2( x2, y1 ).rotate( angle, { x1, y1 } );
			const float_v2 x1y2 = float_v2( x1, y2 ).rotate( angle, { x1, y1 } );
			const float_v2 x2y2 = float_v2( x2, y2 ).rotate( angle, { x1, y1 } );

			Gfx::quad_batch_write( x1, y1, x2y1.x, x2y1.y, x1y2.x, x1y2.y, x2y2.x, x2y2.y,
				g.u1, g.v1, g.u2, g.v2, c1, c2, c3, c4, textureNull, depth );
		}
		else
		{
			Gfx::quad_batch_write( x1, y1, x2, y2, g.u1, g.v1, g.u2, g.v2, c1, c2, c3, c4, textureNull, depth );
		}
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_line( float x1, float y1, float x2, float y2, Color color, float thickness, float depth )
{
#if GRAPHICS_ENABLED
	if ( x1 == x2 )
	// X-axis aligned line
	{
		draw_rectangle( x1, y1, x1 + thickness, y2, color, false, depth );
	}
	else if ( y1 == y2 )
	// Y-axis aligned line
	{
		draw_rectangle( x1, y1, x2, y1 + thickness, color, false, depth );
	}
	else
	// Rotated line
	{
		const float angle = atanf( ( y2 - y1 ) / ( x2 - x1 ) ) + ( x2 < x1 ? PI : 0.0f );
		const float length = float_v2_distance( { x1, y1 }, { x2, y2 } );
		draw_rectangle_angle( x1, y1, x1 + length, y1 + thickness, angle * RAD2DEG_F, color, false, depth );
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_circle_gradient( float x, float y, float radius, Color c1, Color c2, u32 resolution, float depth )
{
	const float increment = 2.0f * PI / resolution;

	for( u32 i = 0; i < resolution; i++ )
	{
		const float angle1 = increment * ( i + 0.0f );
		const float angle2 = increment * ( i + 0.5f );
		const float angle3 = increment * ( i + 1.0f );

		float x1 = x;
		float y1 = y;
		float x2 = x + radius * cos( angle1 );
		float y2 = y + radius * sin( angle1 );
		float x3 = x + radius * cos( angle3 );
		float y3 = y + radius * sin( angle3 );
		float x4 = x + radius * cos( angle2 );
		float y4 = y + radius * sin( angle2 );

		const Assets::GlyphEntry &g = glyphEntryNull;

		Gfx::quad_batch_write( x1, y1, x2, y2, x3, y3, x4, y4, g.u1, g.v1, g.u2, g.v2,
			c1, c2, c2, c2, textureNull, depth );
	}
}


void draw_circle_outline_gradient( float x, float y, float radius, float thickness, Color c1, Color c2,
	u32 resolution, float depth )
{
	resolution <<= 1;
	const float increment = 2.0f * PI / resolution;
	const float radiusOuter = ( radius + thickness * 0.5f );
	const float radiusInner = ( radius - thickness * 0.5f );
	const Assets::GlyphEntry &g = glyphEntryNull;

	for( u32 i = 0; i < resolution; i++ )
	{
        const float angle1 = -increment * ( i + 0.0f );
		constexpr float epsilon = 0.001f; // HACK: Prevents gaps between quads due to not using proper triangle strip
        const float angle2 = -increment * ( i + 1.0f + epsilon );

        const float outerX1 = x + radiusOuter * cos( angle1 );
        const float outerY1 = y + radiusOuter * sin( angle1 );
        const float outerX2 = x + radiusOuter * cos( angle2 );
        const float outerY2 = y + radiusOuter * sin( angle2 );

        const float innerX1 = x + radiusInner * cos( angle1 );
        const float innerY1 = y + radiusInner * sin( angle1 );
        const float innerX2 = x + radiusInner * cos( angle2 );
        const float innerY2 = y + radiusInner * sin( angle2 );

		Gfx::quad_batch_write( innerX1, innerY1, innerX2, innerY2, outerX1, outerY1, outerX2, outerY2,
			g.u1, g.v1, g.u2, g.v2, c1, c1, c2, c2, textureNull, depth );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void draw_text( Font font, u16 size, float x, float y, Color color, const char *string )
{
#if GRAPHICS_ENABLED
	Assert( font < CoreAssets::fontCount );
	Assert( size > 0 );

	int offsetX = 0;
	int offsetY = 0;

	u32 state = UTF8_ACCEPT;
	u32 codepoint;
	char c;

	while( ( c = *string++ ) != '\0' )
	{
		if( utf8_decode( &state, &codepoint, c ) != UTF8_ACCEPT ) { continue; }

		// Newline char
		if( UNLIKELY( codepoint == '\n' ) )
		{
			offsetX = 0;
			offsetY += size;
			continue;
		}

		// Retrieve FontGlyphInfo
		CoreFonts::FontGlyphInfo &glyphInfo = CoreFonts::get( CoreFonts::FontGlyphKey { codepoint, font.ttf, size } );

		// Draw Quad
		if( glyphInfo.width != 0 && glyphInfo.height != 0 )
		{
			const float glyphX1 = x + offsetX + glyphInfo.xshift;
			const float glyphY1 = y + offsetY + glyphInfo.yshift;
			const float glyphX2 = x + offsetX + glyphInfo.xshift + glyphInfo.width;
			const float glyphY2 = y + offsetY + glyphInfo.yshift + glyphInfo.height;

			constexpr u16 uvScale = ( 1 << 16 ) / CoreFonts::FONTS_TEXTURE_SIZE;
			const u16 u1 = ( glyphInfo.u ) * uvScale;
			const u16 v1 = ( glyphInfo.v ) * uvScale;
			const u16 u2 = ( glyphInfo.u + glyphInfo.width ) * uvScale;
			const u16 v2 = ( glyphInfo.v + glyphInfo.height ) * uvScale;

			// TODO: Implement this properly...
			if( UNLIKELY( Gfx::quad_batch_can_break() ) ||
			    UNLIKELY( CoreGfx::state.boundTexture[0] != CoreFonts::glyphAtlasTexture.resource ) )
			{
				CoreFonts::update();
			}

			Gfx::quad_batch_write( glyphX1, glyphY1, glyphX2, glyphY2,
				u1, v1, u2, v2, color, &CoreFonts::glyphAtlasTexture, 0.0f );
		}

		// Advance Character
		offsetX += glyphInfo.advance;
	}
#endif
}


void draw_text_f( Font font, u16 size, float x, float y, Color color, const char *format, ... )
{
#if GRAPHICS_ENABLED
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	draw_text( font, size, x, y, color, buffer );
#endif
}


int_v2 text_dimensions( Font font, u16 size, const char *string )
{
	Assert( font.id < CoreAssets::fontCount );
	Assert( size > 0 );

	int offsetX = 0;
	int offsetY = 0;
	int_v2 result = 0;

	u32 state = UTF8_ACCEPT;
	u32 codepoint;
	char c;

	while( ( c = *string++ ) != '\0' )
	{
		if( utf8_decode( &state, &codepoint, c ) != UTF8_ACCEPT ) { continue; }

		// Newline char
		if( UNLIKELY( codepoint == '\n' ) )
		{
			offsetX = 0;
			result.y = max( result.y, offsetY + size );
			offsetY += size;
			continue;
		}

		// Retrieve FontGlyphInfo & advance
		CoreFonts::FontGlyphInfo &glyphInfo = CoreFonts::get( CoreFonts::FontGlyphKey { codepoint, font.ttf, size } );
		result.x = max( result.x, offsetX + static_cast<int>( glyphInfo.advance ) );
		result.y = max( result.y, offsetY + glyphInfo.height );
		offsetX += glyphInfo.advance;
	}

	return result;
}


int_v2 text_dimensions_f( Font font, u16 size, const char *format, ... )
{
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	return text_dimensions( font, size, buffer );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////