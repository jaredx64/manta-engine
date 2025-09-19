#include <manta/network.hpp>

#if 0
#include <core/checksum.hpp>
#include <core/memory.hpp>

#include <vendor/vendor.hpp>

#include <manta/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void packet_shift_buffer( Socket &socket, const usize shift, const usize size )
{
#if NETWORKING
	// TODO: We shouldn't really be messing with the internal state of Buffer here... consider rewriting this
	memory_move( socket.packet_buffer.data, socket.packet_buffer.data + shift, size );
	socket.packet_size_validation_offset = PACKET_HEADER_DELIMITER_SIZE;
	socket.packet_length = 0;
#endif
}


static bool packet_validate_delimiter( Socket &socket )
{
#if NETWORKING
	if( UNLIKELY( socket.packet_delimiter != PACKET_HEADER_DELIMITER ) )
	{
		// This should never happen... so if it does, something must have went very wrong here.
		// Recover by reseting the state and searching for the next valid delimiter (if any)
		socket.dropped_packets++;

		int byteOffset = 1;
		while( byteOffset < socket.packet_buffer.tell )
		{
			// We've reached the end of packet_buffer where there can't possibly be a
			// PACKET_HEADER_DELIMITER (not enough bytes left)
			if( byteOffset > socket.packet_buffer.tell - PACKET_HEADER_DELIMITER_SIZE )
			{
				byteOffset = socket.packet_buffer.tell;
				break;
			}

			// We've found a new PACKET_HEADER_DELIMITER
			socket.packet_buffer.seek_to( byteOffset );
			socket.packet_delimiter = socket.packet_buffer.peek<usize>();
			// socket.packet_delimiter = buffer_peek<uint64>( socket.packet_buffer, byteOffset ); TODO
			if( socket.packet_delimiter == PACKET_HEADER_DELIMITER )
			{
				break;
			}

			// Keep searching
			byteOffset++;
		}

		// Shift the packet_buffer to the new "safe" position
		packet_shift_buffer( socket, byteOffset, socket.packet_buffer.tell - byteOffset );
		return false;
	}
	// Success
	return true;
#else
	return false;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool packet_validate_size( Socket &socket, const usize offset )
{
#if NETWORKING
	// We can only check for size spoofing if we have enough bytes for the delimiter search
	const usize scan_size = offset + min( static_cast<usize>( PACKET_HEADER_SIZE + socket.packet_length ) - offset,
		socket.packet_buffer.tell - offset );

	if( scan_size - socket.packet_size_validation_offset < PACKET_HEADER_DELIMITER_SIZE ) { return true; }

	// Check for packet size spoofing (only on exceedingly large packets)
	if( socket.packet_length != 0 )
	{
		for( usize i = socket.packet_size_validation_offset; i < scan_size; i++ )
		{
			//socket.packet_delimiter = buffer_peek<usize>( socket.packet_buffer, i );
			socket.packet_buffer.seek_to( i );
			socket.packet_delimiter = socket.packet_buffer.peek<u64>();

			if( UNLIKELY( socket.packet_delimiter == PACKET_HEADER_DELIMITER ) )
			{
				// We've found a PACKET_HEADER_DELIMITER where data should be...
				// either the 1 in 2^64 odds of this happening occurred or we're being packet size spoofed.
				// Let's just drop this packet and start from the new position
				socket.dropped_packets++;

				// Shift the buffer contents to the new "safe" position
				packet_shift_buffer( socket, i, socket.packet_buffer.tell - i );
				return false;
			}
		}

		// Update the packet_size_validation_offset
		socket.packet_size_validation_offset = scan_size;
	}

	// Success, no spoofing detected!
	return true;
#else
	return false;
#endif
}


static bool packet_validate_checksum( Socket &socket )
{
#if NETWORKING
	//return LIKELY( checksum_xcrc32( socket.packet_buffer.data + PACKET_HEADER_SIZE, socket.packet_length, 0 ) == socket.packet_checksum );
	return LIKELY( socket.packet_checksum == 5 );
#else
	return false;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void packet_process_buffer( Socket &socket, void *data, const usize data_size,
	void ( *process )( Socket &socket, Buffer &packet ) )
{
#if NETWORKING
	// Receive packet byte stream
	const usize data_block_start_position = socket.packet_buffer.tell;
	socket.packet_buffer.write( data, data_size );
	//buffered_write_buffer( socket.packet_buffer, data, data_size );

	// Check for size spoofing
	packet_validate_size( socket, data_block_start_position );

	for( ;; )
	{
		if( socket.packet_length == 0 )
		{
			// Check if we have enough data for a header
			if( socket.packet_buffer.tell < PACKET_HEADER_SIZE ) { break; }

			// Read header
			socket.packet_buffer.seek_start();
			socket.packet_delimiter = socket.packet_buffer.read<u64>();
			socket.packet_length = socket.packet_buffer.read<u32>();
			socket.packet_checksum = socket.packet_buffer.read<u32>();

			//socket.packet_delimiter = buffer_peek<uint64>( socket.packet_buffer, 0 );
			//socket.packet_length = buffer_peek<uint32>( socket.packet_buffer, PACKET_HEADER_DELIMITER_SIZE );
			//socket.packet_checksum = buffer_peek<uint32>( socket.packet_buffer,
			//	PACKET_HEADER_DELIMITER_SIZE + PACKET_HEADER_LENGTH_SIZE );

			// Validate Delimiter
			if( UNLIKELY( !packet_validate_delimiter( socket ) ) ) { continue; }
		}

		// Check for size spoofing
		if( UNLIKELY( !packet_validate_size( socket, PACKET_HEADER_DELIMITER_SIZE ) ) ) { continue; }

		// We have header info, so let's check if we have the full packet
		const int packet_length_total = socket.packet_length + PACKET_HEADER_SIZE;
		const int excess_data_size = socket.packet_buffer.tell - packet_length_total;
		if( socket.packet_buffer.tell < packet_length_total ) { break; }

		// Seek the buffer to a good position for reading
		socket.packet_buffer.seek_to( PACKET_HEADER_SIZE );
		//buffer_seek( socket.packet_buffer, PACKET_HEADER_SIZE );

		// Verify the checksum
		if( LIKELY( packet_validate_checksum( socket ) ) )
		{
			// Call processing function
			process( socket, socket.packet_buffer );
		}
		else
		{
			// Failed the checksum, something fishy happened to this packet's data...
			socket.dropped_packets++;
		}

		// Shift the excess buffer contents to the beginning
		packet_shift_buffer( socket, packet_length_total, excess_data_size );

		// Break the loop if there is no more readable excess
		if( excess_data_size < PACKET_HEADER_SIZE ) { break; }
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
bool NSocket::init( const NetworkProtocol protocol )
{
	return true;
}


bool NSocket::free()
{
	return true;
}


bool NSocket::connect( const char *ip, const u16 port )
{
	return true;
}


bool NSocket::disconnect()
{
	return true;
}


void NSocket::send( void *data, const usize size )
{

}


void NSocket::send( const Buffer &buffer )
{

}


void NSocket::send( const Buffer &buffer, const usize size )
{

}


int NSocket::receive( void ( *process )( NSocket &socket, Buffer &packet ) )
{
	return 0;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

bool NConnections::init( const u32 capacity )
{
	// Set state
	Assert( capacity >= 1 );
	this->capacity = capacity;
	this->current = 0;
	this->count = 0;

	// Allocate memory
	MemoryAssert( connections == nullptr );
	connections = reinterpret_cast<NConnection *>( memory_alloc( capacity * sizeof( NConnection ) ) );
	memory_set( connections, 0, capacity * sizeof( NConnection ) );
	for( u32 i = 0; i < capacity; i++ ) { connections[i].id = U32_MAX; }

	return true;
}


bool NConnections::free()
{
	// Free memory
	memory_free( connections );
	connections = nullptr;

	// Reset state
	capacity = 0;
	current = 0;
	count = 0;

	return true;
}


u32 NConnections::make_new()
{
	MemoryAssert( connections != nullptr );
	if( current == capacity ) { return U32_MAX; }

	NConnection &connection = connections[current];
	connection.id = current;
	connection.ip = "";

	while( ++current < capacity )
	{
		if( connections[current].id == U32_MAX ) { break; }
	}

	count++;
	return connection.id;
}


bool NConnections::release( const u32 id )
{
	MemoryAssert( connections != nullptr );
	if( id >= capacity ) { return false; }
	if( connections[id].id == U32_MAX ) { return true; }

	connections[id].id = U32_MAX;
	connections[id].ip = "";
	connections[id].socket.free();

	if( id < current ) { current = id; }

	count--;
	return true;
}


NConnection &NConnections::at( const u32 id )
{
	Assert( id < capacity );
	MemoryAssert( connections != nullptr );
	return connections[id];
}


const NConnection &NConnections::at( const u32 id ) const
{
	Assert( id < capacity );
	MemoryAssert( connections != nullptr );
	return connections[id];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NServer::init( const NetworkProtocol protocol, const u16 port, const u32 count )
{
	// Initialize Connections
	if( !connections.init( count ) ) { return false; }

	// Initialize Socket
	// ...

	return true;
}


bool NServer::free()
{
	// Free Socket
	// ...

	// Free Connections
	if( !connections.free() ) { return false; }

	return true;
}


u32 NServer::connection_accept()
{
	// Accept Incoming Connections

	return U32_MAX;
}


bool NServer::connection_disconnect( u32 id )
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif