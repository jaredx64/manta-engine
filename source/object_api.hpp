#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Object Definition API
//
// This header is to be included in project/source/objects/*.cpp files
//
// The object source files using these macros are not actually compiled into the executable;
// rather, it is processed by the build system executable to generate optimized c++ code

#include <core/buffer.hpp>
#include <core/serializer.hpp>
#include <manta/objects.hpp>
#include <objects.generated.intellisense>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Includes:

	// Used with #include -- adds to objects.generated.hpp only
	#define INCLUDES

	// Used with #include -- adds to objects.generated.hpp only
	#define HEADER_INCLUDES

	// Used with #include -- adds to objects.generated.cpp only
	#define SOURCE_INCLUDES


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Object Properties:

	// Type
	#define OBJECT( name ) using namespace CoreObjects::ObjectIntelliSense_##name;

	// Parent
	#define PARENT( parent ) constexpr bool PARENT_OBJECT = true;

	// Inherit (usage: INHERIT::event_create(); )
	#define INHERIT static_assert( PARENT_OBJECT, "Can not inherit without a declared PARENT(...)!" );

	// Max instantiation count (optional: default = -1)
	#define COUNT( count )

	// Max bucket capacity (optional: default = 1024)
	#define BUCKET_SIZE( bucket_size )

	// Hash (expects string, used for serialization system & unique type identification)
	#define HASH( hash )

	// ObjectContext Categories
	#define CATEGORY(...) enum { __VA_ARGS__ };

	// Disable instantiation
	#define VERSIONS(...) enum { __VA_ARGS__ };

	// Disable instantiation
	#define ABSTRACT( enable )

	// Enable networking events
	#define NETWORKED( enable )


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Access Specifiers:

	// Public object member functions/data
	// Usage: PUBLIC int health;
	// Usage: PUBLIC void hit( const int damage ) { ... }
	#define PUBLIC

	// Protected object member functions/data
	// Usage: PROTECTED u8 state;
	// Usage: PROTECTED void update_state() { ... }
	#define PROTECTED

	// Private object member functions/data
	// Usage: PRIVATE u8 state;
	// Usage: PRIVATE void update_state() { ... }
	#define PRIVATE

	// Global helper functions/data
	// Usage: GLOBAL int g_RabbitKills;
	// Usage: GLOBAL void kill_all_rabbits() { ... }
	#define GLOBAL

	// Friend access specifier for protected/private members
	// Usage: FRIEND( struct/class/function )
	#define FRIEND( friend )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Events:

	// Prevents automatic inheriting of an event
	#define NOINHERIT

	// Disables an event (does not inherit from parent) (i.e. EVENT_UPDATE DISABLE)
	#define DISABLE

	// Prevents automatic engine calls to an object's event (i.e. EVENT_UPDATE MANUAL)
	#define MANUAL

	// Custom Constructor
	#define CONSTRUCTOR void __ctor

	// Write / Read / Serializer / Deserialize
	#define WRITE void write( Buffer &buffer )
	#define READ void read( Buffer &buffer )
	#define SERIALIZE void serialize( Serializer &serializer )
	#define DESERIALIZE void deserialize( Deserializer &deserializer )

	// Called upon instantiation
	#define EVENT_CREATE void event_create()

	// Called upon deletion
	#define EVENT_DESTROY void event_destroy()

	// Custom initilization event
	#define EVENT_INITIALIZE void event_initialize()

	// ObjectContext "step" tick
	#define EVENT_UPDATE_CUSTOM void event_update_custom( const Delta delta )
	#define EVENT_UPDATE_GUI void event_update_gui( const Delta delta )
	#define EVENT_UPDATE void event_update( const Delta delta )

	// ObjectContext "draw" tick
	#define EVENT_RENDER_CUSTOM void event_render_custom( const Delta delta )
	#define EVENT_RENDER_GUI void event_render_gui( const Delta delta )
	#define EVENT_RENDER void event_render( const Delta delta )

	// Custom event
	#define EVENT_CUSTOM void event_custom( const Delta delta )

	// Prepare event
	#define EVENT_PREPARE void event_prepare()

	// Event that returns true/fast
	#define EVENT_TEST bool event_test()

	// Called at beginning & end of frames
	#define EVENT_FRAME_START void event_frame_start( const Delta delta )
	#define EVENT_FRAME_END void event_frame_end( const Delta delta )

	// Called at 'sleep' and 'wake' triggers
	#define EVENT_SLEEP void event_sleep( const Delta delta )
	#define EVENT_WAKE void event_wake( const Delta delta )

	// Sets object flags
	#define EVENT_FLAG void event_flag( const u64 code )

	// Partition update event
	#define EVENT_PARTITION void event_partition( void *ptr )

	// Network send & receive events
	#define EVENT_NETWORK_SEND void event_network_send( Buffer &buffer )
	#define EVENT_NETWORK_RECEIVE void event_network_receive( Buffer &buffer )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////