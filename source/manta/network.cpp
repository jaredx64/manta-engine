#include <manta/network.hpp>

#if 0
#include <core/checksum.hpp>
#include <manta/timer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void packet_shift_buffer( Socket &socket, const int shift, const int size )
{
#if NETWORKING
	memmove( socket.packet_buffer.data, socket.packet_buffer.data + shift, size );
	socket.packet_size_validation_offset = PACKET_HEADER_DELIMITER_SIZE;
	socket.packet_buffer.tell -= shift;
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
			// We've reached the end of packet_buffer where there can't possibly be a PACKET_HEADER_DELIMITER (not enough bytes left)
			if( byteOffset > socket.packet_buffer.tell - PACKET_HEADER_DELIMITER_SIZE )
			{
				byteOffset = socket.packet_buffer.tell;
				break;
			}

			// We've found a new PACKET_HEADER_DELIMITER
			socket.packet_delimiter = buffer_peek<uint64>( socket.packet_buffer, byteOffset );
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


static bool packet_validate_size( Socket &socket, const int offset )
{
#if NETWORKING
	// We can only check for size spoofing if we have enough bytes for the delimiter search
	const int scan_size = offset + min( static_cast<int>( PACKET_HEADER_SIZE + socket.packet_length ) - offset, socket.packet_buffer.tell - offset );
	if( scan_size - socket.packet_size_validation_offset < PACKET_HEADER_DELIMITER_SIZE ) { return true; }

	// Check for packet size spoofing (only on exceedingly large packets)
	if( socket.packet_length != 0 )
	{
		for( int i = socket.packet_size_validation_offset; i < scan_size; i++ )
		{
			socket.packet_delimiter = buffer_peek<uint64>( socket.packet_buffer, i );
			if( UNLIKELY( socket.packet_delimiter == PACKET_HEADER_DELIMITER ) )
			{
				// We've found a PACKET_HEADER_DELIMITER where data should be... either the 1 in 2^64 odds of this happening occurred or
				// we're being packet size spoofed. Let's just drop this packet and start from the new position
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


void packet_process_buffer( Socket &socket, char *data, const int data_size, void (*process)( Socket &socket, Buffer &packet ) )
{
#if NETWORKING
	// Receive packet byte stream
	const int data_block_start_position = socket.packet_buffer.tell;
	buffered_write_buffer( socket.packet_buffer, data, data_size );

	// Check for size spoofing
	packet_validate_size( socket, data_block_start_position );

	for( ;; )
	{
		if( socket.packet_length == 0 )
		{
			// Check if we have enough data for a header
			if( socket.packet_buffer.tell < PACKET_HEADER_SIZE ) { break; }

			// Read header
			socket.packet_delimiter = buffer_peek<uint64>( socket.packet_buffer, 0 );
			socket.packet_length    = buffer_peek<uint32>( socket.packet_buffer, PACKET_HEADER_DELIMITER_SIZE );
			socket.packet_checksum  = buffer_peek<uint32>( socket.packet_buffer, PACKET_HEADER_DELIMITER_SIZE + PACKET_HEADER_LENGTH_SIZE );

			// Validate Delimiter
			if( UNLIKELY( !packet_validate_delimiter( socket ) ) ) { continue; }
		}

		// Check for size spoofing
		if( UNLIKELY( !packet_validate_size( socket, PACKET_HEADER_DELIMITER_SIZE ) ) ) { continue; }

		// We have header info, so let's check if we have the full packet
		const int packet_length_total = socket.packet_length + PACKET_HEADER_SIZE;
		const int excess_data_size    = socket.packet_buffer.tell - packet_length_total;
		if( socket.packet_buffer.tell < packet_length_total ) { break; }

		// Seek the buffer to a good position for reading
		buffer_seek( socket.packet_buffer, PACKET_HEADER_SIZE );

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
		socket.packet_buffer.tell = excess_data_size;

		// Break the loop if there is no more readable excess
		if( excess_data_size < PACKET_HEADER_SIZE ) { break; }
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
