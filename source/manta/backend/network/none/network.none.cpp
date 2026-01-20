#include <manta/network.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Network::init()
{
	return true;
}


bool Network::free()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkSocketTCP::init( const NetworkSocketType type, const NetworkPort port, const u32 id )
{
	// ...
}


void NetworkSocketTCP::free()
{
	// ...
}


bool NetworkSocketTCP::listen()
{
	return true;
}


bool NetworkSocketTCP::connect( const char *host )
{
	return true;
}


void NetworkSocketTCP::disconnect()
{
	// ...
}


bool NetworkSocketTCP::accept_connection( NetworkSocketTCP *connection )
{
	// ...
}


bool NetworkSocketTCP::send( const void *data, const usize size )
{
	return true;
}


NetworkReceiveEvent NetworkSocketTCP::receive( void ( *process )(
	void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	return NetworkReceiveEvent_None;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////