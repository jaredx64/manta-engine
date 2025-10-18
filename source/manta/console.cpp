#include <manta/console.hpp>

#include <core/list.hpp>
#include <core/string.hpp>
#include <core/math.hpp>
#include <core/color.hpp>

#include <manta/window.hpp>
#include <manta/input.hpp>
#include <manta/window.hpp>
#include <manta/draw.hpp>
#include <manta/text.hpp>
#include <manta/ui.hpp>
#include <manta/objects.hpp>
#include <manta/time.hpp>
#include <manta/thread.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Keyboard keyboard;
static Mouse mouse;
static UIContext ui;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr Color COLOR_PANELS = Color { 35, 40, 55, 255 };

constexpr Color COLOR_COMMAND = c_white;
constexpr Color COLOR_HINT_COMMAND = Color { 255, 255, 255, 120 };
constexpr Color COLOR_HINT_PARAMETER_REQUIRED = Color { 255, 255, 255, 120 };
constexpr Color COLOR_HINT_PARAMETER_OPTIONAL = Color { 255, 255, 255, 120 };
constexpr Color COLOR_DESC_COMMAND = Color { 255, 220, 175, 255 };
constexpr Color COLOR_DESC_PARAM = Color { 110, 150, 175, 255 };
constexpr Color COLOR_NAVIGATE = Color { 150, 255, 0, 255 };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Console::backgroundAlpha = 0.70f;
float Console::logAlpha = 0.55f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TOKEN_SIZE ( 63 )
#define COMMAND_SIZE ( 256 )
#define LINE_SIZE ( 1024 )

#define LOG_MAX ( 4096 )
#define HISTORY_MAX ( 128 )
#define PARAMETERS_MAX ( 16 )

#define PARAMETER_CHAR_REQUIRED ( '[' )
#define PARAMETER_CHAR_OPTIONAL ( '<' )
#define PARAMETER_CHAR_DEFAULT  ( '*' )

#define PARAMETER_INDEX(command, param) \
	static_cast<usize>( ( ( command )->numRequiredTokens + ( param ) ) )

enum_type( TokenCompare, int )
{
	TokenCompare_Unequal = -1,
	TokenCompare_Equal = 0,
	TokenCompare_Partial = 1,
	TOKENCOMPARE_COUNT,
};

