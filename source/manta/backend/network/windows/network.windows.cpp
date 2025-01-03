#include <manta/network.hpp>

#if 0
#include <vendor/winsock.hpp>
#include <vendor/windows.hpp>
#include <vendor/stdlib.hpp>

#include <core/debug.hpp>
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static WSAEVENT listenEvent;


bool network_init()
{
	// Initialize Winsock
	WSADATA wsa;
	if( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) )
		PRINT_ERROR_RETURN( false, "NETWORK: WSA" );

	// Success
	return true;
}


bool network_free()
{
	// Success
}


bool network_connect( Socket &socket, const char *host, const uint16 port )
{
	addrinfo *first;
	u_long    noblock = 1;

	// Setup Hints
	addrinfo  hints;
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
		PRINT_ERROR_RETURN( false, "NETWORK: Failed to resolve host" );

	// Connect To Host
	if( connect( socket.id, first->ai_addr, static_cast<int>( first->ai_addrlen ) ) != 0 )
		goto hell;

	// Set async after we finish connecting to have a blocking connection but
	// non-blocking send and receive
	if( ioctlsocket( socket.id, FIONBIO, &noblock ) != 0 )
		goto hell;

	// TODO: Call this here or not?
	freeaddrinfo( first );

	// Success
	return true;

hell:
	// Failure
	freeaddrinfo( first );
	PRINT_ERROR_RETURN( false, "NETWORK: Failed to connect" );
}



void network_disconnect()
{
}



void network_send( Socket &socket, Buffer &buffer, const int size )
{
	int header[3];

	// 16-byte Header
	header[0] = 0xDEADC0DE;
	header[1] = 12;
	header[2] = size;

	// Send
	send( socket.id, reinterpret_cast<const char *>( &size ), PACKET_HEADER_SIZE, 0 );
	send( socket.id, reinterpret_cast<const char *>( buffer.data ), size, 0 );
}


int network_receive( Socket &socket, void (*process)( Socket &socket, Buffer &packet ) )
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


bool listen_socket_init( Socket &socket, const int type, const uint16 port )
{
	// Init socket
	if( !socket_init( socket, type ) )
		PRINT_ERROR_RETURN( false, "LISTEN SOCKET: Failed to init listen socket" );

	// Bind socket to the ip address and port
	sockaddr_in service;
	service.sin_family      = AF_INET;
	service.sin_addr.s_addr = htonl( INADDR_ANY );
	service.sin_port        = htons( port );

	if( bind( socket.id, (SOCKADDR*)&service, sizeof( service ) ) != 0 )
		PRINT_ERROR_RETURN( false, "LISTEN SOCKET: Failed to bind listen socket" );

	// Start listening for incoming connections
	if( listen( socket.id, SOMAXCONN ) != 0 )
		PRINT_ERROR_RETURN( false, "LISTEN SOCKET: Failed to start listening" );

	// Create listen event
	listenEvent = WSACreateEvent();
	if( listenEvent == WSA_INVALID_EVENT )
		goto hell;

	// Associate listen event with the socket
	if( WSAEventSelect( socket.id, listenEvent, FD_ACCEPT ) != 0 )
		goto hell;

	// Success
	PRINT( "Listen socket success!" );
	return true;

hell:
	// Free event
	WSACloseEvent( listenEvent );
	PRINT_ERROR_RETURN( false, "LISTEN SOCKET: Failed to create listen event" );
}


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


bool socket_init( Socket &socket, const int type )
{
	// NOTE: We are assuming the socket is at least zero-initialized here.
	//       This allows us to re-use them for pooling (and avoid reallocating
	//       their receive buffers everywhere, especially on the server!)

	static const int types     [] { SOCK_STREAM, SOCK_DGRAM };
	static const int protocols [] { IPPROTO_TCP, IPPROTO_UDP };

	// Create Socket
	if( ( socket.id = ::socket( AF_INET, types[type], protocols[type] ) ) == INVALID_SOCKET )
		PRINT_ERROR_RETURN( false, "SOCKET: Invalid socket" );

	// Initialize Receive Buffer
	if( socket.packet_buffer.data == nullptr )
		buffer_init( socket.packet_buffer, 8192, false );

	// Disable Nagle Algorithm (TODO)
	if( type == network_socket_tcp )
	{
		DWORD nodelay = 1;
		setsockopt( socket.id, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>( &nodelay ), sizeof( nodelay ) );
	}

	// Success
	return true;
}


void socket_free( Socket &socket )
{
	// Free Receive Buffer
	buffer_free( socket.packet_buffer ); // TODO: Do this smarter

	// Close socket
	closesocket( socket.id );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif