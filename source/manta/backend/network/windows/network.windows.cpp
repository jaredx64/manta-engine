#include <manta/network.hpp>

// TODO: Networking module is WIP
#if 0

#include <vendor/winsock.hpp>
#include <vendor/windows.hpp>
#include <vendor/stdlib.hpp>
#include <vendor/stdio.hpp>

#include <core/debug.hpp>
#include <core/buffer.hpp>

#include <core/checksum.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static WSAEVENT listenEvent;


bool network_init()
{
	// Initialize Winsock
	WSADATA wsa;
	if( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) )
	{
		ErrorReturnMsg( false, "%s: failed to start WSA", __FUNCTION__ );
	}

	return true;
}


bool network_free()
{
	return true;
}


bool network_connect( Socket &socket, const char *host, const u16 port )
{
	addrinfo *first;
	u_long noblock = 1;
	const char *errorMessage = "";

	// Setup Hints
	addrinfo hints;
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_canonname = nullptr;
	hints.ai_addr = nullptr;
	hints.ai_next = nullptr;

	// Winsock wants the port as a string for some reason
	char sport[32];
	itoa( static_cast<int>( port ), sport, 10 );

	// Resolve Host
	if( getaddrinfo( host, sport, &hints, &first ) != 0 || first == nullptr )
	{
		ErrorReturnMsg( false, "%s: failed to resolve host", __FUNCTION__ );
	}

	// Connect To Host
	if( connect( socket.id, first->ai_addr, static_cast<int>( first->ai_addrlen ) ) != 0 )
	{
		errorMessage = "failed to connect to host";
		goto error;
	}

	// Set async after connecting to a blocking connection but non-blocking send and receive
	if( ioctlsocket( socket.id, FIONBIO, &noblock ) != 0 )
	{
		errorMessage = "failed to connect to set ioctlsocket";
		goto error;
	}

	// Success
	freeaddrinfo( first ); // TODO: Call this?
	return true;

error:
	// Failure
	freeaddrinfo( first );
	ErrorReturnMsg( false, "%s: %s", __FUNCTION__, errorMessage );
}



void network_disconnect()
{
	// ...
}



void network_send( Socket &socket, Buffer &buffer, const usize size )
{
	/*
	int header[3];

	// 16-byte Header
	header[0] = 0xDEADC0DE;
	header[1] = 12;
	header[2] = static_cast<int>( size );
	*/

	Assert( buffer.tell >= size );

	PacketHeader header;
	header.delimiter = PACKET_HEADER_DELIMITER;
	header.checksum = checksum_xcrc32( reinterpret_cast<const char *>( buffer.data ), size, 0 );
	header.size = static_cast<u32>( size );

	// Send
	//send( socket.id, reinterpret_cast<const char *>( &size ), PACKET_HEADER_SIZE, 0 );
	send( socket.id, reinterpret_cast<const char *>( &header ), PACKET_HEADER_SIZE, 0 );
	send( socket.id, reinterpret_cast<const char *>( buffer.data ), size, 0 );
}


int network_receive( Socket &socket, void ( *process )( Socket &socket, Buffer &packet ) )
{
	int bytes = 0;

	for( ;; )
	{
		char stream[16384];
		const int size = recv( socket.id, stream, sizeof( stream ), 0 );
		bytes += size > 0 ? size : 0;

		// TODO: Optimize this to remove memmove?
		if( size > 0 )
		{
			// Process packet byte stream
			packet_process_buffer( socket, stream, size, process );
		}
		else if( size == 0 )
		{
			// Graceful Disconnect
			break;
		}
		else
		{
			// Error or no data received
			if( WSAGetLastError() != WSAEWOULDBLOCK )
			{
				// TODO: socket error state
			}
			break;
		}
	}

	// Return the total number of bytes received
	return bytes;
}