enum_type( CommandCompare, int )
{
	CommandCompare_Exact, // Exact match against token list
	CommandCompare_Close, // Close match against token list (final token is partial match, but incomplete)
	CommandCompare_Partial, // Partial match against token list (more one or more additional tokens still required)
	CommandCompare_Contains, // Command comtains at least part of the token list
	CommandCompare_None, // No match against token list
	COMMANDCOMPARE_COUNT,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Token
{
public:
	Token()
	{
		string[0] = '\0';
		length = 0;
	}

	Token( const Token &other )
	{
		memory_copy( string, other.string, sizeof( string ) );
		length = other.length;
	}

	Token( Token &&other )
	{
		memory_copy( string, other.string, sizeof( string ) );
		length = other.length;

		other.string[0] = '\0';
		other.length = 0;
	}

	Token &operator=( const Token &other )
	{
		memory_copy( string, other.string, sizeof( string ) );
		length = other.length;
		return *this;
	}

	Token &operator=( Token &&other )
	{
		memory_copy( string, other.string, sizeof( string ) );
		length = other.length;

		other.string[0] = '\0';
		other.length = 0;
		return *this;
	}

	int init( const char *buffer )
	{
		// Read Token
		bool inQuotes = false;
		int indexBuffer;
		int indexToken;
		for( indexBuffer = 0, indexToken = 0; indexBuffer <= TOKEN_SIZE; indexBuffer++ )
		{
			// Null-terminator
			if( buffer[indexBuffer] == '\0' ) { string[indexToken] = '\0'; indexBuffer--; break; }

			// Spaces
			const bool charSpace = buffer[indexBuffer] == ' ';
			if( charSpace && !inQuotes ) { string[indexToken] = '\0'; break; }

			// Quotes
			const bool charQuote = buffer[indexBuffer] == '\"';
			if( charQuote && inQuotes ) { string[indexToken] = '\0'; break; } else
			if( charQuote && !inQuotes ) { inQuotes = true; continue; }

			// Copy Character
			string[indexToken++] = buffer[indexBuffer];
		}
		string[TOKEN_SIZE - 1] = '\0';

		// Calculate Length
		length = static_cast<u8>( strlen( string ) );

		// Success
		return indexBuffer;
	}

	TokenCompare compare( const Token &other ) const
	{
		if( length < other.length )
		{
			return TokenCompare_Unequal;
		}
		else if( length == other.length )
		{
			for( int i = 0; i < length; i++ )
			{
				if( chrlower( string[i] ) != chrlower( other.string[i] ) ) { return TokenCompare_Unequal; }
			}
			return TokenCompare_Equal;
		}
		else
		{
			for( int i = 0; i < other.length; i++ )
			{
				if( chrlower( string[i] ) != chrlower( other.string[i] ) ) { return TokenCompare_Unequal; }
			}
			return TokenCompare_Partial;
		}
	};

public:
	char string[TOKEN_SIZE];
	u8 length;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Command
{
public:
	Command() { };
	Command( const Command &other ) { }
	Command( Command &&other ) { }

	void init( const char *definition, const char *description,
		CONSOLE_COMMAND_FUNCTION_POINTER( function ), void *payload )
	{
		// Initialize Lists
		this->tokens.init();
		this->string[0] = '\0';
		snprintf( this->string, sizeof( this->string ), "%s", definition );
		this->description = description;
		this->function = function;
		this->payload = payload;

		// Tokenize the definition string
		bool sawRequired = false;
		bool sawParam = false;
		bool sawOption = false;
		for( int i = 0; i < COMMAND_SIZE; i++ )
		{
			// Null-terminator
			if( definition[i] == '\0' ) { break; }

			// Tokenize
			Token token;
			const int increment = token.init( &definition[i] );
			if( token.length == 0 ) { continue; }

			// Validate Tokens
			if( token.string[0] == PARAMETER_CHAR_REQUIRED )
			{
				// Required Parameter
				ErrorIf( sawOption, "Console: optional params must be at the end\n\n\t'%s'", definition );
				sawParam = true;
				numRequiredParams++;
				numParams++;
			}
			else if( token.string[0] == PARAMETER_CHAR_OPTIONAL )
			{
				// Optional Parameter
				sawParam = true;
				sawOption = true;
				numParams++;
			}
			else
			{
				ErrorIf( sawParam, "Console: params must be at the end\n\n\t'%s'", definition );
				sawRequired = true;
				numRequiredTokens++;
			}

			ErrorIf( numParams > PARAMETERS_MAX,
				"Console: exceeded maximum number of allowed parameters (%d)\n\n\t'%s'",
				PARAMETERS_MAX, definition )

			// Register token
			i += increment;
			this->tokens.add( token );
		}

		ErrorIf( !sawRequired, "Console: commands must have at least one token\n\n\t'%s'", definition );
	};

	void free()
	{
		// Free Lists
		tokens.free();
	};

	CommandCompare compare( List<Token> &inputTokens ) const
	{
		CommandCompare match = CommandCompare_None;
		if( inputTokens.count() == 0 ) { return match; }

		// Loop through input tokens
		for( usize index = 0; index < inputTokens.count(); index++ )
		{
			// Too many input tokens
			if( index >= tokens.count() )
			{
				match = CommandCompare_None;
				break;
			}

			const Token &commandToken = tokens[index];
			const Token &inputToken = inputTokens[index];
			const bool isParameter = commandToken.string[0] == PARAMETER_CHAR_REQUIRED ||
				commandToken.string[0] == PARAMETER_CHAR_OPTIONAL;

			if( isParameter )
			{
				// Satisfies the command requirements?
				if( index + 1 == static_cast<usize>( numRequiredTokens + numRequiredParams ) )
				{
					match = CommandCompare_Exact;
					continue;
				}
			}
			else
			{
				// Token is a perfect match?
				if( commandToken.compare( inputToken ) == TokenCompare_Equal )
				{
					// Satisfies the command requirements?
					if( index + 1 == static_cast<usize>( numRequiredTokens + numRequiredParams ) )
					{
						match = CommandCompare_Exact;
						continue;
					}

					// Partial match, but we need additional tokens
					match = CommandCompare_Partial;
					continue;
				}
				// Token is a partial match?
				else if( commandToken.compare( inputToken ) == TokenCompare_Partial )
				{
					// Once completed, does it satisfy the command requirements?
					if( index + 1 == static_cast<usize>( numRequiredTokens + numRequiredParams ) )
					{
						match = CommandCompare_Close;
						continue;
					}
					// Partial match to this token, but we also need additional tokens
					else if( index + 1 == inputTokens.count() )
					{
						match = CommandCompare_Partial;
						continue;
					}
					// Token does not match
					else
					{
						match = CommandCompare_None;
						break;
					}
				}
				// Token does not match
				else
				{
					match = CommandCompare_None;
					break;
				}
			}
		}

		// No match, see if the command contains any of the tokens
		if( match == CommandCompare_None )
		{
			const usize count = min( tokens.count(), static_cast<usize>( numRequiredTokens ) );
			for( usize i = 0, j = 0; i < count; i++ )
			{
				const Token &commandToken = tokens[i];
				const Token &inputToken = inputTokens[j];
				if( strstr( commandToken.string, inputToken.string ) != nullptr && ( ++j == inputTokens.count() ) )
				{
					match = CommandCompare_Contains;
					break;
				}
			}
		}

		// Return
		return match;
	}

	void set_as_input() const
	{
		const u8 length = min( static_cast<usize>( strlen( string ) ),
			min( static_cast<usize>( strchr( string, PARAMETER_CHAR_REQUIRED ) - string ),
			     static_cast<usize>( strchr( string, PARAMETER_CHAR_OPTIONAL ) - string ) ) );

		char buffer[256];
		snprintf( buffer, sizeof( buffer ), "%.*s", length, string );

		Console::set_input( buffer );
	}

public:
	CommandHandle handle = 0;
	char string[COMMAND_SIZE];
	const char *description;
	CONSOLE_COMMAND_FUNCTION_POINTER( function ) = nullptr;
	void *payload = nullptr;
	const char *paramDesc[PARAMETERS_MAX];
	bool hidden = false;
	bool disabled = false;

	List<Token> tokens;
	int numRequiredTokens = 0;
	int numRequiredParams = 0;
	int numParams = 0;

	float tween = 0.0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CommandHistory
{
	CommandHistory( const char *command ) { snprintf( string, sizeof( string ), "%s", command ); }

	char string[COMMAND_SIZE];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct LogLine
{
	LogLine( const Color color, const char *command, const char *string )
	{
		this->color = color;
		snprintf( this->command, sizeof( this->command ), "%s", command );
		snprintf( this->string, sizeof( this->string ), "%s", string );
	}

	Color color;
	char command[COMMAND_SIZE];
	char string[COMMAND_SIZE];

	float tween = 0.0f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Region
{
public:
	Region() { reset(); }

	void reset()
	{
		dirty = false;
		hover = false;
		offset = int_v2 { 0, 0 };
		dimensionsArea = int_v2 { 0, 0 };
		dimensionsView = int_v2 { 0, 0 };
		anchorMouse = int_v2 { 0.0f, 0.0f };
		scrollbarVisible = u8_v2 { true, true };
		scrollbarValue = float_v2 { 0.0f, 0.0f };
		scrollbarValueTo = float_v2 { 0.0f, 0.0f };
		scrollbarSliding = u8_v2 { false, false };
		scrollbarTweenH = float_v2 { 0.0f, 0.0f };
		scrollbarTweenV = float_v2 { 0.0f, 0.0f };
	}

	void draw_scrollbar( const Delta delta, const int x, const int y, bool vertical )
	{
		const Color colorSlider = Color { 80, 95, 105, 255 };
		const Color colorBar = { 25, 30, 40, 255 };

		// Vertical / Horizontal Bars
		const int &sizeView = vertical ? dimensionsView.y : dimensionsView.x;
		const int &sizeArea = vertical ? dimensionsArea.y : dimensionsArea.x;
		u8 &sliding = vertical ? scrollbarSliding.y : scrollbarSliding.x;
		float &value = vertical ? scrollbarValue.y : scrollbarValue.x;
		float &valueTo = vertical ? scrollbarValueTo.y : scrollbarValueTo.x;
		float_v2 &tweens = vertical ? scrollbarTweenV : scrollbarTweenH;

		// Calculate Positions
		const float frac = static_cast<float>( sizeView ) / sizeArea;
		const int_v2 size = vertical ?
			int_v2 { scrollbarThickness, max( static_cast<int>( sizeView * frac ), 16 ) } :
			int_v2 { max( static_cast<int>( sizeView * frac ), 16 ), scrollbarThickness };

		const int_v4 bounds = vertical ?
			int_v4 { x + dimensionsView.x, y, x + dimensionsView.x + size.x, y + sizeView } :
			int_v4 { x, y + dimensionsView.y, x + sizeView, y + dimensionsView.y + size.y };

		const int_v2 position = vertical ?
			int_v2 { 0, static_cast<int>( ( sizeView - size.y ) * value ) } :
			int_v2 { static_cast<int>( ( sizeView - size.x ) * value ), 0 };

		// Mouse Interaction
		const int mX = mouse_x;
		const int mY = mouse_y;

		const bool isHovering = point_in_rect( mX, mY, bounds.x, bounds.y, bounds.z, bounds.w );

		const bool isHoveringSlider = sliding ||
			point_in_rect( mX, mY,
				bounds.x + position.x, bounds.y + position.y,
				bounds.x + position.x + size.x, bounds.y + position.y + size.y );

		if( isHovering )
		{
			// Tween
			tween( tweens.x, sliding || !isHoveringSlider ? 1.0f : 0.0f, 0.1f, delta, 0.01f );
			tween( tweens.y, sliding || isHoveringSlider ? 1.0f : 0.0f, 0.1f, delta, 0.01f );

			// Activate Sliding
			if( Mouse::check_pressed( mb_left ) )
			{
				sliding = true;
				if( isHoveringSlider )
				{
					anchorMouse = int_v2 { mouse_x - ( bounds.x + position.x + size.x / 2 ),
					                      mouse_y - ( bounds.y + position.y + size.y / 2 ) };
				}
			}
		}
		else
		{
			// No hover
			tween( tweens.x, 0.0f, 0.05f, delta, 0.01f );
			tween( tweens.y, 0.0f, 0.05f, delta, 0.01f );
		}

		// Sliding
		const float percentMouseX = ( ( mouse_x - anchorMouse.x ) -
			( bounds.x + size.x / 2 ) ) / ( sizeView - size.x );
		const float percentMouseY = ( ( mouse_y - anchorMouse.y ) -
			( bounds.y + size.y / 2 ) ) / ( sizeView - size.y );
		const float percentMouse = clamp( vertical ? percentMouseY : percentMouseX, 0.0f, 1.0f );
		if( Mouse::check_released( mb_left ) )
		{
			sliding = false;
			anchorMouse = int_v2 { 0, 0 };
		}
		else if( sliding && Mouse::check( mb_left ) )
		{
			// Active sliding
			valueTo = percentMouse;
		}

		// Draw Scollbar
		const Color colorBackground = color_mix( colorBar, c_white, tweens.x * 0.1f );
		const Color colorMain = color_mix(
			color_mix( colorSlider, c_white, tweens.y * 0.2f ), c_lime, sliding );

		// Draw Background
		draw_rectangle( bounds.x, bounds.y, bounds.z, bounds.w, colorBackground );
		draw_rectangle( bounds.x, bounds.y, bounds.z, bounds.w, c_black, true );

		// Draw Slider
		draw_rectangle( bounds.x + position.x, bounds.y + position.y,
			bounds.x + position.x + size.x, bounds.y + position.y + size.y, colorMain );
		draw_rectangle( bounds.x + position.x, bounds.y + position.y,
			bounds.x + position.x + size.x, bounds.y + position.y + size.y, c_black, true );
	}

	void draw( const Delta delta, const int x, const int y )
	{
		// Horizontal Scrollbar
		if( dimensionsArea.x > dimensionsView.x )
		{
			if( scrollbarVisible.x ) { draw_scrollbar( delta, x, y, false ); }
		}
		else
		{
			scrollbarValue.x = 0.0f;
			scrollbarValueTo.x = 0.0f;
			scrollbarTweenH.x = 0.0f;
			scrollbarTweenH.y = 0.0f;
		}
		tween( scrollbarValue.x, scrollbarValueTo.x, 0.025f, delta, 0.01f );
		offset.x = -( dimensionsArea.x - min( dimensionsView.x, dimensionsArea.x ) ) * scrollbarValue.x;

		// Vertical Scrollbar
		if( dimensionsArea.y > dimensionsView.y )
		{
			if( scrollbarVisible.y ) { draw_scrollbar( delta, x, y, true ); }
		}
		else
		{
			scrollbarValue.y = 0.0f;
			scrollbarValueTo.y = 0.0f;
			scrollbarTweenH.y = 0.0f;
			scrollbarTweenH.y = 0.0f;
		}
		tween( scrollbarValue.y, scrollbarValueTo.y, 0.025f, delta, 0.01f );
		offset.y = -( dimensionsArea.y - min( dimensionsView.y, dimensionsArea.y ) ) * scrollbarValue.y;
	}

	void scroll_percent( const float increment, const bool vertical )
	{
		float &valueTo = vertical ? scrollbarValueTo.y : scrollbarValueTo.x;
		valueTo = clamp( valueTo + increment, 0.0f, 1.0f );
	}

	void scroll_pixels( const int increment, const bool vertical )
	{
		float &valueTo = vertical ? scrollbarValueTo.y : scrollbarValueTo.x;
		const int &sizeView = vertical ? dimensionsView.y : dimensionsView.x;
		const int &sizeArea = vertical ? dimensionsArea.y : dimensionsArea.x;

		const int scrollableSize = sizeArea - min( sizeView, sizeArea );
		const float pixelsPercent = static_cast<float>( increment ) / scrollableSize;
		valueTo = clamp( valueTo + pixelsPercent, 0.0f, 1.0f );
	}

	void set_position_pixels( const int position, const bool vertical )
	{
		float &valueTo = vertical ? scrollbarValueTo.y : scrollbarValueTo.x;
		const int &sizeView = vertical ? dimensionsView.y : dimensionsView.x;
		const int &sizeArea = vertical ? dimensionsArea.y : dimensionsArea.x;

		const int scrollableSize = sizeArea - min( sizeView, sizeArea );
		const float pixelsPercent = static_cast<float>( position ) / scrollableSize;
		valueTo = clamp( pixelsPercent, 0.0f, 1.0f );
	}

	void set_position_percent( const float position, const bool vertical )
	{
		float &valueTo = vertical ? scrollbarValueTo.y : scrollbarValueTo.x;
		valueTo = clamp( position, 0.0f, 1.0f );
	}

public:
	bool dirty;
	bool hover;
	int_v2 offset;
	int_v2 dimensionsArea;
	int_v2 dimensionsView;
	int_v2 anchorMouse;
	u8_v2 scrollbarVisible;
	int_v2 scrollbarSize;
	float_v2 scrollbarValue;
	float_v2 scrollbarValueTo;
	u8_v2 scrollbarSliding;
	float_v2 scrollbarTweenH; // x = background, y = slider
	float_v2 scrollbarTweenV; // x = background, y = slider
	constexpr static int scrollbarThickness = 12;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreConsole
{
	static bool initialized = false;
	static bool open = false;
	static bool updateCandidates = false;
	static bool hideCandidates = false;

	static bool stdinAlive = false;
	static bool stdinKill = false;

	static TextEditor input;
	static char buffer[LINE_SIZE];

	static List<LogLine> log;
	static List<Token> tokens;
	static List<Command> commands;
	static List<CommandHistory> history;
	static List<usize> candidates[COMMANDCOMPARE_COUNT];

	static CommandHandle handleCurrent = 0;

	static usize logIndex = USIZE_MAX;
	static usize historyIndex = USIZE_MAX;
	static usize candidateIndex = USIZE_MAX;

	// Regions
	Region inputRegion;
	Region logRegion;
	Region candidateRegion;

	static void reset_regions()
	{
		inputRegion.reset();
		logRegion.reset();
		candidateRegion.reset();
	}

	static Command &get_candidate_command( const usize index )
	{
		for( CommandCompare i = 0, j = 0; i <= CommandCompare_Contains; i++ )
		{
			for( const usize &candidate : CoreConsole::candidates[i] )
			{
				Assert( candidate < CoreConsole::commands.count() );
				if( static_cast<usize>( j++ ) == index ) { return CoreConsole::commands[candidate]; }
			}
		}

		// Unreachable
		Assert( false );
		return CoreConsole::commands[0];
	}

	static usize candidates_count()
	{
		const usize count =
			CoreConsole::candidates[CommandCompare_Exact].count() +
			CoreConsole::candidates[CommandCompare_Close].count() +
			CoreConsole::candidates[CommandCompare_Partial].count() +
			CoreConsole::candidates[CommandCompare_Contains].count();
		return count;
	}

	static usize candidates_parameters_count()
	{
		if( candidates_count() != 1 ) { return 0; }
		const Command &command = CoreConsole::get_candidate_command( 0 );
		return command.numParams;
	}

	static usize candidates_count_view()
	{
		const usize viewLimit = min(
			candidates_count() + candidates_parameters_count(),
			static_cast<usize>( ( Window::height - 128 ) / 24 ) );
		return max( viewLimit, 1LLU );
	}

	static usize log_find_next_command( const usize index )
	{
		for( usize i = index == USIZE_MAX ? 0 : index + 1; i < CoreConsole::log.count(); i++ )
		{
			const LogLine &line = CoreConsole::log[i];
			if( line.command[0] != '\0' ) { return i; }
		}
		return USIZE_MAX;
	}

	static usize log_find_previous_command( const usize index )
	{
		for( usize i = index == USIZE_MAX ? CoreConsole::log.count() : index; i > 0; i-- )
		{
			const LogLine &line = CoreConsole::log[i - 1];
			if( line.command[0] != '\0' ) { return i - 1; }
		}
		return USIZE_MAX;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

THREAD_FUNCTION( stdin_listener )
{
	char buffer[256];

	CoreConsole::stdinAlive = true;
	while( !CoreConsole::stdinKill )
	{
		Thread::sleep( 100 );
		if( fgets( buffer, sizeof( buffer ), stdin ) == nullptr ) { continue; }
		Console::command_execute( buffer );
		buffer[0] = '\0';
	}
	CoreConsole::stdinAlive = false;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreConsole::init()
{
	// Init Objects & UI Context
	ui.init();

	// Initialize TextEditor
	input.allowNewlines = false;
	input.allowTabs = false;
	input->filter = Text::filter_console;
	input->limitCharacters = 255;
	input->callbackOnUpdate = []( Text &text ) { CoreConsole::updateCandidates = true; };

	TextFormat format;
	format.font = fnt_iosevka;
	format.bold = true;
	format.size = 16;
	format.color = COLOR_COMMAND;
	input->defaultFormat = format;

	// Initalize Memory
	CoreConsole::log.init( LOG_MAX );
	CoreConsole::tokens.init();
	CoreConsole::commands.init();
	CoreConsole::history.init( HISTORY_MAX );
	for( int i = 0; i < COMMANDCOMPARE_COUNT; i++ ) { CoreConsole::candidates[i].init(); }

	// System Ready
	CoreConsole::initialized = true;

	// Initialize Deferred DVars
	CoreConsole::DVarInitializer &initializer = CoreConsole::DVarInitializer::get_instance();
	initializer.defer = false;
	for( usize i = 0; i < initializer.current; i++ )
	{
		// Register Command
		CoreConsole::DVarInitializer::dvar_register(
			initializer.dvars[i].definition,
			initializer.dvars[i].parameters,
			initializer.dvars[i].description,
			initializer.dvars[i].payload,
			initializer.dvars[i].function );
	}
	CoreConsole::DVarInitializer::reset_instance();

	// Built-in Commands
	Console::command_init( "help", "Lists all available commands", CONSOLE_COMMAND_LAMBDA
		{
			char buffer[COMMAND_SIZE];

			Console::Log( c_white, "" );
			for( auto itr = CoreConsole::commands.rbegin(); itr != CoreConsole::commands.rend(); ++itr )
			{
				const Command &command = *itr;
				if( command.hidden || command.disabled ) { continue; }

				const u8 length = min( static_cast<usize>( strlen( command.string ) ),
					min( static_cast<usize>( strchr( command.string, PARAMETER_CHAR_REQUIRED ) - command.string ),
						 static_cast<usize>( strchr( command.string, PARAMETER_CHAR_OPTIONAL ) - command.string ) ) );

				snprintf( buffer, sizeof( buffer ), "  %.*s", length, command.string );
				Console::LogCommand( c_white, &buffer[2], buffer );
			}
			Console::Log( c_yellow, "Available Commands:" );
		} );

	Console::command_init( "clear", "Clears the log", CONSOLE_COMMAND_LAMBDA
		{
			Console::clear_log();
		} );

	Console::command_init( "fps [limit]", "Sets the maximum fps", CONSOLE_COMMAND_LAMBDA
		{
			Frame::fpsLimit = Console::get_parameter_u32( 0, Frame::fpsLimit );
			Console::Log( c_lime, "fps %u", Frame::fpsLimit );
		} );

#if 0
	// Standard In Listener
	CoreConsole::stdinAlive = false;
	CoreConsole::stdinKill = false;
	Thread::create( stdin_listener );
#endif

	// Success
	return true;
}


bool CoreConsole::free()
{
#if 0
	// Standard In Listener
	CoreConsole::stdinKill = true;
	while( CoreConsole::stdinAlive ) { Thread::sleep( 1 ); }
#endif

	// Close Console
	Console::close();

	// Initalize Memory
	for( int i = 0; i < COMMANDCOMPARE_COUNT; i++ ) { CoreConsole::candidates[i].free(); }
	CoreConsole::history.free();
	CoreConsole::commands.free();
	CoreConsole::tokens.free();
	CoreConsole::log.free();

	// Init Objects & UI Context
	ui.free();

	// Success
	CoreConsole::initialized = false;
	return true;
}


Command *CoreConsole::command( const CommandHandle handle )
{
	for( usize i = 0; i < CoreConsole::commands.count(); i++ )
	{
		if( CoreConsole::commands[i].handle == handle ) { return &CoreConsole::commands[i]; }
	}
	return nullptr;
}


void CoreConsole::tokenize_input( const char *string )
{
	// Clear Tokens
	CoreConsole::tokens.clear();

	// Tokenize Input
	for( int i = 0; i < COMMAND_SIZE; i++ )
	{
		// Null-terminator
		if( string[i] == '\0' ) { break; }

		// Tokenize
		Token token;
		const int increment = token.init( &string[i] );
		if( token.length == 0 ) { continue; } else { i += increment; }
		CoreConsole::tokens.add( token );
	}
}


void CoreConsole::update_input()
{
	CoreConsole::tokens.clear();
	CoreConsole::input->cstr( CoreConsole::buffer, sizeof( CoreConsole::buffer ) );
	CoreConsole::tokenize_input( CoreConsole::buffer );
}


void CoreConsole::update_candidates( const bool processHiddenCommands )
{
	// Reset Candidates List
	for( int i = 0; i < COMMANDCOMPARE_COUNT; i++ ) { CoreConsole::candidates[i].clear(); }

	// Loop through registered commands and test them
	usize numExact = 0;
	usize numExactPartial = 0;
	usize numPartial = 0;

	for( usize candidate = 0; candidate < CoreConsole::commands.count(); candidate++ )
	{
		const Command &command = CoreConsole::commands[candidate];
		if( ( !processHiddenCommands && command.hidden ) || command.disabled ) { continue; }
		CoreConsole::candidates[command.compare( CoreConsole::tokens )].add( candidate );
	}

	// Mark Region Dirty
	CoreConsole::candidateRegion.dirty = true;
}


void CoreConsole::navigate()
{
	// Navigation
	static char inputCache[COMMAND_SIZE];
	const usize candidatesCount = CoreConsole::candidates_count();
	Region &candidateRegion = CoreConsole::candidateRegion;
	usize &candidateIndex = CoreConsole::candidateIndex;
	Region &logRegion = CoreConsole::logRegion;
	usize &logIndex = CoreConsole::logIndex;
	usize &historyIndex = CoreConsole::historyIndex;

	// Navigate Candidates / Log
	if( Keyboard::check( vk_shift ) )
	{
		if( Keyboard::check_pressed( vk_up ) )
		{
			// Navigate Candidates
			if( candidatesCount > 0 )
			{
				if( candidateIndex == USIZE_MAX )
				{
					CoreConsole::input->cstr( inputCache, sizeof( inputCache ) );
					candidateIndex = candidatesCount - 1;
					CoreConsole::get_candidate_command( candidateIndex ).set_as_input();
					candidateRegion.set_position_percent( 0.0f, false );
				}
				else
				{
					candidateIndex = candidateIndex > 0 ? candidateIndex - 1 : USIZE_MAX;

					if( candidateIndex == USIZE_MAX )
					{
						Console::set_input( inputCache );
					}
					else
					{
						CoreConsole::get_candidate_command( candidateIndex ).set_as_input();
					}

					candidateRegion.set_position_percent( 0.0f, false );
				}
			}
			// Navigate Log
			else
			{
				if( logIndex == USIZE_MAX )
				{
					CoreConsole::input->cstr( inputCache, sizeof( inputCache ) );
				}

				logIndex = CoreConsole::log_find_next_command( logIndex );
				logRegion.set_position_percent( 0.0f, false );

				Console::set_input( logIndex == USIZE_MAX ? inputCache : CoreConsole::log[logIndex].command );
			}

			CoreConsole::updateCandidates = false;
		}
		else if( Keyboard::check_pressed( vk_down ) )
		{
			// Navigate Candidates
			if( candidatesCount > 0 )
			{
				if( candidateIndex == USIZE_MAX )
				{
					CoreConsole::input->cstr( inputCache, sizeof( inputCache ) );
					candidateIndex = 0;
					CoreConsole::get_candidate_command( candidateIndex ).set_as_input();
					candidateRegion.set_position_percent( 0.0f, false );
				}
				else
				{
					candidateIndex = candidateIndex < candidatesCount - 1 ? candidateIndex + 1 : USIZE_MAX;

					if( candidateIndex == USIZE_MAX )
					{
						Console::set_input( inputCache );
					}
					else
					{
						CoreConsole::get_candidate_command( candidateIndex ).set_as_input();
					}

					candidateRegion.set_position_percent( 0.0f, false );
				}
			}
			// Navigate Log
			else
			{
				if( logIndex == USIZE_MAX )
				{
					CoreConsole::input->cstr( inputCache, sizeof( inputCache ) );
				}

				logIndex = CoreConsole::log_find_previous_command( logIndex );
				logRegion.set_position_percent( 0.0f, false );

				Console::set_input( logIndex == USIZE_MAX ? inputCache : CoreConsole::log[logIndex].command );
			}

			CoreConsole::updateCandidates = false;
		}
	}
	// Navigate History
	else if( CoreConsole::history.count() > 0 )
	{
		if( Keyboard::check_pressed( vk_up ) )
		{
			if( historyIndex == USIZE_MAX )
			{
				CoreConsole::input->cstr( inputCache, sizeof( inputCache ) );
				historyIndex = CoreConsole::history.count() - 1;
			}
			else if( historyIndex > 0 )
			{
				historyIndex--;
			}

			Console::set_input( historyIndex == USIZE_MAX ?
				inputCache : CoreConsole::history[historyIndex].string );
		}
		else if( Keyboard::check_pressed( vk_down ) )
		{
			historyIndex = historyIndex < CoreConsole::history.count() - 1 ? historyIndex + 1 : USIZE_MAX;
			Console::set_input( historyIndex == USIZE_MAX ?
				inputCache : CoreConsole::history[historyIndex].string );
		}
	}

	// Release Shift
	if( Keyboard::check_released( vk_shift ) )
	{
		candidateIndex = USIZE_MAX;
		logIndex = USIZE_MAX;
		CoreConsole::updateCandidates = true;
	}

	// Tab
	if( Keyboard::check_pressed( vk_tab ) && CoreConsole::candidates_count() > 0 )
	{
		const Command &command = CoreConsole::get_candidate_command( 0 );
		command.set_as_input();
	}

	// Candidate Navigation Scrolling
	if( candidateIndex != USIZE_MAX )
	{
		const int positionTop = 0 + candidateIndex * 24;
		const int positionBottom = 4 + candidateIndex * 24 + 24;
		const int viewTop = 0 - candidateRegion.offset.y;
		const int viewBottom = candidateRegion.dimensionsView.y - candidateRegion.offset.y;

		if( positionTop < viewTop )
		{
			candidateRegion.set_position_pixels( positionTop, true );
		}
		else if( positionBottom > viewBottom )
		{
			candidateRegion.set_position_pixels( positionBottom - candidateRegion.dimensionsView.y, true );
		}
	}
	// Log Navigation Scrolling
	else if( logIndex != USIZE_MAX )
	{
		const int positionTop = 0 + ( CoreConsole::log.count() - 1 - logIndex  ) * 20;
		const int positionBottom = 2 + ( CoreConsole::log.count() - 1 - logIndex ) * 20 + 20;
		const int viewTop = 0 - logRegion.offset.y;
		const int viewBottom = logRegion.dimensionsView.y - logRegion.offset.y;

		if( positionTop < viewTop )
		{
			logRegion.set_position_pixels( positionTop, true );
		}
		else if( positionBottom > viewBottom )
		{
			logRegion.set_position_pixels( positionBottom - logRegion.dimensionsView.y, true );
		}
	}
}


void CoreConsole::draw_input( const Delta delta )
{
	TextEditor &input = CoreConsole::input;

	// Background
	const int bX1 = 4;
	const int bY1 = 4;
	const int bX2 = Window::width - 4;
	const int bY2 = bY1 + 32;
	draw_rectangle( bX1, bY1, bX2, bY2, COLOR_PANELS );

	// Region
	Region &region = CoreConsole::inputRegion;
	region.scrollbarVisible = u8_v2 { false, false };
	const int rX1 = 20;
	const int rY1 = 4;
	const int rX2 = Window::width - 8;
	const int rY2 = rY1 + 32;
	region.dimensionsView = int_v2 { rX2 - rX1, rY2 - rY1 };
	if( CoreText::ACTIVE_TEXT_EDITOR == &CoreConsole::input )
	{
		const int caretX = input->get_position_from_index( input.caret_get_position() ).x;
		if( caretX + 2 < ( 0 - region.offset.x ) )
		{
			region.set_position_pixels( caretX + 2, false );
		}
		else if( caretX + 6 > ( region.dimensionsView.x - region.offset.x ) )
		{
			region.set_position_pixels( caretX + 6 - region.dimensionsView.x, false );
		}
	}
	region.draw( delta, rX1, rY1 );

	// >
	const Color arrowColor = color_mix( COLOR_COMMAND, c_black, 0.5f );
	draw_text( fnt_iosevka, 16, 8, 14, arrowColor, ">" );

	// Input
	Gfx::scissor_set_nested( rX1, rY1, rX2, rY2 );
	{
		// Hints
		if( input->length() > 0 && CoreConsole::tokens.count() > 0 )
		{
			// Find candidate command
			Command *command = nullptr;
			for( CommandCompare i = CommandCompare_Exact; i <= CommandCompare_Partial; i++ )
			{
				if( CoreConsole::candidates[i].count() == 0 ) { continue; }
				command = &CoreConsole::commands[CoreConsole::candidates[i][0]];
				Assert( command != nullptr );
				break;
			}

			// Draw Hints
			const bool navigating = CoreConsole::historyIndex != USIZE_MAX || CoreConsole::candidateIndex != USIZE_MAX;
			if( command != nullptr && !navigating )
			{
				input->cstr( CoreConsole::buffer, sizeof( CoreConsole::buffer ) );
				const int widthInput = text_dimensions_f( fnt_iosevka.bold(), 16, CoreConsole::buffer ).x;
				const int widthSpace = text_dimensions( fnt_iosevka, 16, " " ).x;
				int xOffset = input.screenX + widthInput + 1;
				int yOffset = input.screenY + 2;

				const bool skipParam = CoreConsole::tokens.count() <= static_cast<usize>( command->numRequiredTokens );
				if( skipParam && input[input->length() - 1].codepoint == ' ' ) { xOffset -= widthSpace; } else
				if( !skipParam && input[input->length() - 1].codepoint != ' ' ) { xOffset += widthSpace; }

				for( usize i = CoreConsole::tokens.count(); i < command->tokens.count() + skipParam; i++ )
				{
					// Command Tokens
					if( i <= static_cast<usize>( command->numRequiredTokens ) )
					{
						const Token &token = command->tokens[i - skipParam];

						const int offset = i == CoreConsole::tokens.count() ?
							CoreConsole::tokens[CoreConsole::tokens.count() - skipParam].length : 0;

						if( offset <= token.length )
						{
							const char *hint = &token.string[i == CoreConsole::tokens.count() ? offset : 0];
							draw_text( fnt_iosevka, 16, xOffset, yOffset, COLOR_HINT_COMMAND, hint );
							xOffset += text_dimensions( fnt_iosevka, 16, hint ).x + widthSpace;
						}
					}
					// Parameter Tokens
					else
					{
						const Token &token = command->tokens[i - skipParam];

						const Color color = token.string[0] == PARAMETER_CHAR_REQUIRED ?
							COLOR_HINT_PARAMETER_REQUIRED : COLOR_HINT_PARAMETER_OPTIONAL;
						draw_text( fnt_iosevka, 16, xOffset, yOffset, color, token.string );
						xOffset += text_dimensions( fnt_iosevka, 16, token.string ).x + widthSpace;
					}
				}
			}
		}

		// TextEditor
		input.screenX = rX1 + 2 + region.offset.x;
		input.screenY = rY1 + 8;
		const int_v2 dimensions = input.draw( delta, input.screenX, input.screenY );
		region.dimensionsArea.x = max( dimensions.x + 4, region.dimensionsView.x );
		region.dimensionsArea.y = region.dimensionsView.y;
	}
	Gfx::scissor_reset();
}


void CoreConsole::draw_log( const Delta delta )
{
	const int mX = mouse_x;
	const int mY = mouse_y;

	// Background
	const int bX1 = 4;
	const int bY1 = 40;
	const int bX2 = Window::width - 4;
	const int bY2 = Window::height - 64;
	draw_rectangle( bX1, bY1, bX2, bY2, Color { 0, 0, 0, static_cast<u8>( Console::logAlpha * 255.0f ) } );

	// Region
	Region &region = CoreConsole::logRegion;
	const int rX1 = bX1;
	const int rY1 = bY1;
	const int rX2 = bX2 - Region::scrollbarThickness;
	const int rY2 = bY2;
	region.dimensionsView = int_v2 { rX2 - rX1, rY2 - rY1 };
	region.draw( delta, rX1, rY1 );

	// Draw Log
	Gfx::scissor_set_nested( rX1, rY1, rX2, rY2 );
	{
		// Lines
		int_v2 dimensions = { 0, 0 };

		int yOffset = 0;
		for( usize i = CoreConsole::log.count(); i > 0; i-- )
		{
			LogLine &line = CoreConsole::log[i - 1];
			int xOffset = 0;
			const int tX = rX1 + region.offset.x + 4;
			const int tY = rY1 + region.offset.y + 4;

			// Highlight
			const bool navigating = CoreConsole::logIndex == ( i - 1 );
			if( line.command[0] != '\0' )
			{
				const int hX1 = rX1 + region.offset.x;
				const int hY1 = rY1 + region.offset.y + yOffset;
				const int hX2 = rX2;
				const int hY2 = rY1 + region.offset.y + yOffset + 20;

				const bool hovering = region.hover && point_in_rect( mX, mY, hX1, hY1 + 1, hX2, hY2 - 1 );
				const float tweenTo = hovering || navigating ? 1.0f : 0.0f;
				tween( line.tween, tweenTo, 0.05f, delta, 0.01f );

				if( line.tween > 0.0f )
				{
					const u8 alpha = static_cast<u8>( line.tween * 0.15f * 255.0f );
					draw_rectangle( hX1, hY1, hX2, hY2, Color { 255, 255, 255, alpha } );
				}

				// Draw navigation '>'
				if( navigating )
				{
					draw_text( fnt_iosevka, 14, tX + xOffset, tY + yOffset, COLOR_NAVIGATE, ">" );
					xOffset += 16;
				}

				// Click
				if( hovering && Mouse::check_pressed( mb_left ) )
				{
					if( Keyboard::check( vk_control ) )
					{
						Console::command_execute( line.command );
						return;
					}
					else
					{
						Console::set_input( line.command );
					}

					Mouse::clear();
				}
			}

			// Draw Text
			const Color color = color_mix( line.color, COLOR_NAVIGATE, navigating );
			const int_v2 lineDimensions = text_dimensions( fnt_iosevka, 14, line.string );
			draw_rectangle( tX + xOffset - 2, tY + yOffset - 2,
				tX + xOffset + lineDimensions.x + 2, tY + yOffset + 16,
				Color { 0, 0, 0, 150 } );
			draw_text( fnt_iosevka, 14, tX + xOffset, tY + yOffset, color, line.string );
			dimensions.x = max( dimensions.x, lineDimensions.x );
			dimensions.y += 20; yOffset += 20;
		}

		// Update Region
		region.dimensionsArea.x = max( dimensions.x + 6, region.dimensionsArea.x );
		region.dimensionsArea.y = max( dimensions.y + 2, region.dimensionsArea.y );
	}
	Gfx::scissor_reset();
}


void CoreConsole::draw_candidates( const Delta delta )
{
	if( CoreConsole::candidates_count() == 0 ) { return; }
	if( CoreConsole::hideCandidates ) { return; }
	const usize parameterHintCount = CoreConsole::candidates_parameters_count();
	const int mX = mouse_x;
	const int mY = mouse_y;
	const int widthSpace = text_dimensions( fnt_iosevka, 14, " " ).x;

	// Background
	const int bX1 = 16;
	const int bY1 = 44;
	const int bX2 = bX1 + ( Window::width - bX1 * 2 ) - 16;
	const int bY2 = bY1 + 4 + CoreConsole::candidates_count_view() * 24;
	draw_rectangle( bX1, bY1, bX2, bY2, COLOR_PANELS );

	// Region
	Region &region = CoreConsole::candidateRegion;
	const int rX1 = bX1;
	const int rY1 = bY1;
	const int rX2 = bX2;
	const int rY2 = bY2;
	region.dimensionsView = int_v2 { rX2 - rX1, rY2 - rY1 };
	if( region.dirty || Window::resized ) { region.dimensionsArea = region.dimensionsView; }
	region.draw( delta, rX1, rY1 );

	int yOffset = 0;
	Gfx::scissor_set_nested( rX1, rY1, rX2, rY2 );

	// Candidates
	for( CommandCompare i = 0, j = 0; i <= CommandCompare_Contains; i++ )
	{
		for( const usize &candidate : CoreConsole::candidates[i] )
		{
			Assert( candidate < CoreConsole::commands.count() );
			Command &command = CoreConsole::commands[candidate];

			int xOffset = 0;
			const int tX = bX1 + 8 + region.offset.x;
			const int tY = bY1 + 8 + yOffset + region.offset.y;

			const bool navigating = CoreConsole::candidateIndex == static_cast<usize>( j++ );
			const Color colorCommand = color_mix( COLOR_COMMAND, COLOR_NAVIGATE, navigating );
			const Color colorRequired = color_mix( COLOR_HINT_PARAMETER_REQUIRED, COLOR_NAVIGATE, navigating );
			const Color colorOptional = color_mix( COLOR_HINT_PARAMETER_OPTIONAL, COLOR_NAVIGATE, navigating );

			// Highlight
			const int hX1 = bX1 + region.offset.x;
			const int hY1 = bY1 + 2 + yOffset + region.offset.y;
			const int hX2 = bX2;
			const int hY2 = bY1 + 26 + yOffset + region.offset.y;
			const bool hovering = region.hover && point_in_rect( mX, mY, hX1, hY1 + 1, hX2, hY2 - 1 );

			tween( command.tween, hovering || navigating ? 1.0f : 0.0f, 0.05f, delta, 0.01f );
			if( command.tween > 0.0f )
			{
				const u8 alpha = static_cast<u8>( command.tween * 0.15f * 255.0f );
				draw_rectangle( hX1, hY1, hX2, hY2, Color { 255, 255, 255, alpha } );
			}

			// Click
			if( hovering && Mouse::check_pressed( mb_left ) )
			{
				command.set_as_input();
				if( Keyboard::check( vk_control ) )
				{
					CoreConsole::update_input();
					Console::command_execute();
					CoreConsole::input.clear();
					return;
				}
				Mouse::clear();
			}

			// Draw navigation '>'
			if( navigating )
			{
				draw_text( fnt_iosevka, 15, tX + xOffset, tY, COLOR_NAVIGATE, ">" );
				xOffset += 16;
			}

			// Draw Command
			for( usize t = 0; t < command.tokens.count(); t++ )
			{
				const Token &token = command.tokens[t];
				const bool isParamRequired = ( token.string[0] == PARAMETER_CHAR_REQUIRED );
				const bool isParamOptional = ( token.string[0] == PARAMETER_CHAR_OPTIONAL );
				const bool isCommand = !( isParamRequired || isParamOptional );
				if( parameterHintCount && !isCommand ) { xOffset -= widthSpace; break; }
				const Color color = isCommand ? colorCommand : ( isParamRequired ? colorRequired : colorOptional );
				draw_text( fnt_iosevka, 15, tX + xOffset, tY, color, token.string );
				xOffset += text_dimensions( fnt_iosevka, 15, token.string ).x;
				xOffset += ( t < command.tokens.count() - 1 ) * widthSpace;
			}

			// Draw Description
			if( command.description[0] != '\0' )
			{
				draw_text_f( fnt_iosevka, 15, tX + xOffset, tY, COLOR_DESC_COMMAND, " - %s", command.description );
				xOffset += text_dimensions_f( fnt_iosevka, 15, " - %s", command.description ).x;
			}

			yOffset += 24;
			region.dimensionsArea.x = max( xOffset + 16, region.dimensionsArea.x );
		}
	}

	// Parameter Hints
	for( usize i = 0; i < parameterHintCount; i++ )
	{
		const Command &command = CoreConsole::get_candidate_command( 0 );
		const Token &token = command.tokens[command.numRequiredTokens + i];
		const bool isParamRequired = ( token.string[0] == PARAMETER_CHAR_REQUIRED );
		const int tX = bX1 + 8 + region.offset.x;
		const int tY = bY1 + 8 + yOffset + region.offset.y;
		int xOffset = 12;

		// Draw Parameter
		const Color paramColor = isParamRequired ? COLOR_HINT_PARAMETER_REQUIRED : COLOR_HINT_PARAMETER_OPTIONAL;
		draw_text( fnt_iosevka, 15, tX + xOffset, tY, paramColor, token.string );
		xOffset += text_dimensions( fnt_iosevka, 15, token.string ).x;

		// Draw Description
		if( command.paramDesc[i][0] != '\0' )
		{
			draw_text_f( fnt_iosevka, 15, tX + xOffset, tY, COLOR_DESC_PARAM, " - %s", command.paramDesc[i] );
			xOffset += text_dimensions_f( fnt_iosevka, 15, " - %s", command.paramDesc[i] ).x;
		}

		yOffset += 24;
		region.dimensionsArea.x = max( xOffset + 20, region.dimensionsArea.x );
	}

	region.dimensionsArea.y = yOffset + 4;
	region.dimensionsView = int_v2 { rX2 - rX1, rY2 - rY1 };
	region.dirty = false;
	Gfx::scissor_reset();
}


void CoreConsole::Log( const Color color, const char *command, const char *message )
{
	// Reset log seek position
	CoreConsole::logIndex = USIZE_MAX;

	// Add line
	if( CoreConsole::log.count() >= LOG_MAX ) { CoreConsole::log.remove( 0 ); }
	CoreConsole::log.add( LogLine { color, command, message } );
	CoreConsole::logRegion.set_position_percent( 0.0f, true );
	CoreConsole::logRegion.set_position_percent( 0.0f, false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CoreConsole::DVarInitializer &CoreConsole::DVarInitializer::get_instance()
{
	// DVars may be initialized before Console::init() -- to handle this, we defer command registration via this
	// DVarInitializer class.

	// A singleton pattern is used here to ensure the arrays above are always in a valid state, regardless of the
	// order or translation unit they are accessed from (necessary for DVars in static scope across multiple
	// translation units).

	static CoreConsole::DVarInitializer initializer;
	return initializer;
}


CoreConsole::DVarInitializer &CoreConsole::DVarInitializer::reset_instance()
{
	CoreConsole::DVarInitializer &initializer = CoreConsole::DVarInitializer::get_instance();
	initializer.current = 0LLU;
	return initializer;
}


void CoreConsole::DVarInitializer::dvar_register( const char *definition, const char *parameter,
	const char *description, void *payload, CONSOLE_COMMAND_FUNCTION_POINTER( function ) )
{
	if( payload == nullptr ) { return; }
	CoreConsole::DVarInitializer &initializer = CoreConsole::DVarInitializer::get_instance();

	// Defer initialization until a later point; store our parameters
	if( initializer.defer )
	{
		AssertMsg( initializer.current < STATIC_DVARS_MAX, "DVarInitializer: exceeded STATIC_DVARS_MAX!" );
		initializer.dvars[initializer.current].definition = definition;
		initializer.dvars[initializer.current].parameters = parameter;
		initializer.dvars[initializer.current].description = description;
		initializer.dvars[initializer.current].payload = payload;
		initializer.dvars[initializer.current].function = function;
		initializer.dvars[initializer.current].hidden = false;
		initializer.dvars[initializer.current].disabled = false;
		initializer.current++;
	}
	// Initialize DVar
	else
	{
		CoreConsole::buffer[0] = '\0';
		snprintf( CoreConsole::buffer, sizeof( CoreConsole::buffer ), "dv %s %s", definition, parameter );
		CommandHandle handle = Console::command_init( CoreConsole::buffer, description, function, payload );
	}
}


void CoreConsole::DVarInitializer::dvar_release( void *payload )
{
	if( payload == nullptr ) { return; }
	CoreConsole::DVarInitializer &initializer = CoreConsole::DVarInitializer::get_instance();

	if( initializer.defer )
	{
		// Remove the DVar from the deferred list (if it exists)
		for( usize i = 0; i < initializer.current; i++ )
		{
			if( initializer.dvars[i].payload == payload )
			{
				// Swap with final element
				if( i > 0 )
				{
					initializer.dvars[i].definition = initializer.dvars[initializer.current - 1].definition;
					initializer.dvars[i].parameters = initializer.dvars[initializer.current - 1].parameters;
					initializer.dvars[i].description = initializer.dvars[initializer.current - 1].description;
					initializer.dvars[i].payload = initializer.dvars[initializer.current - 1].payload;
					initializer.dvars[i].function = initializer.dvars[initializer.current - 1].function;
					initializer.dvars[i].hidden = initializer.dvars[initializer.current - 1].hidden;
					initializer.dvars[i].disabled = initializer.dvars[initializer.current - 1].disabled;
				}

				// Decerement list size & return
				initializer.current--;
				return;
			}
		}
	}
	else
	{
		// Release command from Console
		for( usize i = 0; i < CoreConsole::commands.count(); i++ )
		{
			const Command &command = CoreConsole::commands[i];
			if( command.payload == payload )
			{
				Console::command_free( i );
				return;
			}
		}
	}
}


void CoreConsole::DVarInitializer::dvar_set_hidden( void *payload, const bool hidden )
{
	if( payload == nullptr ) { return; }
	CoreConsole::DVarInitializer &initializer = CoreConsole::DVarInitializer::get_instance();

	if( initializer.defer )
	{
		// Remove the DVar from the deferred list (if it exists)
		for( usize i = 0; i < initializer.current; i++ )
		{
			if( initializer.dvars[i].payload == payload )
			{
				initializer.dvars[i].hidden = hidden;
				return;
			}
		}
	}
	else
	{
		// Release command from Console
		for( usize i = 0; i < CoreConsole::commands.count(); i++ )
		{
			const Command &command = CoreConsole::commands[i];
			if( command.payload == payload )
			{
				Console::command_set_hidden( i, hidden );
				return;
			}
		}
	}
}


void CoreConsole::DVarInitializer::dvar_set_enabled( void *payload, const bool enabled )
{
	if( payload == nullptr ) { return; }
	CoreConsole::DVarInitializer &initializer = CoreConsole::DVarInitializer::get_instance();

	if( initializer.defer )
	{
		// Remove the DVar from the deferred list (if it exists)
		for( usize i = 0; i < initializer.current; i++ )
		{
			if( initializer.dvars[i].payload == payload )
			{
				initializer.dvars[i].disabled = !enabled;
				return;
			}
		}
	}
	else
	{
		// Release command from Console
		for( usize i = 0; i < CoreConsole::commands.count(); i++ )
		{
			const Command &command = CoreConsole::commands[i];
			if( command.payload == payload )
			{
				Console::command_set_enabled( i, enabled );
				return;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dvar_color_picker_window( Color *variable )
{
	static ObjectInstance window;
	static ObjectInstance spectrum;
	static ObjectInstance textboxes;
	static Color *color;
	color = variable;

	// Log
	char buffer[COMMAND_SIZE];
	snprintf( buffer, sizeof( buffer ), "Color picker opened for %p", variable );
	Console::Log( c_white, buffer );

	// Destroy Existing window
	ui.destroy_widget( window );

	// Create New Window
	window = ui.create_widget( Object::UIWidget_Window );
	auto windowHandle = ui.handle<Object::UIWidget_Window>( window );
	if( windowHandle )
	{
		windowHandle->x = 64;
		windowHandle->y = 64;
		windowHandle->width = 264;
		windowHandle->height = 320;
		windowHandle->set_min_size( 264, COMMAND_SIZE );
		windowHandle->color = c_dkgray;
	}

	spectrum = ui.create_widget( Object::UIWidget_ColorPicker_Spectrum, window );
	auto spectrumHandle = ui.handle<Object::UIWidget_ColorPicker_Spectrum>( spectrum );
	Assert( spectrumHandle );
	if( spectrumHandle )
	{
		spectrumHandle->set_rgba( color != nullptr ? *color : c_teal );

		spectrumHandle->callbackOnUpdate = []( UIContext &context, ObjectInstance widget )
			{
				auto widgetHandle = context.handle<Object::UIWidget_ColorPicker_Spectrum>( widget );
				Assert( widgetHandle );
				auto parentHandle = context.handle<Object::UIWidget_Window>( widgetHandle->parent );
				Assert( parentHandle );

				widgetHandle->x = 4;
				widgetHandle->y = 4;
				widgetHandle->width = parentHandle->width - 8;
				widgetHandle->height = parentHandle->height - 80;
			};

		spectrumHandle->callbackOnInteract = []( UIContext &context, ObjectInstance widget )
			{
				auto textboxesHandle = context.handle<Object::UIWidget_ColorPicker_TextBox>( textboxes );
				Assert( textboxesHandle );
				auto spectrumHandle = context.handle<Object::UIWidget_ColorPicker_Spectrum>( spectrum );
				Assert( spectrumHandle );

				const Color c = spectrumHandle->get_rgba();
				textboxesHandle->set_rgba( c );
				if( color != nullptr ) { *color = c; }
			};
	}

	textboxes = ui.create_widget( Object::UIWidget_ColorPicker_TextBox, window );
	auto textboxesHandle = ui.handle<Object::UIWidget_ColorPicker_TextBox>( textboxes );
	Assert( spectrumHandle );
	if( textboxesHandle )
	{
		textboxesHandle->set_rgba( c_teal );

		textboxesHandle->callbackOnUpdate = []( UIContext &context, ObjectInstance widget )
			{
				auto widgetHandle = context.handle<Object::UIWidget_ColorPicker_TextBox>( widget );
				Assert( widgetHandle );
				auto parentHandle = context.handle<Object::UIWidget_Window>( widgetHandle->parent );
				Assert( parentHandle );

				widgetHandle->x = 4;
				widgetHandle->y = parentHandle->height - 68;
			};

		textboxesHandle->callbackOnInteract = []( UIContext &context, ObjectInstance widget )
			{
				auto textboxesHandle = context.handle<Object::UIWidget_ColorPicker_TextBox>( textboxes );
				Assert( textboxesHandle );
				auto spectrumHandle = context.handle<Object::UIWidget_ColorPicker_Spectrum>( spectrum );
				Assert( spectrumHandle );

				const Color c = textboxesHandle->get_rgba();
				spectrumHandle->set_rgba( c );
				if( color != nullptr ) { *color = c; }
			};
	}
}


static void dvar_render_target_window( GfxRenderTarget *variable )
{
	// Log
	char buffer[COMMAND_SIZE];
	snprintf( buffer, sizeof( buffer ), "Render target inspector opened for %p", variable );
	Console::Log( c_white, buffer );

	// Find name
	const char *name = "GfxRenderTarget";
	for( Command &command : CoreConsole::commands )
	{
		if( command.payload == reinterpret_cast<void *>( variable ) )
		{
			name = &command.string[3];
		}
	}

	// Window
	ObjectInstance window = ui.create_widget( Object::UIWidget_Window_GfxRenderTarget );
	auto windowHandle = ui.handle<Object::UIWidget_Window_GfxRenderTarget>( window );
	if( windowHandle )
	{
		windowHandle->x = 64;
		windowHandle->y = 64;
		windowHandle->color = c_dkgray;
		windowHandle->name = name;

		if( variable != nullptr )
		{
			windowHandle->rt = variable;
			windowHandle->width = variable->width;
			windowHandle->height = variable->height;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CoreConsole::DVar::~DVar()
{
	if( payload == nullptr ) { return; }
	DVarInitializer::dvar_release( payload );
}


CoreConsole::DVar::DVar( bool scoped, int *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "[value]", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			int &v = *reinterpret_cast<int *>( payload );
			v = Console::get_parameter_int( 0, v );
		} );
}


CoreConsole::DVar::DVar( bool scoped, u32 *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "[value]", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			u32 &v = *reinterpret_cast<u32 *>( payload );
			v = Console::get_parameter_u32( 0, v );
		} );
}


CoreConsole::DVar::DVar( bool scoped, u64 *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "[value]", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			u32 &v = *reinterpret_cast<u32 *>( payload );
			v = Console::get_parameter_u64( 0, v );
		} );
}


CoreConsole::DVar::DVar( bool scoped, double *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "[value]", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			double &v = *reinterpret_cast<double *>( payload );
			v = Console::get_parameter_double( 0, v );
		} );
}


CoreConsole::DVar::DVar( bool scoped, bool *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "[value]", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			bool &v = *reinterpret_cast<bool *>( payload );
			v = Console::get_parameter_bool( 0, v );
		} );
}


CoreConsole::DVar::DVar( bool scoped, float *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "[value]", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			float &v = *reinterpret_cast<float *>( payload );
			v = Console::get_parameter_float( 0, v );
		} );
}


CoreConsole::DVar::DVar( bool scoped, Color *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "<r> <g> <b> <a>", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			if( Console::get_parameter_count() == 0 )
			{
				dvar_color_picker_window( reinterpret_cast<Color *>( payload ) );
			}
			else
			{
				Color &color = *reinterpret_cast<Color *>( payload );
				color.r = static_cast<u8>( clamp( Console::get_parameter_int( 0, color.r ), 0, 255 ) );
				color.g = static_cast<u8>( clamp( Console::get_parameter_int( 1, color.g ), 0, 255 ) );
				color.b = static_cast<u8>( clamp( Console::get_parameter_int( 2, color.b ), 0, 255 ) );
				color.a = static_cast<u8>( clamp( Console::get_parameter_int( 3, color.a ), 0, 255 ) );
			}
		} );
}


CoreConsole::DVar::DVar( bool scoped, GfxRenderTarget *variable, const char *definition, const char *description )
{
	Assert( variable != nullptr );
	payload = scoped ? reinterpret_cast<void *>( variable ) : nullptr;

	CoreConsole::DVarInitializer::dvar_register( definition, "", description, variable,
		CONSOLE_COMMAND_LAMBDA
		{
			Assert( payload != nullptr );
			dvar_render_target_window( reinterpret_cast<GfxRenderTarget *>( payload ) );
		} );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CommandHandle Console::command_init( const char *definition, const char *description,
	CONSOLE_COMMAND_FUNCTION_POINTER( function ), void *payload )
{
	Assert( CoreConsole::initialized );

	// Hide Duplicate Commands
	bool hidden = false;
	for( const Command &command : CoreConsole::commands )
	{
		if( strncmp( definition, command.string, sizeof( command.string ) ) == 0 )
		{
			hidden = true;
			break;
		}
	}

	// Register Command
	Command &command = CoreConsole::commands.add( Command { } );
	command.init( definition, description, function, payload );
	command.handle = CoreConsole::handleCurrent++;
	command.hidden = hidden;
	for( int i = 0; i < PARAMETERS_MAX; i++ ) { command.paramDesc[i] = ""; }
	CoreConsole::updateCandidates = true;
	return command.handle;
}


void Console::command_param_description( const CommandHandle command, const int param, const char *description )
{
	Assert( CoreConsole::initialized );
	Command *cmd = CoreConsole::command( command );
	Assert( cmd != nullptr );
	Assert( param < cmd->numParams );
	cmd->paramDesc[param] = description;
}


void Console::command_free( const CommandHandle command )
{
	Assert( CoreConsole::initialized );
	for( usize i = 0; i < CoreConsole::commands.count(); i++ )
	{
		Command &cmd = CoreConsole::commands[i];
		if( cmd.handle != command ) { continue; }
		cmd.free();
		CoreConsole::commands.remove( i );
		CoreConsole::updateCandidates = true;
	}
}


void Console::command_set_hidden( const CommandHandle command, const bool hidden )
{
	Assert( CoreConsole::initialized );
	Command *cmd = CoreConsole::command( command );
	Assert( cmd != nullptr );
	cmd->hidden = hidden;
	CoreConsole::updateCandidates = true;
}


void Console::command_set_enabled( const CommandHandle command, const bool enabled )
{
	Assert( CoreConsole::initialized );
	Command *cmd = CoreConsole::command( command );
	Assert( cmd != nullptr );
	cmd->disabled = !enabled;
	CoreConsole::updateCandidates = true;
}


void Console::draw( const Delta delta )
{
	Region &inputRegion = CoreConsole::inputRegion;
	Region &candidateRegion = CoreConsole::candidateRegion;
	Region &logRegion = CoreConsole::logRegion;

	if( !Console::is_open() )
	{
		// Open Console
		if( Keyboard::check_pressed( vk_tilde ) )
		{
			Console::open();
			Keyboard::clear();
		}
	}

	if( Console::is_open() )
	{
		// Activate Keyboard & Mouse
		Keyboard::set_active( keyboard );
		Mouse::set_active( mouse );
		const int mX = mouse_x;
		const int mY = mouse_y;

		// Activate TextEditor
		CoreConsole::input.activate();

		// Navigate Console
		CoreConsole::navigate();

		// Process Input Updates
		CoreConsole::update_input();

		// Update Candidates
		if( CoreConsole::updateCandidates )
		{
			CoreConsole::update_candidates( false );
			CoreConsole::updateCandidates = false;
		}

		// Input Hover
		{
			const int bX1 = 4;
			const int bY1 = 4;
			const int bX2 = Window::width - 4;
			const int bY2 = bY1 + 32;

			const bool canHover = !( ui.get_widget_hover() );
			inputRegion.hover = point_in_rect( mX, mY, bX1, bY1, bX2, bY2 );
			CoreConsole::input.allowClick = inputRegion.hover;
		}

		// Candidates Hover
		bool hoverCandidates = false;
		if( CoreConsole::hideCandidates || CoreConsole::candidates_count() == 0 )
		{
			candidateRegion.hover = false;
		}
		else
		{
			const int bX1 = 16;
			const int bY1 = 44;
			const int bX2 = bX1 + ( Window::width - bX1 * 2 ) - 16;
			const int bY2 = bY1 + 4 + CoreConsole::candidates_count_view() * 24;

			const bool canHover = !( !Mouse::check_pressed( mb_left ) && Mouse::check( mb_left ) ) &&
				!( ui.get_widget_hover() );
			candidateRegion.hover = canHover && point_in_rect( mX, mY, bX1, bY1, bX2, bY2 );

			const int hasScrollbarV = ( candidateRegion.dimensionsArea.y > candidateRegion.dimensionsView.y );
			const int hasScrollbarH = ( candidateRegion.dimensionsArea.x > candidateRegion.dimensionsView.x );
			constexpr int thickness = Region::scrollbarThickness;
			hoverCandidates = canHover && point_in_rect( mX, mY, bX1, bY1,
				bX2 + ( hasScrollbarV * thickness ), bY2 + ( hasScrollbarH * thickness ) );
		}

		// Log Hover
		if( hoverCandidates )
		{
			logRegion.hover = false;
		}
		else
		{
			const int bX1 = 4;
			const int bY1 = 40;
			const int bX2 = Window::width - 4;
			const int bY2 = Window::height - 64;
			const bool canHover = !( !Mouse::check_pressed( mb_left ) && Mouse::check( mb_left ) ) &&
				!( ui.get_widget_hover() );
			logRegion.hover = point_in_rect( mX, mY, bX1, bY1, bX2, bY2 ) && canHover;
		}

		// Mouse Wheel
		if( Mouse::check_wheel_up() )
		{
			if( hoverCandidates ) { candidateRegion.scroll_pixels( -24, true ); }
			if( logRegion.hover ) { logRegion.scroll_pixels( -64, true ); }
		}
		else if( Mouse::check_wheel_down() )
		{
			if( hoverCandidates ) { candidateRegion.scroll_pixels( 24, true ); }
			if( logRegion.hover ) { logRegion.scroll_pixels( 64, true ); }
		}

		// Page Up / Page Down
		if( Keyboard::check_pressed( vk_pageup ) )
		{
			if( hoverCandidates ) { candidateRegion.scroll_pixels( -candidateRegion.dimensionsView.y, true ); }
			if( logRegion.hover ) { logRegion.scroll_pixels( -logRegion.dimensionsView.y, true ); }
		}
		else if( Keyboard::check_pressed( vk_pagedown ) )
		{
			if( hoverCandidates ) { candidateRegion.scroll_pixels( candidateRegion.dimensionsView.y, true ); }
			if( logRegion.hover ) { logRegion.scroll_pixels( logRegion.dimensionsView.y, true ); }
		}

		// Home / End
		if( Keyboard::check_pressed( vk_home ) )
		{
			if( hoverCandidates ) { candidateRegion.set_position_percent( 0.0f, true ); }
			if( logRegion.hover ) { logRegion.set_position_percent( 0.0f, true ); }
		}
		else if( Keyboard::check_pressed( vk_end ) )
		{
			if( hoverCandidates ) { candidateRegion.set_position_percent( 1.0f, true ); }
			if( logRegion.hover ) { logRegion.set_position_percent( 1.0f, true ); }
		}

		// Hide Candidates
		if( Keyboard::check( vk_control ) && Keyboard::check_pressed( vk_h ) )
		{
			CoreConsole::hideCandidates = !CoreConsole::hideCandidates;
		}
	}

	// Draw Console
	if( Console::is_open() )
	{
		// Dim Window
		draw_rectangle( 0, 0, Window::width, Window::height,
			Color { 0, 0, 0, static_cast<u8>( Console::backgroundAlpha * 255.0f ) } );

		// Draw Console Line
		CoreConsole::draw_input( delta );

		// Draw Log
		CoreConsole::draw_log( delta );

		// Draw Candidates
		CoreConsole::draw_candidates( delta );

		// Draw Info
		draw_text( fnt_iosevka, 14, 4, Window::height - 18, c_gray, PROJECT_CAPTION );
		if( CoreConsole::hideCandidates )
		{
			draw_text( fnt_iosevka, 14, 4, Window::height - 38, Color { 200, 0, 0 },
				"suggestions hidden, ctr-h to re-enable" );
		}
	}

	// Draw UIContext
	ui.update_widgets( delta );
	ui.render_widgets( delta );

	if( Console::is_open() )
	{
		// Execute Command
		if( Keyboard::check_pressed( vk_enter ) )
		{
			CoreConsole::input->cstr( CoreConsole::buffer, sizeof( CoreConsole::buffer ) );
			Console::command_execute( CoreConsole::buffer );
			CoreConsole::input.clear();
		}

		// Close Console
		if( Keyboard::check_pressed( vk_tilde ) || Keyboard::check_pressed( vk_escape ) )
		{
			Console::close();
			Keyboard::clear();
		}
	}
}


void Console::set_input( const char *string )
{
	CoreConsole::input.clear();
	CoreConsole::input->append( string );
	CoreConsole::input.caret_set_position_end();
}


bool Console::is_open()
{
	return CoreConsole::open;
}


void Console::open()
{
	// Open Console
	CoreConsole::open = true;
	CoreConsole::updateCandidates = true;

	// Reset TextEditor
	CoreConsole::input.clear();

	// Reset State
	for( LogLine &line : CoreConsole::log ) { line.tween = 0.0f; }
	for( Command &command : CoreConsole::commands) { command.tween = 0.0f; }
	for( int i = 0; i < COMMANDCOMPARE_COUNT; i++ ) { CoreConsole::candidates[i].clear(); }
	CoreConsole::reset_regions();

	// Reset Keyboard
	Keyboard::clear();
}


void Console::close()
{
	CoreConsole::open = false;

	// Deactivate TextEditor
	CoreConsole::input.deactivate();

	// Reset State
	for( int i = 0; i < COMMANDCOMPARE_COUNT; i++ ) { CoreConsole::candidates[i].clear(); }
	CoreConsole::reset_regions();

	// Reset Keyboard
	Keyboard::clear();
}


void Console::toggle()
{
	if( Console::is_open() ) { Console::close(); } else { Console::open(); }
}


static const Command *get_command( const usize index = 0 )
{
	if( CoreConsole::candidates[CommandCompare_Exact].count() <= index ) { return nullptr; }
	const u32 candidate = CoreConsole::candidates[CommandCompare_Exact][index];
	Assert( candidate < CoreConsole::commands.count() );
	return &CoreConsole::commands[candidate];
}


bool Console::command_execute()
{
	CoreConsole::update_candidates( true );
	for( usize i = 0; i < CoreConsole::candidates[CommandCompare_Exact].count(); i++ )
	{
		const Command *const command = get_command( i );
		if( command == nullptr ) { return false; }
		if( command->disabled ) { return false; }

		if( command->function != nullptr )
		{
			command->function( command->payload );
			CoreConsole::updateCandidates = true;
			return true;
		}
	}
	return false;
}


bool Console::command_execute( const char *command )
{
	// Skip empty commands
	if( command[0] == '\0' ) { return false; }

	// Tokenize Custom Command
	CoreConsole::history.add( CommandHistory { command } );
	CoreConsole::historyIndex = USIZE_MAX;
	CoreConsole::tokenize_input( command );

	// Execute Command
	const bool success = Console::command_execute();

	// Reset Tokens & Candidates (if necessary)
	if( Console::is_open() )
	{
		CoreConsole::update_input();
		CoreConsole::update_candidates( false );
	}

	// Success
	if( success ) { return true; }

	// Error
	char buffer[COMMAND_SIZE];
	snprintf( buffer, sizeof( buffer ), "Unknown command: '%s'", command );
	Console::Log( c_red, buffer );
	return false;
}


bool Console::command_execute( const CommandHandle command, const char *parameters )
{
	Assert( command < CoreConsole::commands.count() );
	Command *cmd = CoreConsole::command( command );
	Assert( cmd != nullptr );

	// Command Tokens
	CoreConsole::buffer[0] = '\0';
	for( int i = 0; i <= cmd->numRequiredParams; i++ )
	{
		strappend( CoreConsole::buffer, cmd->tokens[i].string );
		strappend( CoreConsole::buffer, " " );
	}

	// Parameters
	strappend( CoreConsole::buffer, parameters );

	// Execute command
	return Console::command_execute( CoreConsole::buffer );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Console::clear_log()
{
	// Reset log seek position
	CoreConsole::logIndex = USIZE_MAX;

	// Clear log
	CoreConsole::log.clear();
}


void Console::dump_log( const char *path )
{
	String dump;
	for( const LogLine &line : CoreConsole::log ) { dump.append( line.string ).append( "\n" ); }
	dump.save( path );
}


void Console::Log( const Color color, const char *message, ... )
{
	va_list args;
	va_start( args, message );
	char buffer[1024];
	vsnprintf( buffer, sizeof( buffer ), message, args );
	va_end( args );

	CoreConsole::Log( color, "", buffer );
}


void Console::LogCommand( const Color color, const char *command, const char *message, ... )
{
	va_list args;
	va_start( args, message );
	char buffer[1024];
	vsnprintf( buffer, sizeof( buffer ), message, args );
	va_end( args );

	CoreConsole::Log( color, command, buffer );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool has_parameter( const Command *const command, const int param )
{
	Assert( param >= 0 );
	return ( CoreConsole::tokens.count() > PARAMETER_INDEX( command, param ) );
}


usize Console::get_parameter_count()
{
	const Command *const command = get_command();
	Assert( command != nullptr );
	return ( CoreConsole::tokens.count() - PARAMETER_INDEX( command, 0 ) );
}


const char *Console::get_parameter_string( const int param, const char *defaultValue )
{
	const Command *const command = get_command();
	Assert( command != nullptr );
	if( !has_parameter( command, param ) ) { return defaultValue; }
	return CoreConsole::tokens[PARAMETER_INDEX(command, param)].string;
}


int Console::get_parameter_int( const int param, const int defaultValue )
{
	const char *parameter = Console::get_parameter_string( param );
	if( parameter[0] == '\0' ) { return defaultValue; }
	return atoi( parameter );
}


u32 Console::get_parameter_u32( const int param, const u32 defaultValue )
{
	const char *parameter = Console::get_parameter_string( param );
	if( parameter[0] == '\0' ) { return defaultValue; }
	return static_cast<u32>( atoll( parameter ) );
}


u64 Console::get_parameter_u64( const int param, const u64 defaultValue )
{
	const char *parameter = Console::get_parameter_string( param );
	if( parameter[0] == '\0' ) { return defaultValue; }
	return static_cast<u64>( atoll( parameter ) );
}


bool Console::get_parameter_bool( const int param, const bool defaultValue )
{
	const char *parameter = Console::get_parameter_string( param );
	if( parameter[0] == '\0' ) { return defaultValue; }
	if( parameter[0] == 't' || parameter[0] == 'y' ) { return true; }
	if( parameter[0] == 'f' || parameter[0] == 'n' ) { return false; }
	return atoi( parameter );
}


bool Console::get_parameter_toggle( const int param, const bool currentValue )
{
	if( Console::get_parameter_count() <= static_cast<usize>( param ) ) { return !currentValue; }
	const char *parameter = Console::get_parameter_string( param );
	if( parameter[0] == '\0' ) { return currentValue; }
	if( parameter[0] == 't' || parameter[0] == 'y' ) { return true; }
	if( parameter[0] == 'f' || parameter[0] == 'n' ) { return false; }
	return atoi( parameter );
}


float Console::get_parameter_float( const int param, const float defaultValue )
{
	const char *parameter = Console::get_parameter_string( param );
	if( parameter[0] == '\0' ) { return defaultValue; }
	return static_cast<float>( atof( parameter ) );
}


double Console::get_parameter_double( const int param, const double defaultValue )
{
	const char *parameter = Console::get_parameter_string( param );
	if( parameter[0] == '\0' ) { return defaultValue; }
	return atof( parameter );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////