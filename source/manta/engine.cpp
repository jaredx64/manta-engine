#include <manta/engine.hpp>

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/string.hpp>

#include <manta/terminal.hpp>
#include <manta/profiler.hpp>
#include <manta/assets.hpp>
#include <manta/time.hpp>
#include <manta/thread.hpp>
#include <manta/window.hpp>
#include <manta/gfx.hpp>
#include <manta/console.hpp>
#include <manta/audio.hpp>
#include <manta/objects.hpp>
#include <manta/fonts.hpp>
#include <manta/ui.hpp>
#include <manta/network.hpp>

#include <manta/text.hpp>
#include <manta/input.hpp>
#include <manta/filesystem.hpp>

#include <vendor/signal.hpp>

#include <manta/steamworks.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Engine
{
	static bool init( int argc, char **argv )
	{
		ErrorReturnIf( !CoreThread::init(), false, "Engine: failed to initialize thread" );
		ErrorReturnIf( !CoreTerminal::init(), false, "Engine: failed to initialize terminal" );
		STEAMWORKS( ErrorReturnIf( !Steamworks::init(), false, "Failed to initialize Steam API!" ) );
		ErrorReturnIf( !CoreAssets::init(), false, "Engine: failed to initialize assets" );
		ErrorReturnIf( !CoreTime::init(), false, "Engine: failed to initialize timer" );
		ErrorReturnIf( !CoreWindow::init(), false, "Engine: failed to initialize window" );
		ErrorReturnIf( !CoreGfx::init(), false, "Engine: failed to initialize graphics system" );
		ErrorReturnIf( !CoreConsole::init(), false, "Engine: failed to initialize console system" );
		ErrorReturnIf( !CoreAudio::init(), false, "Engine: failed to initialize audio system" );
		ErrorReturnIf( !CoreObjects::init(), false, "Engine: failed to initialize object system" );
		ErrorReturnIf( !CoreFonts::init(), false, "Engine: failed to initialize font system" );
		ErrorReturnIf( !CoreUI::init(), false, "Engine: failed to initialize UI system" );
		ErrorReturnIf( !CoreNetwork::init(), false, "Engine: failed to initialize network system" );

		PROFILING( CoreProfiler::PROFILER.init() );
		PROFILING( CoreProfiler::PROFILER.capturing = true; );

		return true;
	}

	static bool free()
	{
		PROFILING( CoreProfiler::PROFILER.free() );

		ErrorReturnIf( !CoreNetwork::free(), false, "Engine: failed to free Network system" );
		ErrorReturnIf( !CoreUI::free(), false, "Engine: failed to free UI system" );
		ErrorReturnIf( !CoreFonts::free(), false, "Engine: failed to free font system" );
		ErrorReturnIf( !CoreObjects::free(), false, "Engine: failed to free object system" );
		ErrorReturnIf( !CoreConsole::free(), false, "Engine: failed to free console system" );
		ErrorReturnIf( !CoreGfx::free(), false, "Engine: failed to free graphics system" );
		ErrorReturnIf( !CoreAudio::free(), false, "Engine: failed to free audio system" );
		ErrorReturnIf( !CoreWindow::free(), false, "Engine: failed to free window" );
		ErrorReturnIf( !CoreTime::free(), false, "Engine: failed to free timer" );
		ErrorReturnIf( !CoreAssets::free(), false, "Engine: failed to free assets" );
		STEAMWORKS( ErrorReturnIf( !Steamworks::free(), false, "Failed to free Steam API!" ) );
		ErrorReturnIf( !CoreTerminal::free(), false, "Engine: failed to free terminal" );
		ErrorReturnIf( !CoreThread::free(), false, "Engine: failed to free thread" );

		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Engine
{
	static bool painted = false; // Delay showing the window until after a frame is rendered
	static bool exiting = false; // Terminates the main loop

	int main( int argc, char **argv, const ProjectCallbacks &project )
	{
		// Seg Fault Listener
		signal( SIGSEGV, SEGMENTATION_FAULT_HANLDER_FUNCTION );

		// Directories
		path_get_directory_executable( argv[0] );
		path_get_directory_application( PROJECT_NAME );

	#if 0
		// Log
		Print( PrintColor_Yellow, "\n>" );
		for( int i = 0; i < argc; i++ ) { Print( PrintColor_Yellow, " %s", argv[i] ); }
		Print( "\n" );
	#endif

		// Main Loop
		{
			ErrorIf( !Engine::init( argc, argv ), "Failed to initialize the engine" );
			ErrorIf( !project.init( argc, argv ), "Failed to initialize the project" );

			while( !exiting )
			{
				Frame::start();
				PROFILING( CoreProfiler::PROFILER.frame_start() );
				{
					PROFILE_SCOPE( "Frame" );

					// Pre-Engine
					STEAMWORKS( Steamworks::callbacks() );
					CoreTerminal::update();
					Input::update( Frame::delta );
					Window::update( Frame::delta );
					Input::reset_active();

					// Project
					project.update( Frame::delta );

					// Post-Engine
					TextEditor::listen();

					// Show the window after at least 1 frame has been rendered
					if( !painted ) { CoreWindow::show(); painted = true; }
				}
				PROFILING( CoreProfiler::PROFILER.frame_end() );
				Frame::end();
			}

		#if 0
			// Free Project & Engine (if restarting -- otherwise no need to cleanup)
			ErrorIf( !project.free(), "Failed to free the project" );
			ErrorIf( !Engine::free(), "Failed to free the engine" );
		#else
			Debug::memoryLeakDetection = false;
		#endif
		}

		return Debug::exitCode;
	}

	void exit()
	{
		exiting = true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////