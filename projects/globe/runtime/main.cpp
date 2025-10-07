#include <manta/engine.hpp>

#include <manta/random.hpp>

#include <manta/objects.hpp>
#include <manta/time.hpp>
#include <manta/window.hpp>
#include <manta/gfx.hpp>
#include <manta/input.hpp>
#include <manta/draw.hpp>
#include <manta/fonts.hpp>
#include <manta/console.hpp>

#include <manta/3d.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <universe.hpp>
#include <earth.hpp>
#include <atmosphere.hpp>
#include <view.hpp>
#include <labels.hpp>
#include <scene.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Project
{
	bool init( int argc, char **argv )
	{
		universe_init();
		earth_init();
		atmosphere_init();
		scene_init();

		return true;
	}

	bool free()
	{
		universe_free();
		earth_free();
		atmosphere_free();
		scene_free();

		return true;
	}

	void update( Delta delta )
	{
		// Hotkeys
		if( Keyboard::check_pressed( vk_escape ) ) { Engine::exit(); }
		if( Keyboard::check_pressed( vk_f4 ) ) { Window::set_fullscreen( !Window::fullscreen ); }

		// Update
		view_controls( delta );
		view_update( delta );
		universe_update( delta );
		earth_update( delta );
		atmosphere_update( delta );

		// Render
		Gfx::frame_begin();
		{
			// Scene
			scene_draw_3d( delta );
			scene_draw_ui( delta );

			// Console
			Console::draw( delta );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////