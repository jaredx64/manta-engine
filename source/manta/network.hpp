#pragma once

#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

#define NETWORKING 1

#define NETWORK_PORT_DEFAULT ( 25565 )
#define NETWORK_IP_DEFAULT ( "127.0.0.1" )

struct PacketHeader
{
	u64 delimiter;
	u32 checksum;
	u32 size;
};

#define PACKET_HEADER_DELIMITER_SIZE ( 8 )
#define PACKET_HEADER_LENGTH_SIZE ( 4 )
#define PACKET_HEADER_CHECKSUM_SIZE ( 4 )
#define PACKET_HEADER_SIZE ( PACKET_HEADER_DELIMITER_SIZE + PACKET_HEADER_LENGTH_SIZE + PACKET_HEADER_CHECKSUM_SIZE )

#define PACKET_HEADER_DELIMITER ( 0x4E592A91A2B0F53F )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( SocketType, int )
{
	SocketType_TCP = 0,
	SocketType_UDP = 1,
};

struct Socket
{
#if NETWORK_WINDOWS
	// Windows
	usize id;
#endif
#if NETWORK_POSIX
	// POSIX
#endif

	// Generic Packet Data
	u64 packet_delimiter; // Unique flag to identify the start of a packet header
	u32 packet_length; // The length of the current packet
	u32 packet_checksum; // Checksum for the packet data
	Buffer packet_buffer; // Buffer containing a single packet while it is being received

	// Packet Security
	u32 packet_size_validation_offset = PACKET_HEADER_DELIMITER_SIZE;
	u8 dropped_packets; // Number of dropped/corrupt packets this socket has sent
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool network_init();
extern bool network_free();

extern bool network_connect( Socket &socket, const char *host, const u16 port );
extern void network_disconnect();

extern void network_send( Socket &socket, Buffer &buffer, const usize size );
extern int network_receive( Socket &socket, void ( *process )( Socket &socket, Buffer &packet ) );

extern bool listen_socket_init( Socket &socket, const SocketType type, const u16 port );
extern bool listen_socket_accept_connections( Socket &listenSocket, Socket &outSocket );

extern bool socket_init( Socket &socket, const SocketType type );
extern void socket_free( Socket &socket );

extern void packet_process_buffer( Socket &socket, void *data, const usize data_size,
	void ( *process )( Socket &socket, Buffer &packet ) );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( NetworkProtocol, int )
{
	NetworkProtocol_TCP = 0,
	NetworkProtocol_UDP = 1,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NSocket
{
public:
	bool init( const NetworkProtocol protocol );
	bool free();

	bool connect( const char *ip, const u16 port );
	bool disconnect();

	void send( void *data, const usize size );
	void send( const Buffer &buffer );
	void send( const Buffer &buffer, const usize size );
	int receive( void ( *process )( NSocket &socket, Buffer &packet ) );

private:
	NetworkProtocol protocol;
	u64 id = U64_MAX; // Windows
	Buffer buffer;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NConnection
{
public:
	explicit operator bool() const { return id < U32_MAX; }
	operator u32() const { return id; }

public:
	u32 id = U32_MAX;
	const char *ip = "";
	NSocket socket;
};


class NConnections
{
public:
	bool init( const u32 capacity );
	bool free();

	u32 make_new();
	bool release( const u32 id );

	NConnection &at( const u32 id );
	const NConnection &at( const u32 id ) const;
	NConnection &operator[]( const u32 id ) { return at( id ); }
	const NConnection &operator[]( const u32 id ) const { return at( id ); }

public:
	class forward_iterator
	{
	public:
		forward_iterator( NConnections &context, const bool begin ) : context { context }, index{ 0 }
		{
			if( !begin ) { connection = nullptr; return; } // end() state
			MemoryAssert( context.connections != nullptr );
			connection = &context.connections[index];
			if( connection->id == U32_MAX ) { advance(); } // begin() state
		}
		forward_iterator &operator++() { advance(); return *this; }
		bool operator!=( const forward_iterator &other ) const { return connection != other.connection; }
		NConnection &operator*() { MemoryAssert( connection != nullptr ); return *connection; }

	private:
		void advance()
		{
			MemoryAssert( context.connections != nullptr );
			while( ++index < context.capacity )
			{
				connection = &context.connections[index];
				if( connection->id < U32_MAX ) { return; }
			}
			connection = nullptr;
		}

	private:
		NConnections &context;
		NConnection *connection;
		u32 index;
	};

	forward_iterator begin() { return forward_iterator( *this, true ); }
	forward_iterator end() { return forward_iterator( *this, false ); }

private:
	NConnection *connections = nullptr;
	u32 capacity;
	u32 current;

public:
	u32 count;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NServer
{
public:
	bool init( const NetworkProtocol protocol, const u16 port, const u32 count );
	bool free();

	u32 connection_accept();
	bool connection_disconnect( u32 id );

public:
	NSocket socket;
	NConnections connections;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreNetwork
{
	extern bool init();
	extern bool free();

	extern bool connect( NSocket &socket, const char *host, const u16 port );
	extern bool disconnect( NSocket &socket );

	extern bool send( NSocket &socket, void *data, const usize size );
	extern int receive( void ( *process )( NSocket &socket, Buffer &packet ) );

	extern bool socket_init( NSocket &socket, const NetworkProtocol protocol );
	extern bool socket_listener_init( NSocket &socket, const NetworkProtocol protocol, const u16 port );
	extern bool socket_free( NSocket &socket );

	extern NSocket socket_listener_accept_connection( NSocket &socket );
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////