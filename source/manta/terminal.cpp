#include <manta/terminal.hpp>

#if TERMINAL_ENABLED
	#if OS_WINDOWS
		#include <vendor/windows.hpp>
	#elif OS_MACOS || OS_LINUX
		#include <vendor/posix.hpp>
	#endif

	#include <vendor/stdio.hpp>
	#include <vendor/string.hpp>
	#include <manta/thread.hpp>
	#include <manta/console.hpp>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////'
#if TERMINAL_ENABLED

struct TerminalInput { char buffer[256] = { '\0' }; };
static ConcurrentQueue<TerminalInput> inputQueue;


static THREAD_FUNCTION( terminal_poll )
{
	TerminalInput input;
	while( fgets( input.buffer, sizeof( input.buffer ), stdin ) )
	{
		if( input.buffer[0] == '\0' || input.buffer[0] == '\n' ) { continue; }
		input.buffer[strcspn( input.buffer, "\n" )] = '\0';
		inputQueue.enqueue( input );
	}
	return 0;
}


bool CoreTerminal::init()
{
#if OS_WINDOWS && COMPILE_TERMINAL
	if( !AttachConsole( ATTACH_PARENT_PROCESS ) ) { AllocConsole(); }
	FILE *f;
	freopen_s( &f, "CONOUT$", "w", stdout );
	freopen_s( &f, "CONOUT$", "w", stderr );
	freopen_s( &f, "CONIN$",  "r", stdin );

	HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
	if( hOut != INVALID_HANDLE_VALUE )
	{
		DWORD dwMode = 0;
		if( GetConsoleMode( hOut, &dwMode ) )
		{
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode( hOut, dwMode );
		}
	}
#endif

	inputQueue.init();
	Thread::create( terminal_poll );

	return true;
}


bool CoreTerminal::free()
{
	// NOTE: 'terminal_poll' runs for the entire application duration, and the OS will free it up upon exit
	inputQueue.free();
	return true;
}


void CoreTerminal::update()
{
	TerminalInput input;
	while( inputQueue.dequeue( input, false ) )
	{
		Console::command_execute( input.buffer );
		PrintLn( PrintColor_Yellow, "Input: |%s|", input.buffer );
	}
}


void Terminal::get_dimensions( int &columns, int &rows )
{
#if OS_WINDOWS
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );
	columns = static_cast<int>( csbi.srWindow.Right - csbi.srWindow.Left + 1 );
	rows = static_cast<int>( csbi.srWindow.Bottom - csbi.srWindow.Top + 1 );
#elif OS_MACOS || OS_LINUX
	struct winsize ws;
	ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws );
	columns = static_cast<int>( ws.ws_col );
	rows = static_cast<int>( ws.ws_row );
#else
	static_assert( false, "Unsupported OS!" );
#endif
}


void Terminal::clear()
{
	Terminal::write( "\033[2J" );
	Terminal::cursor_move_top_left();
}


void Terminal::clear_line()
{
	Terminal::write( "\033[2K" );
}


void Terminal::clear_to_end()
{
	Terminal::write( "\033[J" );
}


void Terminal::cursor_move( int column, int row )
{
	char buffer[32];
	snprintf( buffer, sizeof( buffer ), "\033[%d;%dH", row, column );
	Terminal::write( buffer );
}


void Terminal::cursor_move_top_left()
{
	Terminal::write( "\033[H" );
}


void Terminal::cursor_save()
{
	Terminal::write( "\033[s" );
}


void Terminal::cursor_restore()
{
	Terminal::write( "\033[u" );
}


void Terminal::cursor_hide()
{
	Terminal::write( "\033[?25l" );
}


void Terminal::cursor_show()
{
	Terminal::write( "\033[?25h" );
}


void Terminal::write( const char *text )
{
	fputs( text, stdout );
}


void Terminal::flush()
{
	fflush( stdout );
}

#else

bool CoreTerminal::init() { return true; }
bool CoreTerminal::free() { return true; }
void CoreTerminal::update() { }

void Terminal::get_dimensions( int &columns, int &rows ) { columns = 0; rows = 0; }
void Terminal::clear() { }
void Terminal::clear_line() { }
void Terminal::clear_to_end() { }

void Terminal::cursor_move( int column, int row ) { }
void Terminal::cursor_move_top_left() { }
void Terminal::cursor_save() { }
void Terminal::cursor_restore() { }
void Terminal::cursor_hide() { }
void Terminal::cursor_show() { }

void Terminal::write( const char *text ) { }
void Terminal::flush() { }

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////