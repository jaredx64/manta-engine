#pragma once

#include <config.hpp>

#include <core/types.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( HEADLESS )
	#define WINDOW_ENABLED ( true )
#else
	#define WINDOW_ENABLED ( false )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreWindow
{
	extern bool init( int width = WINDOW_WIDTH_DEFAULT, int height = WINDOW_HEIGHT_DEFAULT );
	extern bool free();
	extern void show();
	extern void poll();

	extern void terminal_init();

	extern void mouse_get_position( double &x, double &y );
	extern void mouse_set_position( int x, int y );

	extern void set_mouselook( bool enabled );

	extern bool ime_init();
	extern void ime_quit();
	extern void ime_set_focus( bool focused );
	extern void ime_reset();
	extern bool ime_process_key_event( u32 keysym, u32 keycode, bool down );
	extern void ime_pump_events();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Window
{
	extern void update( Delta delta );

	extern void show_message( const char *title, const char *message );
	extern void show_message_error( const char *title, const char *message );

	extern void set_size( int width, int height );
	extern void set_fullscreen( bool enabled );
	extern void show_cursor( bool enabled );
	extern void set_caption( const char *caption );

	extern bool set_clipboard( const char *buffer );
	extern bool get_clipboard( char *buffer, usize size );
	extern bool set_selection( const char *buffer );
	extern bool get_selection( char *buffer, usize size );

	extern int width_logical();
	extern int height_logical();
	extern int width_pixels();
	extern int height_pixels();

	extern int width;
	extern int height;
	extern int widthDefault;
	extern int heightDefault;
	extern int widthMin;
	extern int heightMin;
	extern int widthMax;
	extern int heightMax;

	extern float scale;

	extern bool hasFocus;
	extern bool allowFullscreen;
	extern bool allowResize;
	extern bool fullscreen;
	extern bool resized;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////