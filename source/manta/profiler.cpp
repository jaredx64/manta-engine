#include <manta/profiler.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if COMPILE_PROFILING

#include <vendor/string.hpp>
#include <vendor/new.hpp>

#include <manta/thread.hpp>
#include <manta/draw.hpp>
#include <manta/input.hpp>
#include <manta/console.hpp>

namespace CoreProfiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ProfilerManager PROFILER;
static bool SHOW_PROFILER = false;


void ProfilerManager::init()
{
	CoreTime::init();

	clear();

	Console::command_init( "profiler <enabled>", "Enable the profiler overlay", CONSOLE_COMMAND_LAMBDA
		{
			SHOW_PROFILER = Console::get_parameter_bool( 0, !SHOW_PROFILER );
			Console::Log( c_white, "profiler %d", SHOW_PROFILER );
		} );
}


void ProfilerManager::free()
{
	/// ...
}


void ProfilerManager::clear()
{
	Assert( !capturingFrame );
	new ( this ) ProfilerManager();
}


void ProfilerManager::start()
{
}


void ProfilerManager::stop()
{
}


void ProfilerManager::frame_start()
{
	capturingFrame = capturing;
	if( !capturingFrame ) { return; }
	snapshotsCount[frameCurrent] = { 0 };
	snapshotCurrent = 0;
	depthCurrent = 0;
	timer.start();
	frameTimeStart = timer.us();
}


void ProfilerManager::frame_end()
{
	if( !capturingFrame ) { return; }
	frameTimeUS[frameCurrent] = timer.us() - frameTimeStart;
	snapshotsCount[frameCurrent] = snapshotCurrent;
	frameCurrent = ( frameCurrent + 1 ) % MAX_FRAMES;

	frameTimeUSAverage = 0.0;
	frameTimeUSHighest = 0.0;
	frameTimeUSLowest = 1000000.0;
	for( int i = 0; i < MAX_FRAMES; i++ )
	{
		const double time = frameTimeUS[i];
		frameTimeUSAverage += time;
		frameTimeUSHighest = max( frameTimeUSHighest, time );
		frameTimeUSLowest = min( frameTimeUSLowest, time );
	}
	frameTimeUSAverage /= static_cast<double>( MAX_FRAMES );
}


void ProfilerManager::draw( Delta delta )
{
	if( Keyboard::check_pressed( vk_f1 ) ) { capturing = !capturing; }
	if( !SHOW_PROFILER ) { return; }

	static int frame = 0;
	char mouseString[1024];
	mouseString[0] = '\0';

	const int width = MAX_FRAMES * 8;
	const int height = 256;

	// Background
	{
		const int x1 = 4;
		const int y1 = 4;
		const int x2 = x1 + width + 24;
		const int y2 = y1 + 40 + height * 2 + 20 + 24;
		draw_rectangle( x1, y1, x2, y2, c_black );

		draw_text_f( fnt_iosevka, 16, x1 + 8, y1 + 8, c_white, "FPS: %d (%.3f ms)",
			Frame::fps, frameTimeUSAverage / 1000.0 );
		const char *status = capturing ? "Capturing Active (F1)" : "Capturing Disabled (F1)";
		const int_v2 statusWH = text_dimensions( fnt_iosevka, 16, status );
		draw_text_f( fnt_iosevka, 16, x2 - 8 - statusWH.x, y1 + 8, capturing ? c_lime : c_red, status );

		draw_text_f( fnt_iosevka, 12, x1 + 8, y1 + 40 + height + 8, c_white, "Frame %u (%.3f ms)",
			frame, frameTimeUS[frame] / 1000.0 );
	}

	// Timeline
	{
		const int x1 = 16;
		const int y1 = 40;
		const int x2 = x1 + width;
		const int y2 = y1 + height;
		draw_rectangle( x1 - 4, y1 - 4, x2 + 4, y2 + 4, c_dkgray );

		for( int i = 0; i < MAX_FRAMES; i++ )
		{
			const double frameUS = frameTimeUS[i];
			const int bx1 = x1 + i * 8;
			const int by1 = y2 - static_cast<int>( static_cast<double>( y2 - y1 ) *
				( frameUS / frameTimeUSHighest ) );
			const int bx2 = bx1 + 8;
			const int by2 = y2;

			const Color c1[2] = { i == frame ? Color { 255, 255, 100 } : c_white, c_yellow };
			const Color c2[2] = { i == frame ? Color { 127, 127, 50 } : c_gray, c_yellow };
			const bool hover = Mouse::x() >= bx1 && Mouse::x() < bx2 && Mouse::y() >= by1 && Mouse::y() < by2;

			if( hover )
			{
				snprintf( mouseString, sizeof( mouseString ), "%.3f ms", frameUS / 1000.0 );
				if( Mouse::check_pressed( mb_left ) ) { frame = i; }
			}

			draw_rectangle_gradient( bx1, by1, bx2, by2, c1[hover], c1[hover], c2[hover], c2[hover] );
			draw_rectangle( bx1, by1, bx2, by2, c_black, true );
		}

		// Current Line
		{
			const int x = x1 + frameCurrent * 8;
			draw_line( x, y1, x, y2, c_yellow, 1.0f );
		}

		// Average Line
		{
			const int y = y2 - static_cast<int>( ( y2 - y1 ) * ( frameTimeUSAverage / frameTimeUSHighest ) );
			draw_line( x1, y, x2 + 8, y, c_red, 2.0f );
			draw_text_f( fnt_iosevka, 16, x2 + 12, y - 8, c_red, "%.3f ms", frameTimeUSAverage / 1000.0 );
		}
	}

	// Snapshot
	{
		const int x1 = 16;
		const int y1 = 40 + height + 32;
		const int x2 = x1 + width;
		const int y2 = y1 + height;
		draw_rectangle( x1 - 4, y1 - 4, x2 + 4, y2 + 4, c_dkgray );

		for( int i = 0; i < snapshotsCount[frame]; i++ )
		{
			ProfilerSnapshot &snapshot = snapshots[frame][i];

			const int bx1 = x1 + static_cast<int>( width * ( snapshot.timeStartUS / frameTimeUS[frame] ) );
			const int bx2 = x1 + static_cast<int>( width * ( snapshot.timeEndUS / frameTimeUS[frame] ) );
			const int by1 = y1 + snapshot.depth * 24;
			const int by2 = y1 + ( snapshot.depth + 1 ) * 24;

			const Color c1[2] = { snapshot.color, c_yellow };
			const Color c2[2] = { snapshot.color * 0.50f, c_yellow };
			const bool hover = Mouse::x() >= bx1 && Mouse::x() < bx2 && Mouse::y() >= by1 && Mouse::y() < by2;

			if( hover )
			{
				snprintf( mouseString, sizeof( mouseString ), "%s\n%s\n%.3f ms",
					snapshot.nameSnapshot, snapshot.nameFunction,
					( snapshot.timeEndUS - 	snapshot.timeStartUS ) / 1000.0 );
			}

			draw_rectangle_gradient( bx1, by1, bx2, by2, c2[hover], c1[hover], c2[hover], c1[hover] );
			draw_rectangle( bx1, by1, bx2, by2, c_black, true );
		}
	}

	// Mouse Time
	if( mouseString[0] != '\0' )
	{
		draw_text( fnt_iosevka, 20, Mouse::x() + 12, Mouse::y() + 16, c_yellow, mouseString );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ProfilerScope::ProfilerScope( const char *nameSnapshot, const char *nameFunction, Color color )
{
	if( !PROFILER.capturingFrame || PROFILER.snapshotCurrent >= ProfilerManager::MAX_SNAPSHOTS ) { return; }
	snapshot = &PROFILER.snapshots[PROFILER.frameCurrent][PROFILER.snapshotCurrent++];
	::strncpy( snapshot->nameSnapshot, nameSnapshot, sizeof( snapshot->nameSnapshot ) - 1 );
	snapshot->nameSnapshot[sizeof( snapshot->nameSnapshot ) - 1] = '\0';
	::strncpy( snapshot->nameFunction, nameFunction, sizeof( snapshot->nameFunction ) - 1 );
	snapshot->nameFunction[sizeof( snapshot->nameFunction ) - 1] = '\0';
	snapshot->color = color;
	snapshot->depth = PROFILER.depthCurrent++;
	snapshot->timeStartUS = PROFILER.timer.us();
}


ProfilerScope::~ProfilerScope()
{
	if( !snapshot ) { return; }
	snapshot->timeEndUS = PROFILER.timer.us();
	PROFILER.depthCurrent--;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif