#pragma once

#if 0
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NETWORK_PORT_DEFAULT ( 25565 )
#define NETWORK_IP_DEFAULT ( "127.0.0.1" )

#define PACKET_HEADER_DELIMITER_SIZE ( 8 )
#define PACKET_HEADER_LENGTH_SIZE ( 4 )
#define PACKET_HEADER_CHECKSUM_SIZE ( 4 )
#define PACKET_HEADER_SIZE ( PACKET_HEADER_DELIMITER_SIZE + PACKET_HEADER_LENGTH_SIZE + PACKET_HEADER_CHECKSUM_SIZE )

#define PACKET_HEADER_DELIMITER ( 0x4E592A91A2B0F53F )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	network_socket_tcp,
	network_socket_udp,
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
	u32 packet_length;    // The length of the current packet
	u32 packet_checksum;  // Checksum for the packet data
	Buffer packet_buffer; // Buffer containing a single packet while it is being received

	// Packet Security
	u32 packet_size_validation_offset = PACKET_HEADER_DELIMITER_SIZE;
	u8 dropped_packets; // Number of dropped/corrupt packets this socket has sent
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern bool network_init();
extern void network_free();

extern bool network_connect( Socket &socket, const char *host, const u16 port );
extern void network_disconnect();

extern void network_send( Socket &socket, struct Buffer &buffer, const int size );
extern int  network_receive( Socket &socket, void ( *process )( Socket &socket, Buffer &packet ) );

extern bool listen_socket_init( Socket &socket, const int type, const u16 port );
extern bool listen_socket_accept_connections( Socket &listenSocket, Socket &outSocket );

extern bool socket_init( Socket &socket, const int type );
extern void socket_free( Socket &socket );

extern void packet_process_buffer( Socket &socket, char *data, const int data_size,
	void ( *process )( Socket &socket, Buffer &packet ) );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif