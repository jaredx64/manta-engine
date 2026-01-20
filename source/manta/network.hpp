#pragma once

#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NETWORK_PORT_DEFAULT ( 25565 )
#define NETWORK_IP_DEFAULT ( "127.0.0.1" )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using NetworkConnectionHandle = u32;
#define NetworkConnectionHandle_Null ( 0U )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NETWORK_PACKET_MAX_BYTES ( 1024 * 1024 ) // 1 mb

#define NETWORK_PACKET_DELIMITER ( 0x4E592A91A2B0F53F )

#define NETWORK_PACKET_DELIMITER_BYTES ( 8 )
#define NETWORK_PACKET_CHECKSUM_BYTES ( 4 )
#define NETWORK_PACKET_PAYLOAD_SIZE_BYTES ( 4 )
#define NETWORK_PACKET_HEADER_BYTES ( 16 )

struct NetworkPacketHeaderTCP
{
	u64 delimiter;
	u32 checksum;
	u32 size;
};
static_assert( sizeof( NetworkPacketHeaderTCP ) == NETWORK_PACKET_HEADER_BYTES, "Size mismatch!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NETWORK_PACKET_UDP_HEADER_BYTES ( 8 )

struct NetworkPacketHeaderUDP
{
	NetworkConnectionHandle handle;
	u32 sequence;
};
static_assert( sizeof( NetworkPacketHeaderUDP ) == NETWORK_PACKET_UDP_HEADER_BYTES, "Size mismatch!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct NetworkSocketResource;

enum_type( NetworkSocketType, int )
{
	NetworkSocketType_Client,
	NetworkSocketType_Server,
	NetworkSocketType_Connection,
};


enum_type( NetworkReceiveEvent, int )
{
	NetworkReceiveEvent_None,
	NetworkReceiveEvent_Data,
	NetworkReceiveEvent_Disconnect,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using NetworkPort = u32;


class NetworkIP
{
public:
	NetworkIP() = default;
	NetworkIP( const char* str ) { set( str ); }
	NetworkIP( const NetworkIP &other ) { set( other.ip ); }

	NetworkIP &operator=( const char *str )
	{
		set( str );
		return *this;
	}

	NetworkIP &operator=( const NetworkIP &other )
	{
		if( this != &other ) { set( other.ip ); }
		return *this;
	}

	char &operator[]( usize index ) { Assert( index < sizeof( ip ) ); return ip[index]; }
	const char &operator[]( usize index ) const { Assert( index < sizeof( ip ) ); return ip[index]; }

	bool is_empty() const { return ip[0] == '\0'; }
	explicit operator bool() const { return !is_empty(); }

	const char *cstr() const { return ip; }
	operator const char *() const { return ip; }

	char *cstr() { return ip; }
	operator char *() { return ip; }

	void set( const char *str );

public:
	char ip[48] = { };
};


class NetworkEndpoint
{
public:
	NetworkEndpoint() = default;
	NetworkEndpoint( const char *ip, u32 port ) : ip { ip }, port { port } { };

	bool valid() const { return ip[0] != '\0'; }

public:
	NetworkIP ip = "";
	NetworkPort port = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NetworkSocketTCP
{
public:
	void init( NetworkSocketType type, NetworkPort port, void *context );
	void free();

	bool listen();
	bool connect( const char *host );
	void disconnect();

	bool accept_connection( NetworkSocketTCP *connection );

	NetworkReceiveEvent receive( NetworkConnectionHandle handle,
		void ( *process )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) );

	bool send( const void *data, usize size );

private:
	bool packet_validate_checksum() const;
	bool packet_drop();
	void packet_process_buffer( NetworkConnectionHandle handle, void *data, usize size,
		void ( *process )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) );

public:
	void *context = nullptr;
	struct NetworkSocketResource *resource = nullptr;
	NetworkSocketType type;
	NetworkPort port;

	Buffer buffer;
	u64 delimiter;
	u32 checksum;
	u32 payloadSize;
	u32 packetsDropped;

	bool connected = false;

	void ( *callbackOnDisconnect )( NetworkSocketTCP &socket ) = nullptr;
};


class NetworkConnectionTCP
{
public:
	void init( NetworkPort port, void *context );
	void free();

public:
	NetworkSocketTCP socket = { };
	NetworkConnectionHandle handle;
	NetworkIP ip = "";
};


class NetworkServerTCP
{
public:
	void init( NetworkPort port, u32 connectionsCapacity, void *context );
	void free();
	bool connect();
	bool disconnect();

	NetworkConnectionHandle connection_accept(
		void ( *callbackOnConnect )( void *context, NetworkConnectionHandle handle ) );

	void connection_disconnect( NetworkConnectionHandle handle,
		void ( *callbackOnDisconnect )( void *context, NetworkConnectionHandle handle ) );

	void receive(
		void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ),
		void ( *callbackOnDisconnect )( void *context, NetworkConnectionHandle handle ) );

	bool send_to( NetworkConnectionHandle handle, const void *data, usize size );
	bool send_to( NetworkConnectionHandle handle, const Buffer &buffer );
	bool send( const void *data, usize size );
	bool send( const Buffer &buffer );

private:
	u32 connection_get_index( NetworkConnectionHandle handle ) const;

public:
	void *context = nullptr;
	NetworkConnectionTCP *connections = nullptr;
	NetworkSocketTCP socket;
	u32 connectionsCapacity = 0;
	u32 connectionsCurrent = 0;
	u32 connectionsCount = 0;
};


class NetworkClientTCP
{
public:
	void init( NetworkIP ip, NetworkPort port, void *context );
	void free();
	bool connect( void ( *callbackOnConnect )( void *context ) );
	bool disconnect( void ( *callbackOnDisconnect )( void *context ) );

	void receive(
		void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ),
		void ( *callbackOnDisconnect )( void *context ) );

	bool send( const void *data, usize size );
	bool send( const Buffer &buffer );

public:
	void *context = nullptr;
	NetworkIP hostIP = "";
	NetworkPort hostPort = 0;
	NetworkSocketTCP socket;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class NetworkSocketUDP
{
public:
	void init( NetworkSocketType type, NetworkPort port, void *context );
	void free();

	bool bind();
	void disconnect();

	NetworkReceiveEvent receive(
		void *validateContext,
		bool ( *functionValidate )( void *context, NetworkConnectionHandle handle, u32 sequence,
			const NetworkEndpoint &endpoint ),
		void ( *functionProcess )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) );

	bool send( NetworkConnectionHandle handle, const NetworkEndpoint &endpoint,
		const void *data, usize size );

private:
	bool packet_validate_sequence( u32 current, u32 previous ) const;
	void packet_process_buffer( const NetworkEndpoint &endpoint, void *data, usize size,
		void *validateContext,
		bool ( *functionValidate )( void *context, NetworkConnectionHandle handle, u32 sequence,
			const NetworkEndpoint &endpoint ),
		void ( *functionProcess )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) );

public:
	void *context = nullptr;
	struct NetworkSocketResource *resource = nullptr;
	NetworkSocketType type;
	NetworkPort port;
	Buffer buffer;
	u32 sequence = 0U;

	bool connected = false;

	void ( *callbackOnDisconnect )( NetworkSocketUDP &socket ) = nullptr;
};


struct NetworkConnectionUDP
{
	NetworkConnectionHandle handle = NetworkConnectionHandle_Null;
	NetworkEndpoint endpoint = NetworkEndpoint { };
	u32 sequence = U32_MAX;
};


class NetworkServerUDP
{
public:
	void init( NetworkPort port, u32 connectionsCapacity, void *context );
	void free();
	bool connect();
	bool disconnect();

	bool connection_register( NetworkConnectionHandle handle );
	void connection_release( NetworkConnectionHandle handle );

	void receive(
		void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) );

	bool send_to( NetworkConnectionHandle handle, const void *data, usize size );
	bool send_to( NetworkConnectionHandle handle, const Buffer &buffer );
	bool send( const void *data, usize size );
	bool send( const Buffer &buffer );

private:
	u32 connection_get_index( NetworkConnectionHandle handle ) const;
	static bool packet_validate( void *context, NetworkConnectionHandle handle, u32 sequence,
		const NetworkEndpoint &endpoint );

public:
	void *context = nullptr;
	NetworkConnectionUDP *connections = nullptr;
	NetworkSocketUDP socket;
	u32 connectionsCapacity = 0;
	u32 connectionsCurrent = 0;
	u32 connectionsCount = 0;
};


class NetworkClientUDP
{
public:
	void init( NetworkIP ip, NetworkPort port, void *context );
	void free();
	bool connect();
	bool disconnect();

	void set_handle( NetworkConnectionHandle handle );

	void receive(
		void ( *callbackOnReceive )( void *context, NetworkConnectionHandle handle, Buffer &buffer ) );

	bool send( const void *data, usize size );
	bool send( const Buffer &buffer );

private:
	static bool packet_validate( void *context, NetworkConnectionHandle handle, u32 sequence,
		const NetworkEndpoint &endpoint );

public:
	void *context = nullptr;
	NetworkSocketUDP socket;
	NetworkEndpoint endpoint;
	NetworkConnectionHandle handle = NetworkConnectionHandle_Null;
	u32 sequence = U32_MAX;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreNetwork
{
	extern bool initialized;
	extern bool init();
	extern bool free();
	extern bool send_buffer( NetworkSocketResource *resource, const char *data, int size );
};


namespace Network
{
	extern bool init();
	extern bool free();

	extern bool parse_ip_port( const char *src, NetworkIP &ip, NetworkPort &port );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////