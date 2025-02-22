#pragma once

#include <manta/gfx.hpp>

#include <core/memory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, u32 Capacity>
class GfxResourceFactory
{
public:
	void init()
	{
		Assert( data == nullptr );
		data = reinterpret_cast<T *>( memory_alloc( Capacity * sizeof( T ) ) );
		for( u32 i = 0; i < Capacity; i++ ) { resource( i ).id = GFX_RESOURCE_ID_NULL; }
		current = 0;
	}

	void free()
	{
		if( data != nullptr ) { memory_free( data ); }
		data = nullptr;
		current = 0;
	}

	T *make_new()
	{
		ErrorIf( current == Capacity,
			"Failed to allocate new GfxResource (reached GfxResourceFactory limit: %u)", Capacity );
		T *element = &data[current];
		new ( element ) T();
		reinterpret_cast<GfxResource *>( element )->id = current;
		while( current < Capacity && resource( current ).id != GFX_RESOURCE_ID_NULL ) { current++; }
		return element;
	}

	void remove( const u32 index )
	{
		Assert( index < Capacity );
		data[index].~T();
		resource( index ).id = GFX_RESOURCE_ID_NULL;
		if( index < current ) { current = index; }
	}

	struct forward_iterator
	{
		forward_iterator( T *data, u32 cur, u32 end ) :
			data{ data }, cur{ cur }, end{ end } { cur = find_next( cur ); } // begin()
		forward_iterator( T *data, u32 end ) :
			data{ data }, cur{ end }, end{ end } { } // end()

		u32 find_next( const u32 current )
		{
			cur = current;
			while( cur != end && reinterpret_cast<GfxResource *>( &data[cur] )->id == GFX_RESOURCE_ID_NULL )
			{
				cur++;
			}
			return cur;
		}

		T &operator*() { return data[cur]; }
		bool operator!=( const forward_iterator &other ) const { return cur != other.cur; }
		forward_iterator &operator++() { cur = find_next( cur + 1 ); return *this; }

		T *data = nullptr;
		u32 cur, end;
	};

	inline bool exists( const u32 index ) const { return resource( index ).id != GFX_RESOURCE_ID_NULL; }

	T &get( const u32 index )
	{
		Assert( index < Capacity );
		return data[index];
	}

	forward_iterator begin() { return forward_iterator( data, 0, Capacity ); }
	forward_iterator end() { return forward_iterator( data, Capacity ); }
	forward_iterator begin() const { return forward_iterator( data, 0, Capacity ); }
	forward_iterator end() const { return forward_iterator( data, Capacity ); }

private:
	inline GfxResource &resource( const u32 index ) { return *reinterpret_cast<GfxResource *>( &data[index] ); }

	T *data = nullptr;
	u32 current = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////