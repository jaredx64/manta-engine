#pragma once

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/color.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GfxRenderTarget2D;
class Command;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using CommandHandle = u32;

#define CONSOLE_COMMAND_LAMBDA []( void *payload )
#define CONSOLE_COMMAND_FUNCTION(name) void name( void *payload )
#define CONSOLE_COMMAND_FUNCTION_POINTER(name) void ( *name )( void * )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysConsole
{
	extern bool init();
	extern bool free();

	extern Command *command( const CommandHandle handle );

	extern void navigate();

	extern void tokenize_input( const char *string );
	extern void update_input();
	extern void update_candidates( const bool processHiddenCommands );

	extern void draw_input( const Delta delta );
	extern void draw_log( const Delta delta );
	extern void draw_candidates( const Delta delta );

	extern void Log( const Color color, const char *command, const char *message );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysConsole
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
		static void dvar_set_hidden( void *payload, const bool hidden );
		static void dvar_set_enabled( void *payload, const bool enabled );

	public:
		static constexpr int STATIC_DVARS_MAX = 1024;
		DVarDeferred dvars[STATIC_DVARS_MAX];
		usize current = 0;
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
		DVar( bool scoped, class GfxRenderTarget2D *variable, const char *definition, const char *description );
	public:
		void *payload = nullptr;
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DVAR_RELEASE(variable) \
	SysConsole::DVarInitializer::dvar_release( &( variable ) )

#define DVAR_HOOK(variable) \
	{ SysConsole::DVar __dv_ { false, &( variable ), #variable, "" }; }
#define DVAR_HOOK_DESC(variable,description) \
	{ SysConsole::DVar __dv_ { false, &( variable ), #variable, description }; }
#define DVAR_HOOK_NAME(variable,name) \
	{ SysConsole::DVar __dv_ { false, &( variable ), name, "" }; }
#define DVAR_HOOK_NAME_DESC(variable,name,description) \
	{ SysConsole::DVar __dv_ { false, &( variable ), name, description }; }


#define DVAR(variable) \
	SysConsole::DVar __dv_##variable { true, &( variable ), #variable, "" }
#define DVAR_DESC(variable,description) \
	SysConsole::DVar __dv_##variable { true, &( variable ), #variable, description }
#define DVAR_NAME(variable,name) \
	SysConsole::DVar __dv_##variable { true, &( variable ), name, "" }
#define DVAR_NAME_DESC(variable,name,description) \
	SysConsole::DVar __dv_##variable { true, &( variable ), name, description }

#define DVAR_HIDE(variable) \
	SysConsole::DVarInitializer::dvar_set_hidden( &( variable ), true )
#define DVAR_UNHIDE(variable) \
	SysConsole::DVarInitializer::dvar_set_hidden( &( variable ), false )
#define DVAR_SET_HIDDEN(variable,hidden) \
	SysConsole::DVarInitializer::dvar_set_hidden( &( variable ), hidden )

#define DVAR_ENABLE(variable) \
	SysConsole::DVarInitializer::dvar_set_enabled( &( variable ), true )
#define DVAR_DISABLE(variable) \
	SysConsole::DVarInitializer::dvar_set_enabled( &( variable ), false )
#define DVAR_SET_ENABLED(variable,enabled) \
	SysConsole::DVarInitializer::dvar_set_enabled( &( variable ), enabled )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Console
{
	extern void draw( const Delta delta );

	extern CommandHandle command_init( const char *definition, const char *description,
		CONSOLE_COMMAND_FUNCTION_POINTER( function ), void *payload = nullptr );

	extern void command_param_description( const CommandHandle command,
		const int param, const char *description );

	extern void command_free( const CommandHandle command );
	extern void command_set_hidden( const CommandHandle command, const bool hidden );
	extern void command_set_enabled( const CommandHandle command, const bool enabled );

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

	extern void Log( const Color color, const char *message, ... );
	extern void LogCommand( const Color color, const char *command, const char *message, ... );

	extern usize get_parameter_count();

	extern const char *get_parameter_string( const int param, const char *defaultValue = "" );
	extern int get_parameter_int( const int param, const int defaultValue = 0 );
	extern u32 get_parameter_u32( const int param, const u32 defaultValue = 0 );
	extern u64 get_parameter_u64( const int param, const u64 defaultValue = 0 );
	extern bool get_parameter_bool( const int param, const bool defaultValue = false );
	extern bool get_parameter_toggle( const int param, const bool currentValue = false );
	extern float get_parameter_float( const int param, const float defaultValue = 0.0f );
	extern double get_parameter_double( const int param, const double defaultValue = 0.0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////