bool listen_socket_init( Socket &socket, const SocketType type, const u16 port )
{
	const char *errorMessage = "";

	// Init socket
	if( !socket_init( socket, type ) )
	{
		ErrorReturnMsg( false, "%s: failed to init listen socket", __FUNCTION__ );
	}

	// Bind socket to the ip address and port
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl( INADDR_ANY );
	service.sin_port = htons( port );

	if( bind( socket.id, (SOCKADDR*)&service, sizeof( service ) ) != 0 )
	{
		ErrorReturnMsg( false, "%s: failed to bind listen socket", __FUNCTION__ );
	}

	// Start listening for incoming connections
	if( listen( socket.id, SOMAXCONN ) != 0 )
	{
		ErrorReturnMsg( false, "%s: failed to start listening", __FUNCTION__ );
	}

	// Create listen event
	listenEvent = WSACreateEvent();
	if( listenEvent == WSA_INVALID_EVENT )
	{
		errorMessage = "failed to create listen event";
		goto error;
	}

	// Associate listen event with the socket
	if( WSAEventSelect( socket.id, listenEvent, FD_ACCEPT ) != 0 )
	{
		errorMessage =  "failed to associate listen event with socket";
		goto error;
	}

	// Success
	PrintLn( "Listen socket success!" ); // TODO: remove
	return true;

error:
	// Free event
	WSACloseEvent( listenEvent );
	ErrorReturnMsg( false, "%s: %s", __FUNCTION__, errorMessage );
}

/*
bool listen_socket_accept_connections(Socket &listenSocket, Socket &outSocket)
{
	DWORD result = WSAWaitForMultipleEvents(1, &listenSocket.event, false, 0, false);

	if (result == WSA_WAIT_EVENT_0)
	{
		WSANETWORKEVENTS netEvents;
		if (WSAEnumNetworkEvents(listenSocket.id, listenSocket.event, &netEvents) == 0)
		{
			if ((netEvents.lNetworkEvents & FD_ACCEPT) &&
			    netEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				outSocket.id = accept(listenSocket.id, NULL, NULL);
				if (outSocket.id != INVALID_SOCKET)
					return true;
			}
		}
	}

	return false; // No connection this frame
}
*/

bool listen_socket_accept_connections( Socket &listenSocket, Socket &outSocket )
{
	DWORD result = WSAWaitForMultipleEvents( 1, &listenEvent, false, WSA_INFINITE, false );

	if( result != WSA_WAIT_FAILED )
	{
		outSocket.id = accept( listenSocket.id, NULL, NULL );
		if( outSocket.id != INVALID_SOCKET )
		{
			// Accept connection!
			return true;
		}
	}

	// No connections to be made this frame
	return false;
}


bool socket_init( Socket &socket, const SocketType type )
{
	// NOTE: We are assuming the socket is at least zero-initialized here
	// This allows us to re-use them for pooling (and avoid reallocating
	// their receive buffers everywhere, especially on the server!)

	static const int types [] { SOCK_STREAM, SOCK_DGRAM };
	static const int protocols [] { IPPROTO_TCP, IPPROTO_UDP };

	// Create Socket
	if( ( socket.id = ::socket( AF_INET, types[type], protocols[type] ) ) == INVALID_SOCKET )
	{
		ErrorReturnMsg( false, "%s: invalid socket", __FUNCTION__ );
	}

	// Initialize Receive Buffer
	if( socket.packet_buffer.data == nullptr )
	{
		socket.packet_buffer.init( 8192, false );
	}

	// Disable Nagle Algorithm (TODO)
	if( type == SocketType_TCP )
	{
		DWORD nodelay = 1;
		setsockopt( socket.id, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>( &nodelay ), sizeof( nodelay ) );
	}

	return true;
}


