#pragma once

#include <vendor/new.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/memory.hpp>
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class Factory
{
_PUBLIC:
#if MEMORY_RAII
	Factory( const u32 bucketSize = 64, const bool growable = true, const T &nullElement = { } )
		{ init( bucketSize, growable, nullElement ); }
	Factory( const Factory<T> &other ) { copy( other ); }
	Factory( Factory<T> &&other ) { move( static_cast<Factory<T> &&>( other ) ); }
	~Factory() { free(); }

	Factory<T> &operator=( const Factory<T> &other ) { return copy( other ); }
	Factory<T> &operator=( Factory<T> &&other ) { return move( static_cast<Factory<T> &&>( other ) ); }
#else
	Factory<T> &operator=( const Factory<T> &other ) { Error( "Factory: assignment disabled" ); return *this; }
	Factory<T> &operator=( Factory<T> &&other ) { Error( "Factory: assignment disabled" ); return *this; }

#if MEMORY_ASSERTS
	~Factory()
	{
		// Memory Leak Detection
		if( Debug::memoryLeakDetection && Debug::exitCode == 0 )
		{
			MemoryAssertMsg( buckets == nullptr, "ERROR: Memory leak in Factory (%p) (size: %.2f kb)",
			                 this, KB( size_allocated_bytes() ) );
		}
	}
#endif
#endif

_PRIVATE:
	using GenerationID = u16;
	static const u16 GENERATION_MAX = ( ( 1ULL << 15 ) - 1 );
	using BucketID = u16;
	static const u16 BUCKET_ID_MAX = U16_MAX;
	using BucketIndex = u32;
	static const u32 BUCKET_INDEX_MAX = U32_MAX;

	struct Handle
	{
		u64 alive : 1;
		u64 generation : 15;
		u64 bucket : 16;
		u64 index : 32;
	};

	struct Bucket
	{
		struct ElementSlot
		{
			Handle handle;
			T element;
		};

		Bucket( const BucketID id ) : id{ id } { };
		BucketID id = 0;

		ElementSlot *slots = nullptr;
		BucketIndex capacity = 0;
		BucketIndex current = 0;
		BucketIndex bottom = 0;
		BucketIndex top = 0;

		void init( const BucketIndex reserve = 1 )
		{
			// Set state
			capacity = reserve;
			current = 0;
			bottom = 0;
			top = 0;

			// Allocate memory
			MemoryAssert( slots == nullptr );
			Assert( capacity >= 1 );
			slots = reinterpret_cast<ElementSlot *>( memory_alloc( capacity * sizeof( ElementSlot ) ) );
			ErrorIf( slots == nullptr, "Failed to allocate memory for init Factory Bucket (%p: alloc %d bytes)",
			         slots, capacity * sizeof( ElementSlot ) );
			memory_set( slots, 0, capacity * sizeof( ElementSlot ) );
		}

		void free()
		{
			// Free elements
			MemoryAssert( slots != nullptr );
			for( BucketIndex i = bottom; i < top; i++ )
			{
				ElementSlot &slot = slots[i];
				if( !slot.handle.alive ) { continue; }
				slot.handle.alive = false;
				slot.element.~T();
			}

			// Free elements array
			memory_free( slots );
			slots = nullptr;

			// Reset state
			capacity = 0;
			current = 0;
			bottom = 0;
			top = 0;
		}

		Bucket &copy( const Bucket &other )
		{
			MemoryAssert( other.slots != nullptr );
			if( this == &other ) { return *this; }
			if( slots != nullptr ) { free(); }

			// Copy state
			capacity = other.capacity;
			current = other.current;
			bottom = other.bottom;
			top = other.top;

			// Allocate memory
			MemoryAssert( slots == nullptr );
			Assert( capacity >= 1 );
			slots = reinterpret_cast<ElementSlot *>( memory_alloc( capacity * sizeof( ElementSlot ) ) );
			ErrorIf( slots == nullptr, "Failed to allocate memory for copy Factory Bucket (%p: alloc %d bytes)",
					 slots, capacity * sizeof( ElementSlot ) );

			// Copy memory
			for( usize i = 0; i < capacity; i++ )
			{
				slots[i].handle = other.slots[i].handle;
				new ( &slots[i].element ) T( other.slots[i].element );
			}

			// Return self
			return *this;
		}

		Bucket &move( Bucket &&other )
		{
			MemoryAssert( other.slots != nullptr );
			if( this == &other ) { return *this; }
			if( slots != nullptr ) { free(); }

			// Move the other bucket's resources
			slots = other.slots;
			capacity = other.capacity;
			current = other.current;
			bottom = other.bottom;
			top = other.top;

			// Reset other bucket to a null state
			other.slots = nullptr;
			other.capacity = 0;
			other.current = 0;
			other.bottom = 0;
			other.top = 0;

			// Return self
			return *this;
		}

		void clear()
		{
			// Clear all elements in the bucket
			MemoryAssert( slots != nullptr );
			for( BucketIndex i = bottom; i < top; i++ )
			{
				ElementSlot &slot = slots[i];
				if( slot.handle.alive )
				{
					slot.handle.alive = false;
					slot.element.~T();
				}
			}

			// Reset state
			current = 0;
			bottom = 0;
			top = 0;
		}

		u64 add()
		{
			// Assign available slot
			MemoryAssert( slots != nullptr && current != capacity );
			ElementSlot &slot = slots[current];
			slot.handle.alive = true;
			slot.handle.bucket = id;
			slot.handle.generation++;
			slot.handle.index = current;

			// Default construct the new element
			new ( &slot.element ) T();

			// Update bottom, current, & top
			bottom = current < bottom ? current : bottom;
			while( ++current < capacity )
			{
				if( slots[current].handle.alive == false ) { break; }
			}
			top = current > top ? current : top;

			// Return element handle
			return reinterpret_cast<u64 &>( slot.handle );
		}

		u64 add( const T &element )
		{
			// Assign available slot
			MemoryAssert( slots != nullptr && current != capacity );
			ElementSlot &slot = slots[current];
			slot.handle.alive = true;
			slot.handle.bucket = id;
			slot.handle.generation++;
			slot.handle.index = current;

			// Default construct the new element
			new ( &slot.element ) T( element );

			// Update bottom, current, & top
			bottom = current < bottom ? current : bottom;
			while( ++current < capacity )
			{
				if( slots[current].handle.alive == false ) { break; }
			}
			top = current > top ? current : top;

			// Return element handle
			return reinterpret_cast<u64 &>( slot.handle );
		}

		u64 add( T &&element )
		{
			// Assign available slot
			MemoryAssert( slots != nullptr && current != capacity );
			ElementSlot &slot = slots[current];
			slot.handle.alive = true;
			slot.handle.bucket = id;
			slot.handle.generation++;
			slot.handle.index = current;

			// Default construct the new element
			new ( &slot.element ) T( static_cast<T &&>( element ) );

			// Update bottom, current, & top
			bottom = current < bottom ? current : bottom;
			while( ++current < capacity )
			{
				if( slots[current].handle.alive == false ) { break; }
			}
			top = current > top ? current : top;

			// Return element handle
			return reinterpret_cast<u64 &>( slot.handle );
		}

		bool remove( const BucketIndex index, const GenerationID generation )
		{
			// Valid element?
			MemoryAssert( slots != nullptr );
			if( index >= top ) { return false; }
			ElementSlot &slot = slots[index];
			if( slot.handle.alive == false ) { return false; }
			if( slot.handle.generation != generation ) { return false; }

			// Reset the slot
			slot.element.~T();
			slot.handle.alive = false;

			// Update current
			if( index < current ) { current = index; }

			// Update bottom
			if( index == bottom )
			{
				while( bottom < capacity && slots[bottom].handle.alive == false ) { bottom++; }
				if( bottom == capacity ) { bottom = 0; }
			}

			// Update top
			if( index == ( top - 1 ) )
			{
				while( top > 0 && slots[top - 1].handle.alive == false ) { top--; }
			}

			// Success
			return true;
		}

		T *get( const BucketIndex index, const GenerationID generation ) const
		{
			// Valid index?
			if( index >= top ) { return nullptr; }

			// Valid element?
			MemoryAssert( slots != nullptr );
			ElementSlot &slot = slots[index];
			if( !slot.handle.alive || slot.handle.generation != generation ) { return nullptr; }

			// Success
			return &slot.element;
		}

		bool contains( const BucketIndex index, const GenerationID generation ) const
		{
			// Valid element?
			MemoryAssert( slots != nullptr );
			if( index >= top ) { return false; }
			ElementSlot &slot = slots[index];
			if( slot.handle.alive == false ) { return false; }
			if( slot.handle.generation != generation ) { return false; }
			return true;
		}

		bool equals( const Bucket &other ) const
		{
			// NOTE: Equality here checks whether the elements match between containers

			// Ensure initialization
			if( slots == nullptr && other.slots == nullptr ) { return true; }
			if( slots == nullptr || other.slots == nullptr ) { return false; }

			// Compare states
			if ( id != other.id ||
			     capacity != other.capacity ||
			     bottom != other.bottom ||
			     top != other.top )
			{
				return false;
			}

			// Compare elements
			for( BucketIndex i = 0; i < top; i++ )
			{
				if( slots[i].handle.alive != other.slots[i].handle.alive ) { return false; }
				if( slots[i].handle.alive == false ) { continue; }
				if( slots[i].element != other.slots[i].element ) { return false; }
			}

			// Success
			return true;
		}

		usize size_allocated_bytes() const
		{
			if( slots == nullptr ) { return 0; }
			return capacity * sizeof( ElementSlot );
		}
	};

	void grow()
	{
		MemoryAssert( buckets != nullptr && capacity > 0 );
		Assert( capacity >= 1 && capacity < BUCKET_ID_MAX );

		// Reallocate memory
		capacity = capacity > BUCKET_ID_MAX / 2 ? BUCKET_ID_MAX : capacity * 2;
		buckets = reinterpret_cast<Bucket *>( memory_realloc( buckets, capacity * sizeof( Bucket ) ) );
		ErrorIf( buckets == nullptr, "Failed to reallocate memory for grow Factory (%p: alloc %d bytes)",
		         buckets, capacity * sizeof( Bucket ) );
	}

	bool new_bucket()
	{
		MemoryAssert( buckets != nullptr );
		if( !growable && current > 0 ) { return false; }
		if( current == BUCKET_ID_MAX ) { return false; }
		if( current == capacity ) { grow(); }

		// Allocate new bucket
		Bucket *bucket = &buckets[current];
		new ( bucket ) Bucket( current );
		bucket->init( bucketSize );
		current++;

		// Success
		return true;
	}

	Bucket *insertion_bucket()
	{
		// Find first bucket with room
		MemoryAssert( buckets != nullptr );
		Assert( recent < current );
		Bucket *bucket = &buckets[recent];
		if( bucket->current < bucket->capacity ) { return bucket; }

		// Recent bucket is full, so iterate over all buckets
		for( BucketID i = 0; i < current; i++ )
		{
			bucket = &buckets[i];
			if( bucket->current < bucket->capacity )
			{
				recent = i;
				return bucket;
			}
		}

		// No empty buckets, we need a new one
		if( !new_bucket() ) { return nullptr; }
		recent = current - 1;
		return &buckets[current - 1];
	}

_PUBLIC:
	void init( const u32 bucketSize = 64, const bool growable = true, const T &nullElement = { } )
	{
		// Set state
		this->capacity = 1;
		this->current = 0;
		this->recent = 0;
		this->elementCount = 0;
		this->bucketSize = bucketSize;
		this->growable = growable;
		new ( &this->null ) T( nullElement );

		// Allocate memory
		MemoryAssert( this->buckets == nullptr );
		Assert( this->bucketSize >= 1 );
		buckets = reinterpret_cast<Bucket *>( memory_alloc( this->capacity * sizeof( Bucket ) ) );
		ErrorIf( buckets == nullptr, "Failed to allocate memory for Factory buckets (%p: alloc %d bytes)",
		         buckets, capacity * sizeof( Bucket ) );

		// Initialize first bucket
		ErrorIf( !new_bucket(), "Failed to allocate first bucket for Factory" );
	}

	void free()
	{
		if( buckets == nullptr )
		{
		#if MEMORY_RAII
			return;
		#else
			MemoryWarning( "Factory: attempting to free() factory that is already freed!" ); return;
		#endif
		}

		// Free buckets
		for( BucketID i = 0; i < current; i++ ) { buckets[i].free(); }
		memory_free( buckets );
		buckets = nullptr;

		// Reset state
		capacity = 0;
		current = 0;
		recent = 0;
		elementCount = 0;
		bucketSize = 0;
		growable = false;
		null.~T();
	}

	Factory<T> &copy( const Factory<T> &other )
	{
		MemoryAssert( other.buckets != nullptr );
		if( this == &other ) { return *this; }
		if( buckets != nullptr ) { free(); }

		// Allocate the new buckets array
		buckets = reinterpret_cast<Bucket *>( memory_alloc( other.capacity * sizeof( Bucket ) ) );
		ErrorIf( buckets == nullptr, "Failed to allocate memory for copy Factory buckets" );

		// Copy each bucket from the other factory
		for( BucketID i = 0; i < other.current; i++ )
		{
			new ( &buckets[i] ) Bucket( other.buckets[i].id );
			buckets[i].copy( other.buckets[i] );
		}

		// Copy the state
		capacity = other.capacity;
		current = other.current;
		recent = other.recent;
		elementCount = other.elementCount;
		bucketSize = other.bucketSize;
		growable = other.growable;
		null.~T();
		null = other.null;

		// Return this
		return *this;
	}

	Factory<T> &move( Factory<T> &&other )
	{
		MemoryAssert( other.buckets != nullptr );
		if( this == &other ) { return *this; }
		if( buckets != nullptr ) { free(); }

		// Transfer the other Factory's state & resources
		buckets = other.buckets;
		capacity = other.capacity;
		current = other.current;
		recent = other.recent;
		elementCount = other.elementCount;
		bucketSize = other.bucketSize;
		growable = other.growable;
		null.~T();
		null = static_cast<T &&>( other.null );

		// Nullify the other factory
		other.buckets = nullptr;
		other.capacity = 0;
		other.current = 0;
		other.recent = 0;
		other.elementCount = 0;
		other.bucketSize = 0;
		other.growable = false;
		other.null.~T();

		// Return this
		return *this;
	}

	bool shrink()
	{
		// Try to shrink
		MemoryAssert( buckets != nullptr );
		BucketID top = current;
		while( top > 1 )
		{
			Bucket &bucket = buckets[top - 1];
			if( bucket.bottom != bucket.top ) { break; }
			bucket.free();
			top--;
		}
		if( top == current ) { return false; }

		// Resize factory
		capacity = top;
		current = top;
		recent = 0;
		buckets = reinterpret_cast<Bucket *>( memory_realloc( buckets, capacity * sizeof( Bucket ) ) );
		ErrorIf( buckets == nullptr, "Failed to reallocate memory for shrink Factory (%p: alloc %d bytes)",
		         buckets, capacity * sizeof( Bucket ) );

		// Success
		return true;
	}

	void clear()
	{
		MemoryAssert( buckets != nullptr );
		for( BucketID i = 0; i < current; i++ ) { buckets[i].clear(); }
		elementCount = 0;
		recent = 0;
	}

	u64 add()
	{
		MemoryAssert( buckets != nullptr );
		Bucket *bucket = insertion_bucket();
		if( bucket == nullptr ) { return 0; }
		const u64 handle = bucket->add();
		if( handle ) { elementCount++; return handle; } else { return 0; }
	}

	u64 add( const T &element )
	{
		MemoryAssert( buckets != nullptr );
		Bucket *bucket = insertion_bucket();
		if( bucket == nullptr ) { return 0; }
		const u64 handle = bucket->add( element );
		if( handle ) { elementCount++; return handle; } else { return 0; }
	}

	u64 add( T &&element )
	{
		MemoryAssert( buckets != nullptr );
		Bucket *bucket = insertion_bucket();
		if( bucket == nullptr ) { return 0; }
		const u64 handle = bucket->add( static_cast<T &&>( element ) );
		if( handle ) { elementCount++; return handle; } else { return 0; }
	}

	bool remove( const u64 handle )
	{
		MemoryAssert( buckets != nullptr );
		const Handle h = reinterpret_cast<const Handle &>( handle );
		if( h.alive == false ) { return false; }
		if( h.generation == 0 ) { return false; }
		if( h.bucket >= current ) { return false; }
		if( buckets[h.bucket].remove( h.index, h.generation ) )
		{
			elementCount--;
			recent = h.bucket;
			return true;
		}
		return false;
	}

	T &get( const u64 handle )
	{
		MemoryAssert( buckets != nullptr );
		const Handle h = reinterpret_cast<const Handle &>( handle );
		if( h.alive == false ) { return null; } // Handle isn't alive
		if( h.bucket >= current ) { return null; } // Handle points to invalid bucket id
		T *element = buckets[h.bucket].get( h.index, h.generation );
		if( element == nullptr ) { return null; } // Handle points to invalid element
		return *element;
	}

	const T &get( const u64 handle ) const
	{
		MemoryAssert( buckets != nullptr );
		const Handle h = reinterpret_cast<const Handle &>( handle );
		if( h.alive == false ) { return null; } // Handle isn't alive
		if( h.bucket >= current ) { return null; } // Handle points to invalid bucket id
		T *element = buckets[h.bucket].get( h.index, h.generation );
		if( element == nullptr ) { return null; } // Handle points to invalid element
		return *element;
	}

	bool contains( const u64 handle ) const
	{
		MemoryAssert( buckets != nullptr );
		const Handle h = reinterpret_cast<const Handle &>( handle );
		if( h.alive == false ) { return false; }
		if( h.generation == 0 ) { return false; }
		if( h.bucket >= current ) { return false; }
		return buckets[h.bucket].contains( h.index, h.generation );
	}

	bool equals( const Factory<T> &other ) const
	{
		// NOTE: Equality here checks whether the elements match between containers

		// Compare elements
		if( buckets == nullptr || other.buckets == nullptr ) { return false; }
		for( BucketID i = 0; i < current; i++ )
		{
			if( !buckets[i].equals( other.buckets[i] ) ) { return false; }
		}

		// Succcess
		return true;
	}

	usize count() const
	{
		return elementCount;
	}

	usize size_allocated_bytes() const
	{
		if( buckets == nullptr ) { return 0; }
		usize bytes = capacity * sizeof( Bucket );
		for( BucketID i = 0; i < current; i++ ) { bytes += buckets[i].size_allocated_bytes(); }
		return bytes;
	}

	// Copy & Move Assignment


	// Equality
    bool operator==( const Factory<T> &other ) const { return equals( other ); }
	bool operator!=( const Factory<T> &other ) const { return !equals( other ); }

	// Indexer
    T &operator[]( const u64 handle ) { return get( handle ); }
	const T &operator[]( const u64 handle ) const { return get( handle ); }

	// Initialization check
    explicit operator bool() const { return buckets != nullptr; }

	// Forward Iterator
	class forward_iterator
	{
	_PUBLIC:
		forward_iterator( Factory<T> &context, const bool begin )
			: context( context ), bucketID( 0 ), index{ 0 }
		{
			if( !begin ) { element = nullptr; return; } // end() state
			auto &slot = context.buckets[bucketID].slots[index];
			if( slot.handle.alive ) { element = &slot.element; } else { advance(); } // begin() state
		}
		forward_iterator &operator++() { advance(); return *this; }
		bool operator!=( const forward_iterator &other ) const { return element != other.element; }
		T &operator*() { MemoryAssert( element != nullptr ); return *element; }

	_PRIVATE:
		void advance()
		{
			// Increment index
			index++;

			// Find next alive element
			while( bucketID < context.current )
			{
				// Fetch bucket
				Bucket &bucket = context.buckets[bucketID];
				MemoryAssert( bucket.slots != nullptr );

				// Find next alive element
				while( index < bucket.top )
				{
					auto &slot = context.buckets[bucketID].slots[index];
					if( slot.handle.alive ) { element = &slot.element; return; } // Element found
					index++;
				}

				// End of bucket reached, increment to next bucket if possible
				bucketID++;
				if( bucketID < context.current ) { index = context.buckets[bucketID].bottom; }
			}

			// End of iteration
			element = nullptr;
		}

	_PRIVATE:
		Factory<T> &context;
		T *element;
		BucketID bucketID;
		BucketIndex index;
	};

	forward_iterator begin() { MemoryAssert( buckets != nullptr ); return forward_iterator( *this, true ); }
	forward_iterator end() { MemoryAssert( buckets != nullptr ); return forward_iterator( *this, false ); }
	forward_iterator begin() const { MemoryAssert( buckets != nullptr ); return forward_iterator( *this, true ); }
	forward_iterator end() const { MemoryAssert( buckets != nullptr ); return forward_iterator( *this, false ); }

	// Reverse Iterator
	class reverse_iterator
	{
	_PUBLIC:
		reverse_iterator( Factory<T> &context, const bool begin )
			: context( context )
		{
			if( !begin ) { element = nullptr; return; } // end() state
			bucketID = context.current;
			index = context.bucketSize;
			auto &slot = context.buckets[bucketID - 1].slots[index - 1];
			if( slot.handle.alive ) { element = &slot.element; } else { advance(); } // begin() state
		}
		reverse_iterator &operator++() { advance(); return *this; }
		bool operator!=( const reverse_iterator &other ) const { return element != other.element; }
		T &operator*() { MemoryAssert( element != nullptr ); return *element; }

	_PRIVATE:
		void advance()
		{
			// Decrement index
			index--;

			// Find next alive element
			while( bucketID > 0 )
			{
				// Fetch bucket
				Bucket &bucket = context.buckets[bucketID - 1];
				MemoryAssert( bucket.slots != nullptr );

				// Find next alive element
				while( index > 0 )
				{
					auto &slot = context.buckets[bucketID - 1].slots[index - 1];
					if( slot.handle.alive ) { element = &slot.element; return; } // Element found
					index--;
				}

				// End of bucket reached, increment to next bucket if possible
				bucketID--;
				if( bucketID > 0 ) { index = context.buckets[bucketID - 1].top; }
			}

			// End of iteration
			element = nullptr;
		}

	_PRIVATE:
		Factory<T> &context;
		T *element;
		BucketID bucketID;
		BucketIndex index;
	};

	reverse_iterator rbegin() { MemoryAssert( buckets != nullptr ); return reverse_iterator( *this, true ); }
	reverse_iterator rend() { MemoryAssert( buckets != nullptr ); return reverse_iterator( *this, false ); }
	reverse_iterator rbegin() const { MemoryAssert( buckets != nullptr ); return reverse_iterator( *this, true ); }
	reverse_iterator rend() const { MemoryAssert( buckets != nullptr ); return reverse_iterator( *this, false ); }

_PUBLIC:
	static void write( Buffer &buffer, const Factory<T> &factory )
	{
		MemoryAssert( buffer.data != nullptr );
		MemoryAssert( factory.buckets != nullptr );

		// Save state
		buffer.write<BucketID>( factory.capacity );
		buffer.write<BucketID>( factory.current );
		buffer.write<BucketID>( factory.recent );
		buffer.write<usize>( factory.elementCount );
		buffer.write<BucketIndex>( factory.bucketSize );
		buffer.write<bool>( factory.growable );
		buffer.write<T>( factory.null );

		// Save buckets
		for( BucketID i = 0; i < factory.current; i++ )
		{
			const Bucket &bucket = factory.buckets[i];
			MemoryAssert( bucket.slots != nullptr );
			buffer.write<BucketIndex>( bucket.capacity );
			buffer.write<BucketIndex>( bucket.current );
			buffer.write<BucketIndex>( bucket.bottom );
			buffer.write<BucketIndex>( bucket.top );

			// Save elements
			for( BucketIndex j = 0; j < bucket.current; j++ )
			{
				const typename Bucket::ElementSlot &slot = bucket.slots[j];
				const u64 handle = *reinterpret_cast<const u64 *>( &slot.handle );
				buffer.write<u64>( handle );
				buffer.write<T>( slot.element );
			}
		}
	}

	static void read( Buffer &buffer, Factory<T> &factory )
	{
		if( factory.buckets != nullptr ) { factory.free(); }
		MemoryAssert( factory.buckets == nullptr );

		// Read state
		buffer.read<BucketID>( factory.capacity );
		buffer.read<BucketID>( factory.current );
		buffer.read<BucketID>( factory.recent );
		buffer.read<usize>( factory.elementCount );
		buffer.read<BucketIndex>( factory.bucketSize );
		buffer.read<bool>( factory.growable );
		buffer.read<T>( factory.null );

		// Initialize memory
		Assert( factory.bucketSize >= 1 );
		factory.buckets = reinterpret_cast<Bucket *>( memory_alloc( factory.capacity * sizeof( Bucket ) ) );
		ErrorIf( factory.buckets == nullptr, "Failed to allocate memory for 'read' Factory buckets (%p: alloc %d bytes)",
		         factory.buckets, factory.capacity * sizeof( Bucket ) );

		// Read buckets
		for( BucketID i = 0; i < factory.current; i++ )
		{
			// Initialize bucket
			Bucket &bucket = factory.buckets[i];
			new ( &bucket ) Bucket( i );
			bucket.init( factory.bucketSize );
			MemoryAssert( bucket.slots != nullptr );

			// Read bucket state
			buffer.read<BucketIndex>( bucket.capacity );
			buffer.read<BucketIndex>( bucket.current );
			buffer.read<BucketIndex>( bucket.bottom );
			buffer.read<BucketIndex>( bucket.top );

			// Read elements
			for( BucketIndex j = 0; j < bucket.current; j++ )
			{
				typename Bucket::ElementSlot &slot = bucket.slots[j];
				const u64 handle = buffer.read<u64>();
				slot.handle = *reinterpret_cast<const Handle *>( &handle );
				buffer.read<T>( slot.element );
			}
		}
	}

	static void serialize( Buffer &buffer, const Factory<T> &factory )
	{
		// TODO: Implement Factory serialize() function
		Error( "TODO: Implement this!" );
	}

	static void deserialize( Buffer &buffer, const Factory<T> &factory )
	{
		// TODO: Implement Factory deserialize() function
		Error( "TODO: Implement this!" );
	}

_PRIVATE:
	Bucket *buckets = nullptr;
	usize elementCount = 0;
	BucketIndex bucketSize = 0;
	BucketID capacity = 0;
	BucketID current = 0;
	BucketID recent = 0;
	bool growable = false;
	T null;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////