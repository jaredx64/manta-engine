#include <scene.hpp>

#include <manta/window.hpp>
#include <manta/gfx.hpp>
#include <manta/draw.hpp>
#include <manta/input.hpp>
#include <manta/time.hpp>
#include <manta/3d.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <universe.hpp>
#include <earth.hpp>
#include <atmosphere.hpp>
#include <view.hpp>
#include <labels.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static GfxRenderTargetDescription rtSceneMSAADescription;
static GfxRenderTargetDescription rtSceneCompositeDescription;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Scene
{
	GfxRenderTarget rtSceneMSAA;
	GfxRenderTarget rtSceneComposite;
}

static float fade = 1.0;
static bool labels = true;
static bool debug = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void validate_render_target( GfxRenderTarget &target )
{
	if( target.width != Window::width || target.height != Window::height )
	{
		target.resize( Window::width, Window::height );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void scene_init()
{
	// We we render the Earth with MSAA on to avoid aliased edges (probably not necesasry, but looks good!)
	rtSceneMSAADescription.colorFormat = GfxColorFormat_R8G8B8A8_FLOAT;
	rtSceneMSAADescription.depthFormat = GfxDepthFormat_R32_FLOAT;
	rtSceneMSAADescription.sampleCount = 4;
	Scene::rtSceneMSAA.init( Window::width, Window::height, rtSceneMSAADescription );

	// The composite RT is contains the entire scene contents (minus UI) and used for the final color-graded render
	rtSceneCompositeDescription.colorFormat = GfxColorFormat_R8G8B8A8_FLOAT;
	rtSceneCompositeDescription.depthFormat = GfxDepthFormat_R32_FLOAT;
	rtSceneMSAADescription.sampleCount = 1;
	Scene::rtSceneComposite.init( Window::width, Window::height, rtSceneMSAADescription );
}


void scene_free()
{

}


void scene_draw_3d( const Delta delta )
{
	GfxRenderPass passMSAA;
	validate_render_target( Scene::rtSceneMSAA );
	passMSAA.set_target( 0, Scene::rtSceneMSAA );

	GfxRenderPass passComposite;
	validate_render_target( Scene::rtSceneComposite );
	passComposite.set_target( 0, Scene::rtSceneComposite );

	// Earth
	passMSAA.set_name( "Earth (MSAA)" );
	Gfx::render_pass_begin( passMSAA );
	{
		Gfx::clear_color( Color { 0, 0, 0, 0 } );
		Gfx::clear_depth();

		earth_draw( delta );
	}
	Gfx::render_pass_end( passMSAA );

	// Scene
	passComposite.set_name( "Scene" );
	Gfx::render_pass_begin( passComposite );
	{
		Gfx::clear_color( Color { 0, 0, 0, 0 } );

		// Universe
		universe_draw( delta );

		// Earth (Composite)
		GfxRenderCommand cmd;
		cmd.set_shader( Shader::sh_composite_quad );
		cmd.depth_set_function( GfxDepthFunction_NONE );
		cmd.depth_set_write( GfxDepthWrite_NONE );
		cmd.work( GfxWork
			{
				Gfx::bind_texture( 0, Scene::rtSceneMSAA.textureColor );
				Gfx::draw_vertices( 4, GfxPrimitiveType_TriangleStrip );
			} );
		Gfx::render_command_execute( cmd );

		// Atmosphere
		atmosphere_draw( delta );
	}
	Gfx::render_pass_end( passComposite );

	// Composite
	GfxRenderCommand cmd;
	cmd.set_shader( Shader::sh_composite_quad );
	cmd.work( GfxWork
		{
			Gfx::bind_texture( 0, Scene::rtSceneComposite.textureColor );
			Gfx::draw_vertices( 4, GfxPrimitiveType_TriangleStrip );
		} );
	Gfx::render_command_execute( cmd );
}


void scene_draw_ui( const Delta delta )
{
	Gfx::set_matrix_mvp_2d_orthographic( 0.0, 0.0, 1.0, 0.0, Window::width, Window::height );
	Gfx::sampler_set_filtering_mode( GfxSamplerFilteringMode_NEAREST );

	// Labels
	if( Keyboard::check_pressed( vk_l ) ) { labels = !labels; }
	if( labels ) { labels_draw_ui( delta ); }

	// Scene Fade-in
	if( fade > 0.0f )
	{
		draw_rectangle( 0, 0, Window::width, Window::height,
			Color { 0, 0, 0, static_cast<u8>( fade * 255 ) } );
		fade -= delta;
	}

	// Info
	u16 size = 14;
	const char *line;
	int_v2 lineWH;

	line = "Movement: W, A, S, D";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 0, c_ltgray, line );

	line = "Zoom: Q, E";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 1, c_ltgray, line );

	line = "Orbit Mode: SPACE";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 2, c_ltgray, line );

	line = "Toggle Clouds: C";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 3, c_ltgray, line );

	line = "Show Labels: L";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 4, c_ltgray, line );

	line = "Show Debug: F1";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 5, c_ltgray, line );

	line = "Jump To: Left Mouse";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 6, c_ltgray, line );

	line = "Change Sun: Right Mouse";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 7, c_ltgray, line );

	line = "Console: ~";
	lineWH = text_dimensions( fnt_iosevka, size, line );
	draw_text( fnt_iosevka, size, Window::width - 8.0f - lineWH.x, 8.0f + 20.0 * 8, c_ltgray, line );

	draw_text_f( fnt_iosevka, 16, 8.0f, Window::height - 20, c_gray, "MANTA ENGINE DEMO PROJECT 2025" );
	draw_text_f( fnt_iosevka, 16, 8.0f, 8.0f, c_white, "FPS: %d", Frame::fps );

	// Debug
	if( Keyboard::check_pressed( vk_f1 ) ) { debug = !debug; }
	if( debug )
	{
		PROFILE_GFX( debug_overlay_gfx( 8.0f, 40.0f ) );
		draw_axis_3d( Window::width - 200, Window::height - 200, 192, 192, View::forward, View::up );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////