void socket_free( Socket &socket )
{
	// Free Receive Buffer
	socket.packet_buffer.free(); // TODO: Do this smarter

	// Close socket
	closesocket( socket.id );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

bool SysNetwork::init()
{
	// Initialize Winsock
	WSADATA wsa;
	ErrorReturnIf( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0, false, "%s: failed to start Winsock", __FUNCTION__ );

	return true;
}


bool SysNetwork::free()
{
	// Free Winsock
	ErrorReturnIf( WSACleanup() != 0, false, "%s: failed to free Winsock", __FUNCTION__ );

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const int types [] { SOCK_STREAM, SOCK_DGRAM };
static const int protocols [] { IPPROTO_TCP, IPPROTO_UDP };


bool NSocket::init( const NetworkProtocol protocol )
{
	// NOTE: We are assuming the socket is at least zero-initialized here
	// This allows us to re-use them for pooling (and avoid reallocating
	// their receive buffers everywhere, especially on the server!)

	this->protocol = protocol;

	// Create Socket
	ErrorReturnIf( ( this->id = ::socket( AF_INET, types[protocol], protocols[protocol] ) ) == INVALID_SOCKET, false,
		"%s: invalid socket", __FUNCTION__ );

	// Initialize Receive Buffer // TODO
	if( buffer.data == nullptr ) { buffer.init( 8192, false ); }

	if( protocol == SocketType_TCP )
	{
		// Disable Nagle Algorithm
		int nodelay = 1;
		ErrorReturnIf( ::setsockopt( this->id, IPPROTO_TCP, TCP_NODELAY,
			reinterpret_cast<char *>( &nodelay ), sizeof( nodelay ) ) == SOCKET_ERROR, false,
			"%s: failed to disable nagle", __FUNCTION__ );

		// Reuse Sockets
		int reuse = 1;
        ErrorReturnIf( ::setsockopt( this->id, SOL_SOCKET, SO_REUSEADDR,
			reinterpret_cast<char *>( &reuse ), sizeof( reuse ) ) == SOCKET_ERROR, false,
			"%s: failed to enable socket reuse", __FUNCTION__ );
	}

	return true;
}


bool NSocket::free()
{
	// Free Buffer
	buffer.free();

	// Close Socket
	if( this->id != INVALID_SOCKET )
	{
		ErrorReturnIf( ::closesocket( this->id ) != 0, false,
			"%s: failed to close socket (error: %d)", __FUNCTION__, WSAGetLastError() );
		this->id = INVALID_SOCKET;
	}

	return true;
}


bool NSocket::connect( const char *ip, const u16 port )
{
	// Port (Winsock wants it as a string)
	char sport[32];
	snprintf( sport, sizeof( sport ), "%hu", port );

	// Hints
	addrinfo hints = { };
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = types[this->protocol];
	hints.ai_protocol = protocols[this->protocol];
	hints.ai_addrlen = 0;
	hints.ai_canonname = nullptr;
	hints.ai_addr = nullptr;
	hints.ai_next = nullptr;

	// Resolve host
	addrinfo *first = nullptr;
	int result = ::getaddrinfo( ip, sport, &hints, &first );
	ErrorReturnIf( result != 0 || !first, false,
		"%s: getaddrinfo failed (error: %d)", __FUNCTION__, result );

	// Set non-blocking *before* connect (to avoid blocking)
	u_long noblock = 1;
	if( ::ioctlsocket( this->id, FIONBIO, &noblock ) != 0 )
	{
		::freeaddrinfo( first );
		ErrorReturn( false, "%s: ioctlsocket failed (error: %d)",
			__FUNCTION__, WSAGetLastError() );
	}

	// Attempt connection
	result = ::connect( this->id, first->ai_addr, static_cast<int>( first->ai_addrlen ) );
	if( result != 0 )
	{
		const int lastError = WSAGetLastError();
		if( lastError != WSAEWOULDBLOCK ) // Non-blocking connect in progress
		{
			::freeaddrinfo( first );
			ErrorReturn( false, "%s: connect failed (Error: %d)",
				__FUNCTION__, lastError );
		}
		// else: Connection is in progress (non-blocking mode)
	}

	::freeaddrinfo( first );
	return true;
}


bool NSocket::disconnect()
{
	return true;
}


void NSocket::send( void *data, const usize size )
{
	PacketHeader header;
	header.delimiter = PACKET_HEADER_DELIMITER;
	header.checksum = checksum_xcrc32( reinterpret_cast<const char *>( data ), size, 0 );
	header.size = static_cast<u32>( size );

	::send( id, reinterpret_cast<const char *>( &header ), PACKET_HEADER_SIZE, 0 );
	::send( id, reinterpret_cast<const char *>( data ), size, 0 );
}


void NSocket::send( const Buffer &buffer )
{
	MemoryAssert( buffer.data != nullptr );
	send( buffer.data, buffer.tell );
}


void NSocket::send( const Buffer &buffer, const usize size )
{
	MemoryAssert( buffer.data != nullptr );
	Assert( size < buffer.size_allocated_bytes() );
	send( buffer.data, size );
}


int NSocket::receive( void ( *process )( NSocket &socket, Buffer &packet ) )
{
	int bytes = 0;

	for( ;; )
	{
		char stream[16384];
		const int size = ::recv( id, stream, sizeof( stream ), 0 );
		bytes += size > 0 ? size : 0;

		// TODO: Optimize this to remove memmove?
		if( size > 0 )
		{
			// Process packet byte stream
			//packet_process_buffer( socket, stream, size, process );
		}
		else if( size == 0 )
		{
			// Graceful Disconnect
			break;
		}
		else
		{
			// Error or no data received
			if( WSAGetLastError() != WSAEWOULDBLOCK )
			{
				// TODO: socket error state
			}
			break;
		}
	}

	// Return the total number of bytes received
	return bytes;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////