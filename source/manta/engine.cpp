#include <manta/engine.hpp>

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/string.hpp>

#include <manta/assets.hpp>
#include <manta/time.hpp>
#include <manta/window.hpp>
#include <manta/gfx.hpp>
#include <manta/audio.hpp>
#include <manta/objects.hpp>
#include <manta/fonts.hpp>
#include <manta/ui.hpp>
#include <manta/console.hpp>
#include <manta/thread.hpp>

#include <manta/text.hpp>
#include <manta/input.hpp>
#include <manta/filesystem.hpp>

#include <vendor/signal.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Engine
{
	static bool init( int argc, char **argv )
	{
		// Assets
		ErrorReturnIf( !SysAssets::init(), false, "Engine: failed to initialize assets" );

		// Time
		ErrorReturnIf( !SysTime::init(), false, "Engine: failed to initialize timer" );

		// Thread
		ErrorReturnIf( !SysThread::init(), false, "Engine: failed to initialize thread" );

		// Window
		ErrorReturnIf( !SysWindow::init(), false, "Engine: failed to initialize window" );

		// Graphics
		ErrorReturnIf( !SysGfx::init(), false, "Engine: failed to initialize graphics system" );

		// Audio
		ErrorReturnIf( !SysAudio::init(), false, "Engine: failed to initialize audio system" );

		// Objects
		ErrorReturnIf( !SysObjects::init(), false, "Engine: failed to initialize object system" );

		// Fonts
		ErrorReturnIf( !SysFonts::init(), false, "Engine: failed to initialize font system" );

		// UI
		ErrorReturnIf( !SysUI::init(), false, "Engine: failed to initialize UI system" );

		// Console
		ErrorReturnIf( !SysConsole::init(), false, "Engine: failed to initialize console system" );

		// Success
		return true;
	}

	static bool free()
	{
		// Console
		ErrorReturnIf( !SysConsole::free(), false, "Engine: failed to free console system" );

		// UI
		ErrorReturnIf( !SysUI::free(), false, "Engine: failed to free UI system" );

		// Fonts
		ErrorReturnIf( !SysFonts::free(), false, "Engine: failed to free font system" );

		// Objects
		ErrorReturnIf( !SysObjects::free(), false, "Engine: failed to free object system" );

		// Graphics
		ErrorReturnIf( !SysGfx::free(), false, "Engine: failed to free graphics system" );

		// Audio
		ErrorReturnIf( !SysAudio::free(), false, "Engine: failed to free audio system" );

		// Window
		ErrorReturnIf( !SysWindow::free(), false, "Engine: failed to free window" );

		// Thread
		ErrorReturnIf( !SysThread::free(), false, "Engine: failed to free thread" );

		// Time
		ErrorReturnIf( !SysTime::free(), false, "Engine: failed to free timer" );

		// Assets
		ErrorReturnIf( !SysAssets::free(), false, "Engine: failed to free assets" );

		// Success
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

		// Working Directory & Binary
		path_get_directory( WORKING_DIRECTORY, sizeof( WORKING_DIRECTORY ), argv[0] );

		// Log
#if false
		PrintColor( LOG_YELLOW, "\n>" );
		for( int i = 0; i < argc; i++ ) { PrintColor( LOG_YELLOW, " %s", argv[i] ); }
		Print( "\n" );
#endif

		// Main Loop
		{
			// Init Engine & Project
			ErrorIf( !Engine::init( argc, argv ), "Failed to initialize the engine" );
			ErrorIf( !project.init( argc, argv ), "Failed to initialize the project" );

			// Main Loop
			while( !exiting )
			{
				Frame::start();
				{
					// Pre-Engine
					{
						// Keyboard & Mouse
						Keyboard::update( Frame::delta );
						Mouse::update( Frame::delta );

						// Window
						Window::update( Frame::delta );

						// Keyboard & Mouse
						Keyboard::reset_active();
						Mouse::reset_active();
					}

					// Project
					{
						project.update( Frame::delta );
					}

					// Post-Engine
					{
						// Text Editor
						TextEditor::listen();
					}

					// Show the window after at least 1 frame has been rendered
					if( !painted ) { SysWindow::show(); painted = true; }
				}
				Frame::end();
			}

			// Free Project & Engine (if restarting--otherwise no need to cleanup)
			#if 0
				ErrorIf( !project.free(), "Failed to free the project" );
				ErrorIf( !Engine::free(), "Failed to free the engine" );
			#else
				Debug::memoryLeakDetection = false;
			#endif
		}

		// Return Code
		return Debug::exitCode;
	}

	void exit()
	{
		exiting = true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////