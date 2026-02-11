
#include <manta/network.hpp>

#include <vendor/vendor.hpp>
#include <vendor/limits.hpp>
#include <vendor/string.hpp>

#include <core/checksum.hpp>
#include <core/memory.hpp>
#include <core/debug.hpp>
#include <core/math.hpp>

#include <manta/random.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static NetworkConnectionHandle generate_connection_handle()
{
	static Random random;
	return random.next_u32( 1U, U32_MAX );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkIP::set( const char *str )
{
	MemoryAssert( str != nullptr );
    const usize length = min( strlen( str ), sizeof( ip ) - 1 );
    memory_copy( ip, str, length );
    ip[length] = '\0';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkSocketTCP::send( const void *data, usize size )
{
	Assert( connected );
	Assert( resource != nullptr );

	Assert( size <= INT_MAX );

	const NetworkPacketHeaderTCP header = NetworkPacketHeaderTCP
	{
		NETWORK_PACKET_DELIMITER,
		checksum_xcrc32( reinterpret_cast<const char *>( data ), size, 0 ),
		static_cast<u32>( size ),
	};

	const char *dataHeader = reinterpret_cast<const char *>( &header );
	const int sizeHeader = sizeof( header );
	if( !CoreNetwork::send_buffer( resource, dataHeader, sizeHeader ) ) { return false; }
	const char *dataPayload = reinterpret_cast<const char *>( data );
	const int sizePayload = static_cast<int>( size );
	if( !CoreNetwork::send_buffer( resource, dataPayload, sizePayload ) ) { return false; }

	return true;
}


bool NetworkSocketTCP::packet_validate_checksum() const
{
	if( NETWORK_PACKET_HEADER_BYTES + payloadSize > buffer.size() ) { return false; }
	const u32 checksumPacket = checksum;
	const u32 checksumBuffer = checksum_xcrc32(
		reinterpret_cast<char *>( buffer.data + NETWORK_PACKET_HEADER_BYTES ), payloadSize, 0 );
	return checksumPacket == checksumBuffer;
}


bool NetworkSocketTCP::packet_drop()
{
	const usize size = buffer.size();

	// Seek to next delimiter in the stream
	for( usize i = 1; i + NETWORK_PACKET_DELIMITER_BYTES <= size; i++ )
	{
		buffer.seek_to( i );
		if( buffer.peek<u64>() == NETWORK_PACKET_DELIMITER )
		{
			buffer.shift( -static_cast<int>( i ) );
			delimiter = 0LLU;
			checksum = 0U;
			payloadSize = 0U;
			return true;
		}
	}

	// If no delimiter found, keep only last (DELIMITER_SIZE - 1) bytes
	const usize keep = min( size, static_cast<usize>( NETWORK_PACKET_DELIMITER_BYTES - 1 ) );
	buffer.shift( -static_cast<int>( size - keep ) );
	delimiter = 0LLU;
	checksum = 0U;
	payloadSize = 0U;
	return false;
}


void NetworkSocketTCP::packet_process_buffer( NetworkConnectionHandle handle, void *data, usize size,
	void ( *process )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	Assert( data != nullptr );
	Assert( size > 0 );

	buffer.seek_end();
	buffer.write( data, size );
	buffer.seek_start();

	for( ;; )
	{
		// 1. Do we have a header?
		if( payloadSize == 0 )
		{
			if( buffer.size() < NETWORK_PACKET_HEADER_BYTES ) { break; }

			buffer.seek_start();

			// Validate delimiter
			if( UNLIKELY( buffer.peek<u64>() != NETWORK_PACKET_DELIMITER ) )
			{
				packet_drop();
				packetsDropped++;
				continue;
			}

			delimiter = buffer.read<u64>();
			checksum = buffer.read<u32>();
			payloadSize = buffer.read<u32>();

			// Validate payload
			if( UNLIKELY( payloadSize == 0 || payloadSize > NETWORK_PACKET_MAX_BYTES ) )
			{
				packet_drop();
				packetsDropped++;
				continue;
			}
		}

		// 2. Do we have the full packet?
		const usize packetSizeTotal = NETWORK_PACKET_HEADER_BYTES + payloadSize;
		if( buffer.size() < packetSizeTotal ) { break; }

		// 3. Process packet
		if( packet_validate_checksum() )
		{
			buffer.seek_to( NETWORK_PACKET_HEADER_BYTES );
			if( process ) { process( context, handle, buffer ); }
		}
		else
		{
			packetsDropped++;
		}

		buffer.shift( -static_cast<int>( packetSizeTotal ) );
		payloadSize = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkConnectionTCP::init( NetworkPort port, void *context )
{
	this->socket.init( NetworkSocketType_Connection, port, context );
	this->handle = NetworkConnectionHandle_Null;
	this->ip = "";
}


void NetworkConnectionTCP::free()
{
	this->socket.free();
	this->handle = NetworkConnectionHandle_Null;
	this->ip = "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkServerTCP::init( NetworkPort port, u32 connectionsCapacity, void *context )
{
	socket.init( NetworkSocketType_Server, port, context );

	MemoryAssert( connections == nullptr );
	const usize connectionsSizeBytes = connectionsCapacity * sizeof( NetworkConnectionTCP );
	connections = reinterpret_cast<NetworkConnectionTCP *>( memory_alloc( connectionsSizeBytes ) );
	for( u32 i = 0; i < connectionsCapacity; i++ )
	{
		new ( &connections[i] ) NetworkConnectionTCP();
		connections[i].init( port, context );
	}

	this->connectionsCapacity = connectionsCapacity;
	this->connectionsCurrent = 0U;
	this->connectionsCount = 0U;

	this->context = context;
}


void NetworkServerTCP::free()
{
	Assert( !socket.connected );

	if( connections != nullptr )
	{
		for( u32 i = 0; i < connectionsCapacity; i++ ) { connections[i].free(); }
		memory_free( connections );
		connections = nullptr;
	}

	socket.free();
}


bool NetworkServerTCP::connect()
{
	Assert( !socket.connected );
	if( !socket.listen() ) { socket.free(); return false; }
	return true;
}


bool NetworkServerTCP::disconnect()
{
	if( !socket.connected ) { return true; }
	for( u32 i = 0; i < connectionsCapacity; i++ ) { connection_disconnect( connections[i].handle, nullptr ); }
	socket.disconnect();
	return true;
}


NetworkConnectionHandle NetworkServerTCP::connection_accept(
	void ( *callbackOnConnect )( void *context, NetworkConnectionHandle handle ) )
{
	const bool allowConnections = connectionsCurrent != connectionsCapacity;
	NetworkSocketTCP *connection = allowConnections ? &connections[connectionsCurrent].socket : nullptr;
	if( !socket.accept_connection( connection ) ) { return NetworkConnectionHandle_Null; }

	const u32 connectionIndex = connectionsCurrent;
	Assert( connections[connectionIndex].socket.connected );
	Assert( connections[connectionIndex].handle == NetworkConnectionHandle_Null );

	connections[connectionIndex].handle = generate_connection_handle();

	if( callbackOnConnect ) { callbackOnConnect( context, connections[connectionIndex].handle ); }

	for( ++connectionsCurrent;
		connectionsCurrent < connectionsCapacity && connections[connectionsCurrent].socket.connected;
		connectionsCurrent++ ) { }

	return connections[connectionIndex].handle;
}


void NetworkServerTCP::connection_disconnect( NetworkConnectionHandle handle,
	void ( *callbackOnDisconnect )( void *context, NetworkConnectionHandle handle ) )
{
	if( handle == NetworkConnectionHandle_Null ) { return; }

	const u32 connectionIndex = connection_get_index( handle );
	if( connectionIndex == U32_MAX ) { return; }

	NetworkConnectionTCP &connection = connections[connectionIndex];
	Assert( connection.socket.connected );
	connection.socket.disconnect();

	Assert( connectionIndex < connectionsCapacity );
	Assert( !connections[connectionIndex].socket.connected );
	Assert( connections[connectionIndex].handle != NetworkConnectionHandle_Null );

	if( callbackOnDisconnect ) { callbackOnDisconnect( context, connections[connectionIndex].handle ); }

	connections[connectionIndex].handle = NetworkConnectionHandle_Null;

	connectionsCurrent = connectionIndex < connectionsCurrent ? connectionIndex : connectionsCurrent;
}


void NetworkServerTCP::receive(
	void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ),
	void ( *callbackOnDisconnect )( void *context, NetworkConnectionHandle handle ) )
{
	for( u32 i = 0; i < connectionsCapacity; i++ )
	{
		NetworkConnectionTCP &connection = connections[i];
		if( !connection.socket.connected ) { continue; }

		const NetworkReceiveEvent event = connection.socket.receive( connection.handle, callbackOnReceive );
		if( event == NetworkReceiveEvent_Disconnect )
		{
			connection_disconnect( connection.handle, callbackOnDisconnect );
		}
	}
}


bool NetworkServerTCP::send_to( NetworkConnectionHandle handle, const void *data, usize size )
{
	const u32 connectionIndex = connection_get_index( handle );
	if( connectionIndex >= connectionsCapacity ) { return false; }
	return connections[connectionIndex].socket.send( data, size );
}


bool NetworkServerTCP::send_to( NetworkConnectionHandle handle, const Buffer &buffer )
{
	return NetworkServerTCP::send_to( handle, buffer.data, buffer.size() );
}


bool NetworkServerTCP::send( const void *data, usize size )
{
	bool success = true;
	for( u32 i = 0; i < connectionsCapacity; i++ )
	{
		if( connections[i].handle == NetworkConnectionHandle_Null ) { continue; }
		if( !connections[i].socket.send( data, size ) ) { success = false; }
	}
	return success;
}


bool NetworkServerTCP::send( const Buffer &buffer )
{
	return NetworkServerTCP::send( buffer.data, buffer.size() );
}


u32 NetworkServerTCP::connection_get_index( NetworkConnectionHandle handle ) const
{
	if( handle == NetworkConnectionHandle_Null ) { return U32_MAX; }
	for( u32 i = 0; i < connectionsCapacity; i++ ) { if( connections[i].handle == handle ) { return i; } }
	return U32_MAX;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClientTCP::init( NetworkIP ip, NetworkPort port, void *context )
{
	socket.init( NetworkSocketType_Client, port, context );
	this->context = context;
	this->hostIP = ip;
	this->hostPort = port;
}


void NetworkClientTCP::free()
{
	socket.free();
}


bool NetworkClientTCP::connect( void ( *callbackOnConnect )( void *context ) )
{
	Assert( !socket.connected );
	if( !socket.connect( hostIP ) ) { socket.free(); return false; }
	if( callbackOnConnect ) { callbackOnConnect( context ); }
	return true;
}


bool NetworkClientTCP::disconnect( void ( *callbackOnDisconnect )( void *context ) )
{
	if( !socket.connected ) { return true; }
	socket.disconnect();
	if( callbackOnDisconnect ) { callbackOnDisconnect( context ); }
	return true;
}


void NetworkClientTCP::receive(
	void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ),
	void ( *callbackOnDisconnect )( void *context ) )
{
	Assert( socket.connected );
	const NetworkReceiveEvent event = socket.receive( NetworkConnectionHandle_Null, callbackOnReceive );
	if( event == NetworkReceiveEvent_Disconnect ) { disconnect( callbackOnDisconnect ); }
}


bool NetworkClientTCP::send( const void *data, usize size )
{
	return socket.send( data, size );
}


bool NetworkClientTCP::send( const Buffer &buffer )
{
	return NetworkClientTCP::send( buffer.data, buffer.size() );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkSocketUDP::packet_validate_sequence( u32 current, u32 previous ) const
{
	return static_cast<i32>( current - previous ) > 0;
}


void NetworkSocketUDP::packet_process_buffer( const NetworkEndpoint &endpoint, void *data, usize size,
	void *validateContext,
	bool ( *functionValidate )( void *context, NetworkConnectionHandle handle, u32 sequence,
		const NetworkEndpoint &endpoint ),
	void ( *functionProcess )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	Assert( data != nullptr );
	Assert( size > 0 );

	buffer.clear();
	buffer.write( data, size );
	buffer.seek_start();

	if( buffer.size() <= NETWORK_PACKET_UDP_HEADER_BYTES ) { return; }

	// 1. Read Header
	const NetworkConnectionHandle handle = buffer.read<NetworkConnectionHandle>();
	const u32 sequence = buffer.read<u32>();

	// 2. Validate
	if( functionValidate && !functionValidate( validateContext, handle, sequence, endpoint ) ) { return; }

	// 3. Process
	Assert( buffer.tell == NETWORK_PACKET_UDP_HEADER_BYTES );
	if( functionProcess ) { functionProcess( context, handle, buffer ); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkServerUDP::init( NetworkPort port, u32 connectionsCapacity, void *context )
{
	socket.init( NetworkSocketType_Server, port, context );

	MemoryAssert( connections == nullptr );
	const usize connectionsSizeBytes = connectionsCapacity * sizeof( NetworkConnectionUDP );
	connections = reinterpret_cast<NetworkConnectionUDP *>( memory_alloc( connectionsSizeBytes ) );
	for( u32 i = 0; i < connectionsCapacity; i++ ) { new ( &connections[i] ) NetworkConnectionUDP(); }

	this->connectionsCapacity = connectionsCapacity;
	this->connectionsCurrent = 0U;
	this->connectionsCount = 0U;

	this->context = context;
}


void NetworkServerUDP::free()
{
	Assert( !socket.connected );

	if( connections != nullptr )
	{
		memory_free( connections );
		connections = nullptr;
	}

	socket.free();
}


bool NetworkServerUDP::connect()
{
	Assert( !socket.connected );
	if( !socket.bind() ) { socket.free(); return false; }
	return true;
}


bool NetworkServerUDP::disconnect()
{
	if( !socket.connected ) { return true; }
	for( u32 i = 0; i < connectionsCapacity; i++ ) { connection_release( connections[i].handle ); }
	socket.disconnect();
	return true;
}


bool NetworkServerUDP::connection_register( NetworkConnectionHandle handle )
{
	Assert( connections != nullptr );

	if( handle == NetworkConnectionHandle_Null ) { return false; }

	if( connectionsCurrent == connectionsCapacity ) { return false; }

	Assert( connections[connectionsCurrent].handle == NetworkConnectionHandle_Null );
	connections[connectionsCurrent].handle = handle;
	connections[connectionsCurrent].sequence = U32_MAX;

	for( ++connectionsCurrent;
		connectionsCurrent < connectionsCapacity &&
			connections[connectionsCurrent].handle != NetworkConnectionHandle_Null;
		connectionsCurrent++ ) { }

	return true;
}


void NetworkServerUDP::connection_release( NetworkConnectionHandle handle )
{
	Assert( connections != nullptr );

	if( handle == NetworkConnectionHandle_Null ) { return; }

	const u32 connectionIndex = connection_get_index( handle );
	if( connectionIndex == U32_MAX ) { return; }

	connections[connectionIndex].handle = NetworkConnectionHandle_Null;
	connections[connectionIndex].endpoint = NetworkEndpoint { };

	connectionsCurrent = connectionIndex < connectionsCurrent ? connectionIndex : connectionsCurrent;
}


void NetworkServerUDP::receive(
	void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	socket.receive( this, NetworkServerUDP::packet_validate, callbackOnReceive );
}


bool NetworkServerUDP::send_to( NetworkConnectionHandle handle, const void *data, usize size )
{
	const u32 connectionIndex = connection_get_index( handle );
	if( connectionIndex >= connectionsCapacity ) { return false; }
	const NetworkConnectionUDP &connection = connections[connectionIndex];
	return socket.send( handle, connection.endpoint, data, size );
}


bool NetworkServerUDP::send_to( NetworkConnectionHandle handle, const Buffer &buffer )
{
	return NetworkServerUDP::send_to( handle, buffer.data, buffer.size() );
}


bool NetworkServerUDP::send( const void *data, usize size )
{
	bool success = true;
	for( u32 i = 0; i < connectionsCapacity; i++ )
	{
		const NetworkConnectionUDP &connection = connections[i];
		if( connection.handle == NetworkConnectionHandle_Null ) { continue; }
		if( !socket.send( connection.handle, connection.endpoint, data, size ) ) { success = false; }
	}
	return success;
}


bool NetworkServerUDP::send( const Buffer &buffer )
{
	return NetworkServerUDP::send( buffer.data, buffer.size() );
}


u32 NetworkServerUDP::connection_get_index( NetworkConnectionHandle handle ) const
{
	if( handle == NetworkConnectionHandle_Null ) { return U32_MAX; }
	for( u32 i = 0; i < connectionsCapacity; i++ ) { if( connections[i].handle == handle ) { return i; } }
	return U32_MAX;
}


bool NetworkServerUDP::packet_validate( void *context, NetworkConnectionHandle handle, u32 sequence,
	const NetworkEndpoint &endpoint )
{
	Assert( context != nullptr );
	NetworkServerUDP *server = reinterpret_cast<NetworkServerUDP *>( context );

	// 1. Handle
	const u32 connectionIndex = server->connection_get_index( handle );
	if( connectionIndex == U32_MAX ) { return false; }
	NetworkConnectionUDP &connection = server->connections[connectionIndex];

	// 2. Sequence
	if( static_cast<i32>( sequence - connection.sequence ) <= 0 ) { return false; }
	connection.sequence = sequence;

	// 3. Endpoint
#if 1
	connection.endpoint.ip = endpoint.ip;
	connection.endpoint.port = endpoint.port;
#else
	if( connection.endpoint.ip[0] == '\0' )
	{
		connection.endpoint.ip = endpoint.ip;
		connection.endpoint.port = endpoint.port;
	}
#endif

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClientUDP::init( NetworkIP ip, NetworkPort port, void *context )
{
	socket.init( NetworkSocketType_Client, 0, context );
	this->context = context;
	this->endpoint = NetworkEndpoint { ip, port };
	this->handle = NetworkConnectionHandle_Null;
}


void NetworkClientUDP::free()
{
	socket.free();
}


bool NetworkClientUDP::connect()
{
	Assert( !socket.connected );
	sequence = U32_MAX;
	if( !socket.bind() ) { socket.free(); return false; }
	return true;
}


bool NetworkClientUDP::disconnect()
{
	if( !socket.connected ) { return true; }
	socket.disconnect();
	return true;
}


void NetworkClientUDP::set_handle( NetworkConnectionHandle handle )
{
	this->handle = handle;
}


void NetworkClientUDP::receive(
	void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) )
{
	socket.receive( this, NetworkClientUDP::packet_validate, callbackOnReceive );
}


bool NetworkClientUDP::send( const void *data, usize size )
{
	return socket.send( handle, endpoint, data, size );
}


bool NetworkClientUDP::send( const Buffer &buffer )
{
	return NetworkClientUDP::send( buffer.data, buffer.size() );
}


bool NetworkClientUDP::packet_validate( void *context, NetworkConnectionHandle handle, u32 sequence,
	const NetworkEndpoint &endpoint )
{
	Assert( context != nullptr );
	NetworkClientUDP *client = reinterpret_cast<NetworkClientUDP *>( context );

	// 1. Handle
	if( handle != client->handle ) { return false; }

	// 2. Sequence
	if( static_cast<i32>( sequence - client->sequence ) <= 0 ) { return false; }
	client->sequence = sequence;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreNetwork::initialized = false;


bool CoreNetwork::init()
{
	// Note: Network initialization is lazy-initialized upon NetworkServer/NetworkClient startup
	initialized = false;
	return true;
}


bool CoreNetwork::free()
{
	return Network::free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Network::parse_ip_port( const char *src, NetworkIP &ip, NetworkPort &port )
{
	if( !src ) { return false; }

	const char *colon = nullptr;
	for( const char *c = src; *c; ++c )
	{
		if( *c == ':' ) { colon = c; break; }
	}

	if( !colon ) { return false; }

	const usize ipLength = static_cast<usize>( colon - src );
	if( ipLength + 1 > sizeof( ip ) ) { return false; }

	// Copy IP
	for( usize i = 0; i < ipLength; ++i ) { ip[i] = src[i]; }
	ip[ipLength] = '\0';

	// Parse port
	port = 0;
	const char *p = colon + 1;
	if( !*p ) { return false; }
	while( *p )
	{
		if( *p < '0' || *p > '9' ) { return false; }
		port = port * 10 + ( *p - '0' );
		if( port > 65535 ) { return false; }
		++p;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////