#include <manta/network.hpp>

#include <vendor/winsock.hpp>
#include <vendor/stdlib.hpp>

#include <core/debug.hpp>
#include <core/buffer.hpp>

#include <core/checksum.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct NetworkSocketResource
{
	SOCKET socket;
	HANDLE event;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static sockaddr_in network_endpoint_to_sockaddr_in( const NetworkEndpoint &endpoint )
{
	sockaddr_in addr { };
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons( static_cast<u_short>( endpoint.port ) );
	::inet_pton( AF_INET, endpoint.ip.ip, &addr.sin_addr );
	return addr;
}


static NetworkEndpoint sockaddr_in_to_network_endpoint( const sockaddr_in &addr )
{
	NetworkEndpoint endpoint;
	endpoint.port = ::ntohs( addr.sin_port );
	::inet_ntop( AF_INET, &addr.sin_addr, endpoint.ip.ip, sizeof( endpoint.ip.ip ) );
	return endpoint;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkSocketTCP::init( const NetworkSocketType type, const NetworkPort port, void *context )
{
	MemoryAssert( resource == nullptr );
	resource = reinterpret_cast<NetworkSocketResource *>( memory_alloc( sizeof( NetworkSocketResource ) ) );
	resource->socket = INVALID_SOCKET;
	resource->event = WSA_INVALID_EVENT;

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
	if( resource != nullptr )
	{
		disconnect();
		if( resource->event != WSA_INVALID_EVENT ) { WSACloseEvent( resource->event ); }
		memory_free( resource );
		resource = nullptr;
	}

	if( buffer.data != nullptr )
	{
		buffer.free();
	}
}


bool NetworkSocketTCP::listen()
{
	Assert( resource != nullptr );
	Assert( type == NetworkSocketType_Server );

	// Initialize Network System
	if( !Network::init() )
	{
		ErrorReturnMsg( false, "%s: failed to initialize network system", __FUNCTION__ );
	}

	// Create Socket
	resource->socket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( resource->socket == INVALID_SOCKET )
	{
		ErrorReturnMsg( false, "%s: invalid socket", __FUNCTION__ );
	}

	// Non-blocking socket
	u_long nonBlocking = 1;
	if( ::ioctlsocket( resource->socket, FIONBIO, &nonBlocking ) != 0 )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to set non-blocking mode", __FUNCTION__ );
	}

	// Bind socket to the ip address and port
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl( INADDR_ANY );
	addr.sin_port = ::htons( static_cast<u_short>( port ) );
	if( ::bind( resource->socket, reinterpret_cast<SOCKADDR *>( &addr ), sizeof( addr ) ) != 0 )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to bind listen socket", __FUNCTION__ );
	}

	// Start listening for incoming connections
	if( ::listen( resource->socket, SOMAXCONN ) != 0 )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to start listening", __FUNCTION__ );
	}

	// Create listen event
	resource->event = WSACreateEvent();
	if( resource->event == WSA_INVALID_EVENT )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to create listen event", __FUNCTION__ );
	}

	// Associate listen event with the socket
	if( WSAEventSelect( resource->socket, resource->event, FD_ACCEPT | FD_CLOSE ) != 0 )
	{
		WSACloseEvent( resource->event );
		resource->event = WSA_INVALID_EVENT;
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to associate listen event with socket", __FUNCTION__ );
	}

	connected = true;
	return true;
}


bool NetworkSocketTCP::connect( const char *host )
{
	Assert( resource != nullptr );
	Assert( type == NetworkSocketType_Client );

	// Initialize Network System
	if( !Network::init() )
	{
		ErrorReturnMsg( false, "%s: failed to initialize network system", __FUNCTION__ );
	}

	char portString[32];
	snprintf( portString, sizeof( portString ), "%u", port );

	addrinfo hints;
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_canonname = nullptr;
	hints.ai_addr = nullptr;
	hints.ai_next = nullptr;

	// Resolve Host
	addrinfo *first = nullptr;
	if( ::getaddrinfo( host, portString, &hints, &first ) != 0 || first == nullptr )
	{
		ErrorReturnMsg( false, "%s: failed to resolve host", __FUNCTION__ );
	}

	// Connect
	bool success = false;
	for( addrinfo *it = first; it != nullptr; it = it->ai_next )
	{
		SOCKET socket = ::socket( it->ai_family, it->ai_socktype, it->ai_protocol );
		if( socket == INVALID_SOCKET ) { continue; }
		if( ::connect( socket, it->ai_addr, static_cast<int>( it->ai_addrlen ) ) == 0 )
		{
			resource->socket = socket;
			success = true;
			break;
		}
		::closesocket( socket );
	}
	::freeaddrinfo( first );
	ErrorReturnIf( !success, false, "%s: failed to connect to host (all addresses failed)", __FUNCTION__ );

	// Disable Nagle
	DWORD noDelay = 1;
	if( ::setsockopt( resource->socket, IPPROTO_TCP, TCP_NODELAY,
		reinterpret_cast<char*>( &noDelay ), sizeof( noDelay ) ) != 0 )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to disable Nagle", __FUNCTION__ );
	}

	// Non-blocking
	u_long noBlock = 1;
	if( ::ioctlsocket( resource->socket, FIONBIO, &noBlock ) != 0 )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to connect to set ioctlsocket", __FUNCTION__ );
	}

	connected = true;
	return true;
}


void NetworkSocketTCP::disconnect()
{
	if( !resource || resource->socket == INVALID_SOCKET ) { return; }
	Assert( connected );
	::shutdown( resource->socket, SD_BOTH );
	::closesocket( resource->socket );
	resource->socket = INVALID_SOCKET;
	if( callbackOnDisconnect ) { callbackOnDisconnect( *this ); }
	connected = false;
}


bool NetworkSocketTCP::accept_connection( NetworkSocketTCP *connection )
{
	Assert( type == NetworkSocketType_Server );
	Assert( resource != nullptr );

	DWORD result = WSAWaitForMultipleEvents( 1, &resource->event, false, 0, false );
	if( result == WSA_WAIT_FAILED || result == WSA_WAIT_TIMEOUT ) { return false; }

	WSANETWORKEVENTS events;
	if( WSAEnumNetworkEvents( resource->socket, resource->event, &events ) != 0 ) { return false; }

	if( ( events.lNetworkEvents & FD_ACCEPT ) && events.iErrorCode[FD_ACCEPT_BIT] == 0 )
	{
		const SOCKET socket = ::accept( resource->socket, nullptr, nullptr );
		if( socket == INVALID_SOCKET ) { return false; }

		if( connection == nullptr )
		{
			::shutdown( socket, SD_BOTH );
			::closesocket( socket );
			return false;
		}

		DWORD noDelay = 1;
		if( ::setsockopt( socket, IPPROTO_TCP, TCP_NODELAY,
			reinterpret_cast<char*>( &noDelay ), sizeof( noDelay ) ) != 0 )
		{
			::closesocket( socket );
			ErrorReturnMsg( false, "%s: failed to disable Nagle on connection socket", __FUNCTION__ );
		}

		u_long nonBlocking = 1;
		if( ::ioctlsocket( socket, FIONBIO, &nonBlocking ) != 0 )
		{
			::closesocket( socket );
			ErrorReturnMsg( false, "%s: failed to set non-blocking connection socket", __FUNCTION__ );
		}

		Assert( connection->type == NetworkSocketType_Connection );
		Assert( connection->resource != nullptr );
		connection->resource->socket = socket;
		connection->connected = true;
		return true;
	}

	return false;
}


NetworkReceiveEvent NetworkSocketTCP::receive( NetworkConnectionHandle handle,
	void ( *process )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	Assert( connected );
	Assert( resource && resource->socket != INVALID_SOCKET );

	int bytes = 0;

	for( ;; )
	{
		char stream[16384];
		const int size = ::recv( resource->socket, stream, sizeof( stream ), 0 );
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
			const int error = WSAGetLastError();
			if( error == WSAEWOULDBLOCK ) { break; } // No more data available (non-blocking)
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
	resource->socket = INVALID_SOCKET;
	resource->event = WSA_INVALID_EVENT;

	this->type = type;
	this->port = port;
	this->context = context;

	buffer.init( NETWORK_PACKET_MAX_BYTES, false );
	sequence = 0U;
}


void NetworkSocketUDP::free()
{
	if( resource != nullptr )
	{
		disconnect();
		if( resource->event != WSA_INVALID_EVENT ) { WSACloseEvent( resource->event ); }
		memory_free( resource );
		resource = nullptr;
	}

	if( buffer.data != nullptr )
	{
		buffer.free();
	}
}


bool NetworkSocketUDP::bind()
{
	Assert( resource != nullptr );

	// Initialize Network System
	if( !Network::init() )
	{
		ErrorReturnMsg( false, "%s: failed to initialize network system", __FUNCTION__ );
	}

	// Create Socket
	resource->socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( resource->socket == INVALID_SOCKET )
	{
		ErrorReturnMsg( false, "%s: invalid socket", __FUNCTION__ );
	}

	// Non-blocking socket
	u_long nonBlocking = 1;
	if( ::ioctlsocket( resource->socket, FIONBIO, &nonBlocking ) != 0 )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to set non-blocking mode", __FUNCTION__ );
	}

	// Bind socket to the ip address and port
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl( INADDR_ANY );
	addr.sin_port = ::htons( static_cast<u_short>( port ) );
	if( ::bind( resource->socket, reinterpret_cast<SOCKADDR *>( &addr ), sizeof( addr ) ) != 0 )
	{
		::closesocket( resource->socket );
		resource->socket = INVALID_SOCKET;
		ErrorReturnMsg( false, "%s: failed to bind socket", __FUNCTION__ );
	}

	return true;
}


void NetworkSocketUDP::disconnect()
{
	if( !resource || resource->socket == INVALID_SOCKET ) { return; }
	::closesocket( resource->socket );
	resource->socket = INVALID_SOCKET;
	sequence = 0U;
	if( callbackOnDisconnect ) { callbackOnDisconnect( *this ); }
}


NetworkReceiveEvent NetworkSocketUDP::receive( void *validateContext,
	bool ( *functionValidate )( void *context, NetworkConnectionHandle handle, u32 sequence,
		const NetworkEndpoint &endpoint ),
	void ( *functionProcess )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	Assert( resource != nullptr );
	Assert( resource->socket != INVALID_SOCKET );

	int packets = 0;
	static char packet[NETWORK_PACKET_MAX_BYTES];
	constexpr int MAX_PACKETS_PER_TICK = 128;

	for( ;; )
	{
		if( packets >= MAX_PACKETS_PER_TICK ) { break; }

		sockaddr_in addr { };
		int addrLength = sizeof( addr );

		const int size = ::recvfrom( resource->socket, packet, sizeof( packet ),
			0, reinterpret_cast<sockaddr *>( &addr ), &addrLength );

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
			const int error = WSAGetLastError();
			if( error == WSAEWOULDBLOCK ) { break; }
			return NetworkReceiveEvent_Disconnect;
		}
	}

	return packets > 0 ? NetworkReceiveEvent_Data : NetworkReceiveEvent_None;
}


bool NetworkSocketUDP::send( NetworkConnectionHandle handle, const NetworkEndpoint &endpoint,
	const void *data, usize size )
{
	Assert( resource != nullptr );
	Assert( resource->socket != INVALID_SOCKET );
	Assert( data );
	Assert( size > 0 && size <= NETWORK_PACKET_MAX_BYTES - NETWORK_PACKET_UDP_HEADER_BYTES );

	static char packet[NETWORK_PACKET_MAX_BYTES];

	const NetworkPacketHeaderUDP header = NetworkPacketHeaderUDP { handle, sequence };

	memory_copy( packet, &header, sizeof( header ) );
	memory_copy( packet + sizeof( header ), data, size );

	const sockaddr_in addr = network_endpoint_to_sockaddr_in( endpoint );
	int sent = ::sendto( resource->socket, packet, static_cast<int>( size + sizeof( header ) ),
		0, reinterpret_cast<const sockaddr*>( &addr ), sizeof( addr ) );

	if( sent == SOCKET_ERROR || sent != static_cast<int>( size + sizeof( header ) ) ) { return false; }

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

		if( WSAGetLastError() == WSAEWOULDBLOCK )
		{
			continue;
		}

		return false;
	}

	return true;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Network::init()
{
	if( CoreNetwork::initialized ) { return true; }

	WSADATA wsa;
	if( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 ) { return false; }

	CoreNetwork::initialized = true;
	return true;
}


bool Network::free()
{
	if( !CoreNetwork::initialized ) { return true; }

	if( WSACleanup() != 0 ) { return false; }

	CoreNetwork::initialized = false;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////