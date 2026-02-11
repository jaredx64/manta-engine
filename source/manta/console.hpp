#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/color.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GfxRenderTarget;
class Command;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using CommandHandle = u32;

#define CONSOLE_COMMAND_LAMBDA []( void *payload )
#define CONSOLE_COMMAND_FUNCTION(name) void name( void *payload )
#define CONSOLE_COMMAND_FUNCTION_POINTER(name) void ( *name )( void * )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreConsole
{
	extern bool init();
	extern bool free();

	extern Command *command( const CommandHandle handle );

	extern void navigate();

	extern void tokenize_input( const char *string );
	extern void update_input();
	extern void update_candidates( bool processHiddenCommands );

	extern void draw_input( Delta delta );
	extern void draw_log( Delta delta );
	extern void draw_candidates( Delta delta );

	extern void Log( Color color, const char *command, const char *message );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GfxRenderTarget;
class RadialColorGraph;
class RadialFloatGraph;

namespace CoreConsole
{
	struct DVarDeferred
	{
		const char *definition;
		const char *parameters;
		const char *description;
		void *payload;
		CONSOLE_COMMAND_FUNCTION_POINTER( function );
		bool hidden;
		bool disabled;
	};

	class DVarInitializer
	{
	public:
		static DVarInitializer &get_instance();
		static DVarInitializer &reset_instance();

		static void dvar_register( const char *definition, const char *parameter, const char *description,
			void *payload, CONSOLE_COMMAND_FUNCTION_POINTER( function ) );
		static void dvar_release( void *payload );
		static void dvar_set_hidden( void *payload, bool hidden );
		static void dvar_set_enabled( void *payload, bool enabled );

	public:
		static constexpr int STATIC_DVARS_MAX = 1024;
		DVarDeferred dvars[STATIC_DVARS_MAX];
		usize current = 0LLU;
		bool defer = true;
	};

	class DVar
	{
	public:
		~DVar();
		DVar( bool scoped, int *variable, const char *definition, const char *description );
		DVar( bool scoped, u32 *variable, const char *definition, const char *description );
		DVar( bool scoped, u64 *variable, const char *definition, const char *description );
		DVar( bool scoped, bool *variable, const char *definition, const char *description );
		DVar( bool scoped, float *variable, const char *definition, const char *description );
		DVar( bool scoped, double *variable, const char *definition, const char *description );
		DVar( bool scoped, Color *variable, const char *definition, const char *description );
		DVar( bool scoped, GfxRenderTarget *variable, const char *definition, const char *description );
		DVar( bool scoped, RadialColorGraph *variable, const char *definition, const char *description );
		DVar( bool scoped, RadialFloatGraph *variable, const char *definition, const char *description );
	public:
		void *payload = nullptr;
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DVAR_RELEASE(variable) \
	CoreConsole::DVarInitializer::dvar_release( &( variable ) )

#define DVAR_HOOK(variable) \
	{ CoreConsole::DVar __dv_ { false, &( variable ), #variable, "" }; }
#define DVAR_HOOK_DESC(variable,description) \
	{ CoreConsole::DVar __dv_ { false, &( variable ), #variable, description }; }
#define DVAR_HOOK_NAME(variable,name) \
	{ CoreConsole::DVar __dv_ { false, &( variable ), name, "" }; }
#define DVAR_HOOK_NAME_DESC(variable,name,description) \
	{ CoreConsole::DVar __dv_ { false, &( variable ), name, description }; }


#define DVAR(variable) \
	CoreConsole::DVar __dv_##variable { true, &( variable ), #variable, "" }
#define DVAR_DESC(variable,description) \
	CoreConsole::DVar __dv_##variable { true, &( variable ), #variable, description }
#define DVAR_NAME(variable,name) \
	CoreConsole::DVar __dv_##variable { true, &( variable ), name, "" }
#define DVAR_NAME_DESC(variable,name,description) \
	CoreConsole::DVar __dv_##variable { true, &( variable ), name, description }

#define DVAR_HIDE(variable) \
	CoreConsole::DVarInitializer::dvar_set_hidden( &( variable ), true )
#define DVAR_UNHIDE(variable) \
	CoreConsole::DVarInitializer::dvar_set_hidden( &( variable ), false )
#define DVAR_SET_HIDDEN(variable,hidden) \
	CoreConsole::DVarInitializer::dvar_set_hidden( &( variable ), hidden )

#define DVAR_ENABLE(variable) \
	CoreConsole::DVarInitializer::dvar_set_enabled( &( variable ), true )
#define DVAR_DISABLE(variable) \
	CoreConsole::DVarInitializer::dvar_set_enabled( &( variable ), false )
#define DVAR_SET_ENABLED(variable,enabled) \
	CoreConsole::DVarInitializer::dvar_set_enabled( &( variable ), enabled )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Console
{
	extern float backgroundAlpha;
	extern float logAlpha;

	extern void draw( const Delta delta );

	extern CommandHandle command_init( const char *definition, const char *description,
		CONSOLE_COMMAND_FUNCTION_POINTER( function ), void *payload = nullptr );

	extern void command_param_description( const CommandHandle command,
		const int param, const char *description );

	extern void command_free( const CommandHandle command );
	extern void command_set_hidden( const CommandHandle command, bool hidden );
	extern void command_set_enabled( const CommandHandle command, bool enabled );

	extern bool command_execute();
	extern bool command_execute( const char *command );
	extern bool command_execute( CommandHandle command, const char *parameters );

	extern void set_input( const char *string );

	extern bool is_open();
	extern void open();
	extern void close();
	extern void toggle();

	extern void clear_log();
	extern void dump_log( const char *path );

	extern void Log( Color color, const char *message, ... );
	extern void LogCommand( Color color, const char *command, const char *message, ... );

	extern usize get_parameter_count();

	extern const char *get_parameter_string( int param, const char *defaultValue = "" );
	extern int get_parameter_int( int param, int defaultValue = 0 );
	extern u32 get_parameter_u32( int param, u32 defaultValue = 0 );
	extern u64 get_parameter_u64( int param, u64 defaultValue = 0 );
	extern bool get_parameter_bool( int param, bool defaultValue = false );
	extern bool get_parameter_toggle( int param, bool currentValue = false );
	extern float get_parameter_float( int param, float defaultValue = 0.0f );
	extern double get_parameter_double( int param, double defaultValue = 0.0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////