#include <manta/window.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreWindow
{
	bool init( const int defaultWidth, const int defaultHeight )
	{
		return true;
	}


	bool free()
	{
		return true;
	}


	void show()
	{
	}

	void poll()
	{
	}


	void mouse_get_position( double &x, double &y )
	{
	}


	void mouse_set_position( const int x, const int y )
	{
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Window
{
	void show_message( const char *title, const char *message )
	{
	}


	void show_message_error( const char *title, const char *message )
	{
	}


	bool resize( int width, int height )
	{
		return true;
	}


	void set_fullscreen( bool enabled )
	{
	}


	void set_caption( const char *caption )
	{
	}


	bool set_clipboard( const char *buffer )
	{
		return true;
	}


	bool get_clipboard( char *buffer, const usize size )
	{
		return false;
	}


	bool set_selection( const char *buffer )
	{
		return true;
	}


	bool get_selection( char *buffer, const usize size )
	{
		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////