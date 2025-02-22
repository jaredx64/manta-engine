#include <manta/engine.hpp>

#include <manta/random.hpp>

#include <manta/objects.hpp>
#include <manta/time.hpp>
#include <manta/window.hpp>
#include <manta/gfx.hpp>
#include <manta/input.hpp>
#include <manta/draw.hpp>
#include <manta/fonts.hpp>

#include <scene.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Project
{
	bool init( int argc, char **argv )
	{
		scene_init();
		return true;
	}

	bool free()
	{
		scene_free();
		return true;
	}

	void update( Delta delta )
	{
		// Hotkeys
		if( Keyboard::check_pressed( vk_escape ) ) { Engine::exit(); }
		if( Keyboard::check_pressed( vk_f4 ) ) { Window::set_fullscreen( !Window::fullscreen ); }

		// Update Scene
		scene_update( delta );

		// Render
		Gfx::frame_begin();
		{
			Gfx::clear_color( { 20, 20, 40 } );
			Gfx::set_matrix_mvp_2d_orthographic( 0.0f, 0.0f, 1.0f, 0.0f, Window::width, Window::height );

			// Draw Scene
			scene_draw( delta );

			// Debug
			draw_text_f( fnt_iosevka, 20, 8.0f, 8.0f, c_white, "FPS: %d", Frame::fps );
		}
		Gfx::frame_end();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{
	ProjectCallbacks callbacks { Project::init, Project::free, Project::update };
	return Engine::main( argc, argv, callbacks );
}