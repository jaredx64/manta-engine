#include <manta/network.hpp>

#include <vendor/posix.hpp>
#include <vendor/stdlib.hpp>

#include <core/debug.hpp>
#include <core/buffer.hpp>

#include <core/checksum.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct NetworkSocketResource
{
	int socket;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool set_nonblocking( int fd )
{
	int flags = ::fcntl( fd, F_GETFL, 0 );
	if( flags == -1 ) { return false; }
	return ::fcntl( fd, F_SETFL, flags | O_NONBLOCK ) == 0;
}


static sockaddr_in network_endpoint_to_sockaddr_in( const NetworkEndpoint &endpoint )
{
	sockaddr_in addr { };
	addr.sin_family = AF_INET;
	addr.sin_port = htons( static_cast<uint16_t>( endpoint.port ) );
	::inet_pton( AF_INET, endpoint.ip.ip, &addr.sin_addr );
	return addr;
}


static NetworkEndpoint sockaddr_in_to_network_endpoint( const sockaddr_in &addr )
{
	NetworkEndpoint endpoint;
	endpoint.port = ntohs( addr.sin_port );
	::inet_ntop( AF_INET, &addr.sin_addr, endpoint.ip.ip, sizeof( endpoint.ip.ip ) );
	return endpoint;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkSocketTCP::init( NetworkSocketType type, NetworkPort port, void *context )
{
	MemoryAssert( resource == nullptr );
	resource = reinterpret_cast<NetworkSocketResource *>( memory_alloc( sizeof( NetworkSocketResource ) ) );
	resource->socket = -1;

	this->type = type;
	this->port = port;
	this->context = context;

	buffer.init( 8192, true );
	delimiter = 0LLU;
	checksum = 0U;
	payloadSize = 0U;
	packetsDropped = 0U;

	connected = false;
}


void NetworkSocketTCP::free()
{
	if( resource )
	{
		disconnect();
		memory_free( resource );
		resource = nullptr;
	}

	if( buffer.data )
	{
		buffer.free();
	}
}


bool NetworkSocketTCP::listen()
{
	Assert( type == NetworkSocketType_Server );
	Assert( resource != nullptr );

	// Initialize Network System
	if( !Network::init() )
	{
		ErrorReturnMsg( false, "%s: failed to initialize network system", __FUNCTION__ );
	}

	// Create Socket
	resource->socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( resource->socket < 0 )
	{
		ErrorReturnMsg( false, "%s: socket failed", __FUNCTION__ );
	}
	int yes = 1;
	::setsockopt( resource->socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes ) );

	// Non-blocking socket
	if( !set_nonblocking( resource->socket ) )
	{
		::close( resource->socket );
		resource->socket = -1;
		ErrorReturnMsg( false, "%s: non-blocking failed", __FUNCTION__ );
	}

	// Bind socket to the ip address and port
	sockaddr_in addr { };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons( static_cast<u16>( port ) );

	if( ::bind( resource->socket, reinterpret_cast<sockaddr *>( &addr ), sizeof( addr ) ) != 0 )
	{
		::close( resource->socket );
		resource->socket = -1;
		ErrorReturnMsg( false, "%s: bind failed", __FUNCTION__ );
	}

	// Start listening for incoming connections
	if( ::listen( resource->socket, SOMAXCONN ) != 0 )
	{
		::close( resource->socket );
		resource->socket = -1;
		ErrorReturnMsg( false, "%s: listen failed", __FUNCTION__ );
	}

	connected = true;
	return true;
}


bool NetworkSocketTCP::connect( const char *host )
{
	Assert( type == NetworkSocketType_Client );
	Assert( resource != nullptr );

	// Initialize Network System
	if( !Network::init() )
	{
		ErrorReturnMsg( false, "%s: failed to initialize network system", __FUNCTION__ );
	}

	char portString[32];
	snprintf( portString, sizeof( portString ), "%u", port );

	addrinfo hints { };
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve Host
	addrinfo *first = nullptr;
	if( ::getaddrinfo( host, portString, &hints, &first ) != 0 )
	{
		ErrorReturnMsg( false, "%s: getaddrinfo failed", __FUNCTION__ );
	}

	// Connect
	bool success = false;
	for( addrinfo *it = first; it; it = it->ai_next )
	{
		int fd = ::socket( it->ai_family, it->ai_socktype, it->ai_protocol );
		if( fd < 0 ) { continue; }

		if( ::connect( fd, it->ai_addr, it->ai_addrlen ) == 0 )
		{
			resource->socket = fd;
			success = true;
			break;
		}

		::close( fd );
	}
	::freeaddrinfo( first );
	ErrorReturnIf( !success, false, "%s: connect failed", __FUNCTION__ );

	// Disable Nagle
	int noDelay = 1;
	::setsockopt( resource->socket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof( noDelay ) );

	// Non-blocking
	if( !set_nonblocking( resource->socket ) )
	{
		::close( resource->socket );
		resource->socket = -1;
		ErrorReturnMsg( false, "%s: non-blocking failed", __FUNCTION__ );
	}

	connected = true;
	return true;
}


void NetworkSocketTCP::disconnect()
{
	if( !resource || resource->socket < 0 ) { return; }
	Assert( connected );
	::shutdown( resource->socket, SHUT_RDWR );
	::close( resource->socket );
	resource->socket = -1;
	if( callbackOnDisconnect ) { callbackOnDisconnect( *this ); }
	connected = false;
}


bool NetworkSocketTCP::accept_connection( NetworkSocketTCP *connection )
{
	Assert( type == NetworkSocketType_Server );
	Assert( resource != nullptr );

	sockaddr_in addr;
	socklen_t len = sizeof( addr );

	int client = ::accept( resource->socket, reinterpret_cast<sockaddr *>( &addr ), &len );
	if( client < 0 )
	{
		if( errno == EWOULDBLOCK || errno == EAGAIN ) { return false; }
		return false;
	}

	if( connection == nullptr )
	{
		::shutdown( client, SHUT_RDWR );
		::close( client );
		return false;
	}

	int noDelay = 1;
	::setsockopt( client, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof( noDelay ) );

	if( !set_nonblocking( client ) )
	{
		::close( client );
		return false;
	}

	Assert( connection->type == NetworkSocketType_Connection );
	Assert( connection->resource != nullptr );
	connection->resource->socket = client;
	connection->connected = true;
	return true;
}


NetworkReceiveEvent NetworkSocketTCP::receive( NetworkConnectionHandle handle,
	void ( *process )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	Assert( connected );
	Assert( resource && resource->socket >= 0 );

	int bytes = 0;

	for( ;; )
	{
		char stream[16384];
		int size = ::recv( resource->socket, stream, sizeof( stream ), 0 );
		bytes += size > 0 ? size : 0;

		if( size > 0 )
		{
			packet_process_buffer( handle, stream, size, process );
		}
		else if( size == 0 )
		{
			return NetworkReceiveEvent_Disconnect;
		}
		else
		{
			if( errno == EWOULDBLOCK || errno == EAGAIN ) { break; } // No more data available (non-blocking)
			return NetworkReceiveEvent_Disconnect;
		}
	}

	return bytes == 0 ? NetworkReceiveEvent_None : NetworkReceiveEvent_Data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkSocketUDP::init( NetworkSocketType type, NetworkPort port, void *context )
{
	MemoryAssert( resource == nullptr );
	resource = reinterpret_cast<NetworkSocketResource *>( memory_alloc( sizeof( NetworkSocketResource ) ) );
	resource->socket = -1;

	this->type = type;
	this->port = port;
	this->context = context;

	buffer.init( NETWORK_PACKET_MAX_BYTES, false );
	sequence = 0U;
}


void NetworkSocketUDP::free()
{
	if( resource )
	{
		disconnect();
		memory_free( resource );
		resource = nullptr;
	}

	if( buffer.data )
	{
		buffer.free();
	}
}


bool NetworkSocketUDP::bind()
{
	Assert( resource != nullptr );

	if( !Network::init() )
	{
		ErrorReturnMsg( false, "%s: network init failed", __FUNCTION__ );
	}

	resource->socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( resource->socket < 0 )
	{
		ErrorReturnMsg( false, "%s: socket failed", __FUNCTION__ );
	}

	// Non-blocking
	if( !set_nonblocking( resource->socket ) )
	{
		::close( resource->socket );
		resource->socket = -1;
		ErrorReturnMsg( false, "%s: non-blocking failed", __FUNCTION__ );
	}

	// Allow rebinding
	int yes = 1;
	::setsockopt( resource->socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes ) );

	sockaddr_in addr { };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons( static_cast<u16>( port ) );
	if( ::bind( resource->socket, reinterpret_cast<sockaddr *>( &addr ), sizeof( addr ) ) != 0 )
	{
		::close( resource->socket );
		resource->socket = -1;
		ErrorReturnMsg( false, "%s: bind failed", __FUNCTION__ );
	}

	return true;
}


void NetworkSocketUDP::disconnect()
{
	if( !resource || resource->socket < 0 ) { return; }
	::close( resource->socket );
	resource->socket = -1;
	sequence = 0U;
	if( callbackOnDisconnect ) { callbackOnDisconnect( *this ); }
}


NetworkReceiveEvent NetworkSocketUDP::receive( void *validateContext,
	bool ( *functionValidate )( void *context, NetworkConnectionHandle handle, u32 sequence,
		const NetworkEndpoint &endpoint ),
	void ( *functionProcess )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	Assert( resource != nullptr && resource->socket >= 0 );

	int packets = 0;
	static char packet[NETWORK_PACKET_MAX_BYTES];
	constexpr int MAX_PACKETS_PER_TICK = 128;

	for( ;; )
	{
		if( packets >= MAX_PACKETS_PER_TICK ) { break; }

		sockaddr_in addr { };
		socklen_t addrLen = sizeof( addr );

		int size = ::recvfrom( resource->socket, packet, sizeof( packet ),
			0, reinterpret_cast<sockaddr *>( &addr ), &addrLen );

		if( size > 0 )
		{
			Assert( size < static_cast<int>( sizeof( packet ) ) );
			if( size >= static_cast<int>( sizeof( packet ) ) ) { packets++; continue; }
			const NetworkEndpoint endpoint = sockaddr_in_to_network_endpoint( addr );
			packet_process_buffer( endpoint, packet, size, validateContext, functionValidate, functionProcess );
			packets++;
			continue;
		}
		else if( size == 0 )
		{
			packets++;
			continue;
		}
		else
		{
			if( errno == EWOULDBLOCK || errno == EAGAIN ) { break; }
			return NetworkReceiveEvent_Disconnect;
		}
	}

	return packets > 0 ? NetworkReceiveEvent_Data : NetworkReceiveEvent_None;
}



bool NetworkSocketUDP::send( NetworkConnectionHandle handle, const NetworkEndpoint &endpoint,
	const void *data, usize size )
{
	Assert( resource && resource->socket >= 0 );
	Assert( data );
	Assert( size > 0 && size <= NETWORK_PACKET_MAX_BYTES - NETWORK_PACKET_UDP_HEADER_BYTES );

	static char packet[NETWORK_PACKET_MAX_BYTES];

	const NetworkPacketHeaderUDP header = NetworkPacketHeaderUDP { handle, sequence };

	memory_copy( packet, &header, sizeof( header ) );
	memory_copy( packet + sizeof( header ), data, size );

	const sockaddr_in addr = network_endpoint_to_sockaddr_in( endpoint );

	int sent = ::sendto( resource->socket, packet, static_cast<int>( size + sizeof( header ) ),
		0, reinterpret_cast<const sockaddr *>( &addr ), sizeof( addr ) );

	if( sent < 0 || sent != static_cast<int>( size + sizeof( header ) ) )
	{
		if( errno == EWOULDBLOCK || errno == EAGAIN ) { return false; }
		return false;
	}

	sequence++;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreNetwork::send_buffer( NetworkSocketResource *resource, const char *data, int size )
{
	while( size > 0 )
	{
		int sent = ::send( resource->socket, data, size, 0 );

		if( sent > 0 )
		{
			data += sent;
			size -= sent;
			continue;
		}

		if( sent == 0 )
		{
			return false;
		}

		if( errno == EWOULDBLOCK || errno == EAGAIN )
		{
			continue;
		}

		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Network::init()
{
	if( CoreNetwork::initialized ) { return true; }
	CoreNetwork::initialized = true;
	return true;
}


bool Network::free()
{
	if( !CoreNetwork::initialized ) { return true; }
	CoreNetwork::initialized = false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////