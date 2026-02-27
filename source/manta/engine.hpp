#pragma once

#include <config.hpp>

#include <core/types.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ProjectCallbacks
{
public:
	ProjectCallbacks( bool ( *init )( int, char ** ), bool ( *free )(), void ( *update )( const Delta ) ) :
		callback_init { init },
		callback_free { free },
		callback_update { update } { }

	bool init( int argc, char **argv ) const
	{
		if( callback_init == nullptr ) { return true; }
		return callback_init( argc, argv );
	}

	bool free() const
	{
		if( callback_free == nullptr ) { return true; }
		return callback_free();
	}

	void update( const Delta delta ) const
	{
		if( callback_update == nullptr ) { return; }
		callback_update( delta );
	}

private:
	bool ( *callback_init )( int, char ** ); // bool init( int argc, char **argv );
	bool ( *callback_free )(); // bool free();
	void ( *callback_update )( const Delta ); // void update( Delta delta );
};


namespace Engine
{
	extern int main( int argc, char **argv, const ProjectCallbacks &project );
	extern void exit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define APPLICATION_MAIN( name )