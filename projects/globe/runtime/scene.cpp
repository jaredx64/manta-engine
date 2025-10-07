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
	GfxRenderTarget2D rtSceneMSAA;
	GfxRenderTarget2D rtSceneComposite;
}

static float fade = 1.0;
static bool labels = true;
static bool debug = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void validate_render_target( GfxRenderTarget2D &target )
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
	validate_render_target( Scene::rtSceneMSAA );
	validate_render_target( Scene::rtSceneComposite );

	// Earth
	Scene::rtSceneMSAA.bind();
	Gfx::set_depth_test_mode( GfxDepthTestMode_LESS );
	Gfx::clear_color( Color { 0, 0, 0, 0 } );
	Gfx::clear_depth();
	{
		// Earth (MSAA)
		earth_draw( delta );
	}
	Gfx::set_depth_test_mode( GfxDepthTestMode_NONE );
	Scene::rtSceneMSAA.release();

	// Scene
	Scene::rtSceneComposite.bind();
	Gfx::set_depth_test_mode( GfxDepthTestMode_LESS );
	Gfx::clear_color( Color { 0, 0, 0, 0 } );
	Gfx::clear_depth();
	{
		// Universe
		universe_draw( delta );

		// Earth (Composite)
		Gfx::shader_bind( Shader::SHADER_DEFAULT );
		Gfx::set_depth_test_mode( GfxDepthTestMode_NONE );
		Gfx::set_matrix_mvp_2d_orthographic( 0.0, 0.0, 1.0, 0.0, Window::width, Window::height );
		draw_render_target_2d( Scene::rtSceneMSAA, 0.0f, 0.0f );
		Gfx::clear_depth();
		Gfx::shader_release();

		// Atmosphere
		atmosphere_draw( delta );
	}
	Gfx::set_depth_test_mode( GfxDepthTestMode_NONE );
	Scene::rtSceneComposite.release();

	// Composite
	Gfx::shader_bind( Shader::sh_grade );
	{
		Scene::rtSceneComposite.textureColor.bind( 0 );
		Gfx::draw_vertices( 4, GfxPrimitiveType_TriangleStrip );
		Scene::rtSceneComposite.textureColor.release();
	}
	Gfx::shader_release();
}


void scene_draw_ui( const Delta delta )
{
	Gfx::set_matrix_mvp_2d_orthographic( 0.0, 0.0, 1.0, 0.0, Window::width, Window::height );

	// Labels
	if( Keyboard::check_pressed( vk_l ) ) { labels = !labels; }
	if( labels ) { labels_draw_ui( delta ); }

	// Scene Fade-in
	if( fade > 0.0f )
	{
		Gfx::reset_blend_mode();
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