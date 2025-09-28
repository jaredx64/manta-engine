#include <manta/objects.hpp>

#include <core/debug.hpp>
#include <core/memory.hpp>
#include <core/buffer.hpp>
#include <core/serializer.hpp>

#include <vendor/vendor.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NULL_BUCKET ( 0 )

#define NULL_TYPE ( 0 )

#define TYPE_BUCKET(context,type) \
	CoreObjects::CATEGORY_TYPE_BUCKET[context][type]

#define TYPE_VALID(context,type) \
	( type > 0 && type < ObjectType::OBJECT_TYPE_COUNT && TYPE_BUCKET( context, type ) != 0 )

#define TYPE_INVALID(context,type) \
	UNLIKELY( !TYPE_VALID( context, type ) )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool object_is_parent_of( const ObjectType thisType, const ObjectType otherType )
{
	if( thisType >= otherType ) { return false; }
	if( thisType == ObjectType::DEFAULT || otherType == ObjectType::DEFAULT ) { return false; }
	if(	thisType >= ObjectType::OBJECT_TYPE_COUNT || otherType >= ObjectType::OBJECT_TYPE_COUNT ) { return false; }
	const u16 thisDepth = CoreObjects::TYPE_INHERITANCE_DEPTH[thisType];
	if( CoreObjects::TYPE_INHERITANCE_DEPTH[otherType] <= thisDepth ) { return false; }

	for( ObjectType_t type = thisType + 1; type < otherType; type++ )
	{
		if( CoreObjects::TYPE_INHERITANCE_DEPTH[type] <= thisDepth ) { return false; }
	}

	return true;
}

bool object_is_child_of( const ObjectType thisType, const ObjectType otherType )
{
	return object_is_parent_of( otherType, thisType );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Object::Serialization::InstanceTable Object::Serialization::instances[ObjectType::OBJECT_TYPE_COUNT];
const ObjectContext *Object::Serialization::context = nullptr;
bool Object::Serialization::dirty = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Object::Serialization::InstanceTable::init( const u32 reserve )
{
	if( keys != nullptr ) { free(); }
	capacity = reserve;
	current = 0;
	if( capacity == 0 ) { return; }
	keys = reinterpret_cast<u32 *>( memory_alloc( capacity * sizeof( u32 ) ) );
}


void Object::Serialization::InstanceTable::free()
{
	if( keys == nullptr ) { return; }
	memory_free( keys );
	keys = nullptr;
	capacity = 0;
	current = 0;
}


void Object::Serialization::InstanceTable::add( const u32 key )
{
	MemoryAssert( keys != nullptr );
	Assert( current < capacity );
	keys[current++] = key;
}


u32 Object::Serialization::InstanceTable::get_key( const u32 instance ) const
{
	MemoryAssert( keys != nullptr );
	if( instance >= capacity ) { return U32_MAX; }
	return keys[instance];
}


u32 Object::Serialization::InstanceTable::find_instance( const u32 key ) const
{
	MemoryAssert( keys != nullptr );

    u32 left = 0;
    u32 right = current;

	while( left < right )
	{
		const u32 pivot = left + ( right - left ) / 2;
		if( keys[pivot] == key ) { return pivot; }
		if( keys[pivot] < key ) { left = pivot + 1; } else { right = pivot; }
	}

	return U32_MAX;
}


void Object::Serialization::init()
{
	context = nullptr;
}


void Object::Serialization::free()
{
	for( ObjectType_t type = 0; type < ObjectType::OBJECT_TYPE_COUNT; type++ ) { instances[type].free(); }
	context = nullptr;
}


void Object::Serialization::prepare( const ObjectContext &activeContext )
{
	// State
	if( context == &activeContext && !dirty ) { return; }
	context = &activeContext;
	dirty = false;

	// Generate instance table from objects
	for( ObjectType_t type = 0; type < ObjectType::OBJECT_TYPE_COUNT; type++ )
	{
		// Initialize instance table
		if( !CoreObjects::TYPE_SERIALIZED[type] ) { continue; }
		const u16 bucketID = TYPE_BUCKET( context->category, type );
		if( bucketID == NULL_BUCKET ) { continue; }
		instances[type].init( context->count( type ) );

		// Cache object 'keys' (u32: bucketID | index)
		ObjectContext::ObjectBucket *bucket = &context->buckets[bucketID];
		for( ;; )
		{
			// Ensure the bucket is our type
			if( UNLIKELY( bucket->type != type ) ) { break; }

			// Loop over live objects in the bucket and cache them
			for( u16 index = bucket->bottom; index < bucket->top; index++ )
			{
				const Object &object = bucket->get_object_id( index );
				if( !object.alive ) { continue; }
				const u32 key = ( ( object.bucketID & 0xFFFF ) << 16 ) | ( object.index & 0xFFFF );
				instances[type].add( key );
			}

			// Move to next bucket
			if( bucket->bucketIDNext == NULL_BUCKET ) { break; }
			if( context->buckets[bucket->bucketIDNext].type != bucket->type ) { break; }
			bucket = &context->buckets[bucket->bucketIDNext];
		}
	}
}


u32 Object::Serialization::instance_from_object( const Object &object )
{
	AssertMsg( context != nullptr, "Missing Object::Serialization::prepare()...end() block!" );
	MemoryAssert( context->buckets != nullptr );
	if( TYPE_INVALID( context->category, object.type ) ) { return U32_MAX; }
	if( !CoreObjects::TYPE_SERIALIZED[object.type] ) { return U32_MAX; }
	if( !context->exists( object ) ) { return U32_MAX; };

	// Find instance
	const u32 key = ( ( object.bucketID & 0xFFFF ) << 16 ) | ( object.index & 0xFFFF );
	return instances[object.type].find_instance( key );
}


Object Object::Serialization::object_from_instance( const ObjectType type, const u32 instance )
{
	AssertMsg( context != nullptr, "Missing Object::Serialization::prepare()...end() block!" );
	MemoryAssert( context->buckets != nullptr );
	if( TYPE_INVALID( context->category, type ) ) { return Object { }; }
	if( !CoreObjects::TYPE_SERIALIZED[type] ) { return Object { }; }

	// Find object
	const u32 key = instances[type].get_key( instance );
	if( key == U32_MAX ) { return Object { }; }
	const u16 bucketID = ( key >> 16 ) & 0xFFFF;
	const u16 index = ( key ) & 0xFFFF;
	if( context->buckets[bucketID].data == nullptr || context->buckets[bucketID].type != type ) { return Object { }; }
	return context->buckets[bucketID].get_object_id( index );
}


void Object::serialize( Buffer &buffer, const Object &object )
{
	SerializedObject serialized { 0, 0 };
	if( CoreObjects::TYPE_SERIALIZED[object.type] )
	{
		const u32 instance = Object::Serialization::instance_from_object( object );
		if( instance != U32_MAX ) { serialized = { CoreObjects::TYPE_HASH[object.type], instance }; }
	}
	buffer.write<SerializedObject>( serialized );
}


void Object::deserialize( Buffer &buffer, Object &object )
{
	SerializedObject serialized;
	buffer.read<SerializedObject>( serialized );

	// Null hash?
	if( serialized.hash == 0 ) { object = Object { }; return; }

	// Attempt resolve type from hash & deserialize
	for( ObjectType_t type = 0; type < ObjectType::OBJECT_TYPE_COUNT; type++ )
	{
		if( CoreObjects::TYPE_HASH[type] == serialized.hash )
		{
			if( !CoreObjects::TYPE_SERIALIZED[type] ) { break; }
			object = Object::Serialization::object_from_instance( type, serialized.instance );
			return;
		}
	}

	// Failed
	object = Object { };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if COMPILE_DEBUG

u64 Object::id() const
{
	u64 result = 0;
	result |= static_cast<u64>( alive ) << 63;
	result |= static_cast<u64>( type ) << 48;
	result |= static_cast<u64>( generation ) << 32;
	result |= static_cast<u64>( bucketID ) << 16;
	result |= static_cast<u64>( index );
	return result;
}

#include <vendor/stdio.hpp>
void Object::id_string( char *buffer, const usize length ) const
{
	if( alive )
	{
		const char *name = type < ObjectType::OBJECT_TYPE_COUNT ? CoreObjects::TYPE_NAME[type] : "invalid";
		snprintf( buffer, length, "[%s (%u), b: %u, i: %u, g: %u]", name, type, bucketID, index, generation );
	}
	else
	{
		snprintf( buffer, length, "[null]" );
	}
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ObjectContext::init()
{
	// State
	capacity = CoreObjects::CATEGORY_TYPE_COUNT[category];
	current = CoreObjects::CATEGORY_TYPE_COUNT[category];
	disableEvents = false;

	// Allocate Memory
	buckets = reinterpret_cast<ObjectBucket *>( memory_alloc( capacity * sizeof( ObjectBucket ) ) );
	objectCount = reinterpret_cast<u32 *>( memory_alloc( capacity * sizeof( u32 ) ) );
	bucketCache = reinterpret_cast<u16 *>( memory_alloc( capacity * sizeof( u16 ) ) );

	// Zero memory
	memory_set( objectCount, 0, capacity * sizeof( u32 ) );
	memory_set( bucketCache, 0, capacity * sizeof( u16 ) );

	// Default-initialize ObjectBuckets for every object type
	for( u16 i = capacity; i > 0; i-- )
	{
		const u16 bucketID = i - 1;
		ObjectBucket *bucket = &buckets[bucketID];
		new ( bucket ) ObjectBucket{ *this };
		bucket->bucketID = bucketID;
		bucket->type = CoreObjects::CATEGORY_TYPES[category][bucketID];
		bucket->bucketIDNext = i == capacity ? NULL_BUCKET : i;
		bucketCache[TYPE_BUCKET( category, bucket->type )] = bucketID;
	}


	// Success
	return true;
}


bool ObjectContext::free()
{
	// Already freed?
	if( buckets == nullptr ) { return true; }

	// Destroy all objects
	destroy_all();

	// Free ObjectBucket's
	for( u16 bucketID = 0; bucketID < current; bucketID++ ) { buckets[bucketID].free(); }

	// Free memory
	if( buckets != nullptr )
	{
		memory_free( buckets );
		buckets = nullptr;
	}

	if( objectCount != nullptr )
	{
		memory_free( objectCount );
		objectCount = nullptr;
	}

	if( bucketCache != nullptr )
	{
		memory_free( bucketCache );
		bucketCache = nullptr;
	}

	// Reset state
	capacity = 0;
	current = 0;

	// Success
	return true;
}


bool ObjectContext::grow()
{
	// Calculate new capacity
	if( capacity == U16_MAX ) { return false; }
	u32 newCapacity = ( capacity == 0 ? 1 : capacity * 2 );
	newCapacity = ( newCapacity > U16_MAX ? U16_MAX : newCapacity );

	// Realloc 'buckets' buffer
	buckets = reinterpret_cast<ObjectBucket *>( memory_realloc( buckets, newCapacity * sizeof( ObjectBucket ) ) );
	if( buckets == nullptr ) { return false; }

	// Default initialize new section of buffer
	for( u16 bucketID = capacity; bucketID < static_cast<u16>( newCapacity ); bucketID++ )
	{
		new ( &buckets[bucketID] ) ObjectBucket { *this };
	}

	// Update capacity
	capacity = static_cast<u16>( newCapacity );

	// Success
	return true;
}


u16 ObjectContext::new_bucket( const ObjectType_t type )
{
	// Grow buffer?
	if( current == capacity )
	{
		if( !grow() ) { return NULL_BUCKET; }
	}

	// Initialize new bucket
	if( !buckets[current].init( type ) ) { return NULL_BUCKET; }
	buckets[current].bucketID = current;

	// Success
	return current++;
}


ObjectContext::ObjectBucket *ObjectContext::new_object( const ObjectType_t type )
{
	// Validate type
	if( TYPE_INVALID( category, type ) ) { return nullptr; }

	// At type capacity?
	if( count( type ) == CoreObjects::TYPE_MAX_COUNT[type] ) { return nullptr; }

	// Lazy initialize first bucket for each object type
	ObjectBucket *bucket = &buckets[bucketCache[TYPE_BUCKET( category, type )]];
	if( bucket->data == nullptr ) { bucket->init( type ); }

	// Find first bucket with room
	for( ;; )
	{
		// Early out if the bucket still has capacity
		if( LIKELY( bucket->current < CoreObjects::TYPE_BUCKET_CAPACITY[type] ) ) { break; }

		// This bucket is full, but a 'next' bucket of our type already exists
		if( bucket->bucketIDNext != NULL_BUCKET && buckets[bucket->bucketIDNext].type == bucket->type )
		{
			bucket = &buckets[bucket->bucketIDNext];
			continue;
		}

		// No existing buckets with room, lets make a new bucket
		const u16 bucketIDPre = bucket->bucketID; // Stored as new_bucket() realloc can cause pointer invalidation
		const u16 bucketIDNew = new_bucket( type );
		if( UNLIKELY( bucketIDNew == NULL_BUCKET ) ) { return nullptr; } // ObjectContext completely full of buckets
		bucket = &buckets[bucketIDNew];

		// Update bucket links
		bucket->bucketIDNext = buckets[bucketIDPre].bucketIDNext;
		buckets[bucketIDPre].bucketIDNext = bucketIDNew;

		// End loop
		break;
	}

	// Add object
	return bucket;
}


byte *ObjectContext::get_object_pointer( const Object &object ) const
{
	// Fetch Bucket
	MemoryAssert( buckets != nullptr );
	if( TYPE_INVALID( category, object.type ) ) { return nullptr; }
	if( UNLIKELY( object.bucketID >= current ) ) { return nullptr; }
	ObjectBucket *bucket = &buckets[object.bucketID];
	if( UNLIKELY( bucket->type != object.type ) ) { return nullptr; }
	if( UNLIKELY( bucket->data == nullptr ) ) { return nullptr; }

	// Get Object Pointer
	return bucket->get_object_pointer( object.index, object.generation );
}


Object ObjectContext::create( const ObjectType type )
{
	Assert( type < ObjectType::OBJECT_TYPE_COUNT );

	// Find Available Bucket
	ObjectBucket *bucket = new_object( type );
	if( UNLIKELY( bucket == nullptr ) ) { return Object { }; }

	// Create Object
	void *const object = bucket->data + ( bucket->current * CoreObjects::TYPE_SIZE[type] );
	CoreObjects::TYPE_CONSTRUCT[type]( object ); // Constructor
	return bucket->new_object( object );
}


bool ObjectContext::destroy( Object &object )
{
	// Fetch Bucket
	if( UNLIKELY( object.bucketID >= current ) ) { return false; }
	ObjectBucket *bucket = &buckets[object.bucketID];
	if( TYPE_INVALID( category, object.type ) ) { return false; }
	if( UNLIKELY( bucket->type != object.type ) ) { return false; }
	if( UNLIKELY( bucket->data == nullptr ) ) { return false; }

	// Remove Object
	const bool success = bucket->delete_object( object.index, object.generation );
	return success;
}


void ObjectContext::destroy_all()
{
	// Free all buckets
	for( u16 bucketID = 0; bucketID < current; bucketID++ )
	{
		ObjectBucket *bucket = &buckets[bucketID];
		bucket->clear();
		bucket->free();
	}

	// Reset current
	current = CoreObjects::CATEGORY_TYPE_COUNT[category];

	// Default-initialize ObjectBuckets for every object type
	for( u16 i = current; i > 0; i-- )
	{
		const u16 bucketID = i - 1;
		ObjectBucket *bucket = &buckets[bucketID];
		new ( bucket ) ObjectBucket{ *this };
		bucket->bucketID = bucketID;
		bucket->type = CoreObjects::CATEGORY_TYPES[category][bucketID];
		bucket->bucketIDNext = i == current ? NULL_BUCKET : i;
		bucketCache[TYPE_BUCKET( category, bucket->type )] = bucketID;
	}
}


void ObjectContext::destroy_all_type( const ObjectType type )
{
	// Free buckets
	for( u16 bucketID = 0; bucketID < current; bucketID++ )
	{
		if( buckets[bucketID].type != type ) { continue; }
		buckets[bucketID].clear();
	}
}


bool ObjectContext::activate( Object &object, const bool setActive )
{
	MemoryAssert( buckets != nullptr );
	CoreObjects::DEFAULT_t *instance =
		reinterpret_cast<CoreObjects::DEFAULT_t *>( get_object_pointer( object ) );
	if( instance == nullptr ) { return false; }
	instance->id.deactivated = !setActive;
	object.deactivated = !setActive;
	return true;
}


void ObjectContext::activate_all( const bool setActive )
{
	MemoryAssert( buckets != nullptr );
	for( ObjectType_t type = 1; type < CoreObjects::CATEGORY_TYPE_COUNT[category]; type++ )
	{
		activate_all_type( type, setActive );
	}
}


void ObjectContext::activate_all_type( const ObjectType type, const bool setActive )
{
	MemoryAssert( buckets != nullptr );
	if( TYPE_INVALID( category, type ) ) { return; }

	// Activate all objects
	const u16 bucketID = TYPE_BUCKET( category, type );
	ObjectContext::ObjectBucket *bucket = &buckets[bucketID];
	for( ;; )
	{
		// Ensure the bucket is our type
		if( UNLIKELY( bucket->type != type ) ) { break; }

		// Loop over live objects in the bucket and cache them
		for( u16 i = bucket->bottom; i < bucket->top; i++ )
		{
			byte *objectPtr = bucket->data + i * CoreObjects::TYPE_SIZE[type];
			CoreObjects::DEFAULT_t &object =
				*reinterpret_cast<CoreObjects::DEFAULT_t *>( objectPtr );
			object.id.deactivated = !setActive;
		}

		// Move to next bucket
		if( bucket->bucketIDNext == NULL_BUCKET ) { break; }
		if( buckets[bucket->bucketIDNext].type != bucket->type ) { break; }
		bucket = &buckets[bucket->bucketIDNext];
	}
}


u32 ObjectContext::count( const ObjectType type ) const
{
	MemoryAssert( objectCount != nullptr );
	return objectCount[TYPE_BUCKET( category, type )];
}


u32 ObjectContext::count_all() const
{
	MemoryAssert( objectCount != nullptr );
	return objectCount[NULL_TYPE];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ObjectContext::write( Buffer &buffer, const ObjectContext &context )
{
	// TODO
}


void ObjectContext::read( Buffer &buffer, ObjectContext &context )
{
	// TODO
}


void ObjectContext::serialize( Buffer &buffer, const ObjectContext &context )
{
	Assert( context.initialized() );

	Serializer serializer;
	serializer.begin( buffer, 0 );
	CoreObjects::serialize( serializer, context );
	serializer.end();
}


void ObjectContext::deserialize( Buffer &buffer, ObjectContext &context )
{
	if( context.initialized() ) { context.free(); context.init(); }

	context.disableEvents = true;
	Deserializer deserializer;
	deserializer.begin( buffer, 0 );
	CoreObjects::deserialize( deserializer, context );
	deserializer.end();
	context.disableEvents = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ObjectContext::ObjectBucket::init( const ObjectType_t type )
{
	// State
	this->type = type;
	this->current = 0;
	this->top = 0;
	this->bottom = 0;

	// Allocate Memory
	data = reinterpret_cast<byte *>( memory_alloc( CoreObjects::TYPE_BUCKET_CAPACITY[type] *
		CoreObjects::TYPE_SIZE[type] ) );
	memory_set( data, 0, CoreObjects::TYPE_BUCKET_CAPACITY[type] * CoreObjects::TYPE_SIZE[type] );
	return true;
}


void ObjectContext::ObjectBucket::free()
{
	// Free memory
	if( data == nullptr ) { return; }
	memory_free( data );
	data = nullptr;
}


void ObjectContext::ObjectBucket::clear()
{
	// Destroy objects
	if( data == nullptr ) { return; }
	for( u32 i = 0; i < top; i++ )
	{
		byte *const objectPtr = data + i * CoreObjects::TYPE_SIZE[type];
		delete_object( i, reinterpret_cast<CoreObjects::DEFAULT_t *>( objectPtr )->id.generation );
	}
}


void *ObjectContext::ObjectBucket::new_object_pointer()
{
	// At capacity?
	//MemoryAssert( data != nullptr );
	//if( UNLIKELY( current == CoreObjects::TYPE_BUCKET_CAPACITY[type] ) ) { return nullptr; }

	// Return Pointer
	return data + current * CoreObjects::TYPE_SIZE[type];
}


Object ObjectContext::ObjectBucket::new_object( void *ptr )
{
	// Object
	CoreObjects::DEFAULT_t *object = reinterpret_cast<CoreObjects::DEFAULT_t *>( ptr );

	// Set Object
	const u16 generation = object->id.generation + 1;
	object->id = { type, generation, bucketID, current };
	object->id.alive = true;

	// Update bottom, current, & top
	bottom = current < bottom ? current : bottom;
	while( ++current < CoreObjects::TYPE_BUCKET_CAPACITY[type] )
	{
		const byte *const objectPtr = data + current * CoreObjects::TYPE_SIZE[type];
		if( reinterpret_cast<const CoreObjects::DEFAULT_t *>( objectPtr )->id.alive == false ) { break; }
	}
	top = current > top ? current : top;

	// Increment Object Count
	context.objectCount[TYPE_BUCKET( context.category, type )]++;
	context.objectCount[NULL_TYPE]++; // total object count
	Object::Serialization::dirty |= ( Object::Serialization::context == &context );

	// Cache bucket
	context.bucketCache[TYPE_BUCKET( context.category, type )] = bucketID;

	// Create Event
	if( LIKELY( !context.disableEvents ) ) { object->event_create(); }

	// Success
	return object->id;
}


bool ObjectContext::ObjectBucket::delete_object( const u16 index, const u16 generation )
{
	// Verify alive
	MemoryAssert( data != nullptr );
	Assert( index < CoreObjects::TYPE_BUCKET_CAPACITY[type] );
	byte *const objectPtr = data + index * CoreObjects::TYPE_SIZE[type];
	CoreObjects::DEFAULT_t *object = reinterpret_cast<CoreObjects::DEFAULT_t *>( objectPtr );
	if( UNLIKELY( object->id.alive == false ) ) { return false; }
	if( UNLIKELY( object->id.generation != generation ) ) { return false; }

	// Destroy event
	object->event_destroy();

	// Destructor
	CoreObjects::TYPE_DESTRUCT[type]( object );

	// Mark dead
	object->id.alive = false;

	// Update current
	if( index < current ) { current = index; }

	// Update bottom
	if( index == bottom )
	{
		CoreObjects::DEFAULT_t *objectBottom =
			reinterpret_cast<CoreObjects::DEFAULT_t *>( data + bottom * CoreObjects::TYPE_SIZE[type] );

		while( bottom < CoreObjects::TYPE_BUCKET_CAPACITY[type] && objectBottom->id.alive == false )
		{
			bottom++;
			objectBottom = reinterpret_cast<CoreObjects::DEFAULT_t *>( data +
				bottom * CoreObjects::TYPE_SIZE[type] );
		}
		if( bottom == CoreObjects::TYPE_BUCKET_CAPACITY[type] ) { bottom = 0; }
	}

	// Update top
	if( index == ( top - 1 ) )
	{
		CoreObjects::DEFAULT_t *objectTop =
			reinterpret_cast<CoreObjects::DEFAULT_t *>( data + ( top - 1 ) * CoreObjects::TYPE_SIZE[type] );

		while( top > 0 && objectTop->id.alive == false )
		{
			top--;
			objectTop = reinterpret_cast<CoreObjects::DEFAULT_t *>( data +
				( top - 1 ) * CoreObjects::TYPE_SIZE[type] );
		}
	}

	// Cache bucket
	context.bucketCache[TYPE_BUCKET( context.category, type )] =
		context.bucketCache[TYPE_BUCKET( context.category, type )] > bucketID ?
			bucketID : context.bucketCache[TYPE_BUCKET( context.category, type )];

	// Decerement Object Count
	context.objectCount[TYPE_BUCKET( context.category, type )]--;
	context.objectCount[NULL_TYPE]--; // total object count
	Object::Serialization::dirty |= ( Object::Serialization::context == &context );

	// Success
	return true;
}


byte *ObjectContext::ObjectBucket::get_object_pointer( const u16 index, const u16 generation ) const
{
	// Get Object Pointer
	MemoryAssert( data != nullptr );
	Assert( index < CoreObjects::TYPE_BUCKET_CAPACITY[type] );
	byte *objectPtr = data + index * CoreObjects::TYPE_SIZE[type];
	const CoreObjects::DEFAULT_t *const object =
		reinterpret_cast<CoreObjects::DEFAULT_t *>( objectPtr );

	// Error cases
	if( UNLIKELY( object->id.alive == false ) ) { return nullptr; } // object slot isn't alive
	if( UNLIKELY( object->id.generation != generation ) ) { return nullptr; } // object doesn't match slot generation

	// Return Object Pointer
	return objectPtr;
}


const Object &ObjectContext::ObjectBucket::get_object_id( const u16 index ) const
{
	// Get Object Pointer
	MemoryAssert( data != nullptr );
	Assert( index < CoreObjects::TYPE_BUCKET_CAPACITY[type] );
	byte *objectPtr = data + index * CoreObjects::TYPE_SIZE[type];
	const CoreObjects::DEFAULT_t *const object =
		reinterpret_cast<CoreObjects::DEFAULT_t *>( objectPtr );

	// Return Object ID
	return object->id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ObjectContext::ObjectIteratorAll::find_object_ptr( ObjectIterator &itr,
	const ObjectBucket *const bucket, const u16 start )
{
	// Bucket Verification
	if( bucket->data == nullptr ) { return false; }

	// Loop over the bucket's instances & check if they're alive
	for( u32 i = start; i < bucket->top; i++ )
	{
		byte *const objectPtr = bucket->data + i * CoreObjects::TYPE_SIZE[bucket->type];
		const CoreObjects::DEFAULT_t *const object =
			reinterpret_cast<const CoreObjects::DEFAULT_t *>( objectPtr );

		// Skip dead or deactivated objects
		if( !object->id.alive ) { continue; }

		// Success
		itr.ptr = objectPtr;
		itr.bucketID = bucket->bucketID;
		itr.index = static_cast<u16>( i );
		return true;
	}

	// Failure
	return false;
}


bool ObjectContext::ObjectIteratorActive::find_object_ptr( ObjectIterator &itr,
	const ObjectBucket *const bucket, const u16 start )
{
	// Bucket Verification
	if( bucket->data == nullptr ) { return false; }

	// Loop over the bucket's instances & check if they're alive
	for( u32 i = start; i < bucket->top; i++ )
	{
		byte *const objectPtr = bucket->data + i * CoreObjects::TYPE_SIZE[bucket->type];
		const CoreObjects::DEFAULT_t *const object =
			reinterpret_cast<const CoreObjects::DEFAULT_t *>( objectPtr );

		// Skip dead or deactivated objects
		if( !object->id.alive || object->id.deactivated ) { continue; }

		// Success
		itr.ptr = objectPtr;
		itr.bucketID = bucket->bucketID;
		itr.index = static_cast<u16>( i );
		return true;
	}

	// Failure
	return false;
}


void ObjectContext::ObjectIterator::find_object( u32 startIndex )
{
	// Begin iteration from our bucket id
	if( TYPE_INVALID( context.category, this->type ) ) { this->ptr = nullptr; return; }
	if( UNLIKELY( this->bucketID == NULL_BUCKET ) ) { this->ptr = nullptr; return; }
	ObjectBucket *bucket = &context.buckets[this->bucketID];

	// Loop over ObjectBuckets until we find a live object of our type
	for( ;; )
	{
		// Find object
		if( find_object_ptr( *this, bucket, startIndex ) ) { return; }

		// Move to the next bucket
		if( bucket->bucketIDNext == NULL_BUCKET ) { break; }
		bucket = &context.buckets[bucket->bucketIDNext];
		startIndex = bucket->bottom;

		// Check if the bucket is still our type
		if( bucket->type == this->type ) { continue; }

		// Are we iterating polymorphically (allow children types too)?
		if( !polymorphic ) { break; }

		// If so, check if the bucket is one of our child types
		const u16 targetTypeDepth = CoreObjects::TYPE_INHERITANCE_DEPTH[this->type];
		const u16 bucketTypeDepth = CoreObjects::TYPE_INHERITANCE_DEPTH[bucket->type];
		if( bucketTypeDepth > targetTypeDepth ) { continue; }

		// We must be at the end of valid buckets
		break;
	}

	// If we've made it this far, there are no live objects of our type
	ptr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if COMPILE_DEBUG
#include <core/string.hpp>
#include <manta/draw.hpp>
#include <manta/input.hpp>


const Color colors[] =
{
	c_white,
	c_red,
	c_aqua,
	c_yellow,
	c_green,
	c_orange,
};


void ObjectContext::ObjectBucket::draw( String &label, float x, float y )
{
	const u16 bCapacity = CoreObjects::TYPE_BUCKET_CAPACITY[type];
	const bool isCache = ( context.bucketCache[TYPE_BUCKET( context.category, type )] == bucketID );
	ObjectContext::ObjectBucket &next = context.buckets[bucketIDNext];
	float allocatedMemoryKb = 0.0f;

	// Only draw if non-abstract (capacity > 9) and initialized (memory allocated)
	if( bCapacity > 0 && data != nullptr )
	{
		const u16 bWidth = static_cast<u16>( sqrt( static_cast<double>( bCapacity ) ) );
		const u16 bHeight = static_cast<u16>( ceil( static_cast<double>( bCapacity ) / bWidth ) );
		const float cellSize = 24.0f;

		// Draw Information
		char info[256];
		allocatedMemoryKb = KB( bCapacity * CoreObjects::TYPE_SIZE[type] );

		snprintf( info, sizeof( info ),
			"bucket: %d\n\ntype: %s (id: %d)\n\ninheritance depth: %d\n\ncapacity: %d\n\nmemory: %.2f kb (%d bytes/o)",
			bucketID,
			CoreObjects::TYPE_NAME[type],
			type,
			CoreObjects::TYPE_INHERITANCE_DEPTH[type],
			bCapacity,
			allocatedMemoryKb,
			CoreObjects::TYPE_SIZE[type] );

		draw_text( fnt_iosevka, 14, x, y, c_white, info );
		y += 144.0f;

		// Draw Background
		draw_rectangle( x, y, x + bWidth * cellSize, y + bHeight * cellSize, c_dkgray, false );

		// Draw Object Cells
		for( u16 index = 0; index < bCapacity; index++ )
		{
			byte *objectPtr = data + index * CoreObjects::TYPE_SIZE[type];
			CoreObjects::DEFAULT_t &object = *reinterpret_cast<CoreObjects::DEFAULT_t *>( objectPtr );
			const bool alive = object.id.alive;
			const bool active = object.id.deactivated;

			const u16 ix = index % bWidth;
			const u16 iy = index / bWidth;

			const float cx1 = x + ix * cellSize;
			const float cy1 = y + iy * cellSize;
			const float cx2 = cx1 + cellSize;
			const float cy2 = cy1 + cellSize;

			if( alive )
			{
				draw_rectangle( cx1, cy1, cx2, cy2, color_mix( colors[type], c_black, active ? 0.75f : 0.33f ), false );
				draw_rectangle( cx1, cy1, cx2, cy2, color_mix( colors[type], c_black, active ? 1.00f : 0.50f ), true );

				if( point_in_rect( mouse_x, mouse_y, cx1, cy1, cx2, cy2 ) )
				{
					char buffer[128];
					snprintf( buffer, sizeof( buffer ),
						"type id: %u (%s)\n\nbucket id: %u\n\nindex: %u\n\ngeneration: %u\n\nid: %llu",
						object.id.type,
						CoreObjects::TYPE_NAME[type],
						object.id.bucketID,
						object.id.index,
						object.id.generation,
						object.id.id() );

					label = buffer;
				}
			}
		}

		// Bottom Cell
		if( bottom >= 0 )
		{
			const u16 ix = bottom % bWidth;
			const u16 iy = bottom / bWidth;

			const float cx = x + ix * cellSize + cellSize * 0.5f;
			const float cy = y + iy * cellSize + cellSize * 0.5f;

			draw_rectangle( cx - cellSize * 0.25f, cy - cellSize * 0.25f,
			                cx + cellSize * 0.25f, cy + cellSize * 0.25f, colors[type], false );
		}

		// Top Cell
		if( top - 1 >= 0 )
		{
			const u16 ix = ( top - 1 ) %  bWidth;
			const u16 iy = ( top - 1 ) / bWidth;

			const float cx = x + ix * cellSize + cellSize * 0.5f;
			const float cy = y + iy * cellSize + cellSize * 0.5f;

			draw_rectangle( cx - cellSize * 0.25f, cy - cellSize * 0.25f,
			                cx + cellSize * 0.25f, cy + cellSize * 0.25f, colors[type], false );
		}

		// Current Cell
		if( current != bCapacity )
		{
			const u16 ix = current % bWidth;
			const u16 iy = current / bWidth;

			const float cx = x + ix * cellSize + cellSize * 0.5f;
			const float cy = y + iy * cellSize + cellSize * 0.5f;

			draw_rectangle( cx - cellSize * 0.25f, cy - cellSize * 0.25f, cx + cellSize * 0.25f, cy + cellSize * 0.25f,
						    isCache ? c_white : c_gray, false );
		}

		// Cache Outline
		if( isCache )
		{
			draw_rectangle( x - 8, y - 8, x + bWidth * cellSize + 8, y + bHeight * cellSize + 8,
			                color_mix( colors[type], c_white, 0.5f ), true );
		}

		// Draw Next Arrow
		if( next.data != nullptr && ( next.type == type ||
		    CoreObjects::TYPE_INHERITANCE_DEPTH[next.type] > CoreObjects::TYPE_INHERITANCE_DEPTH[type] ) )
		{
			const float x1 = ( x + bWidth * cellSize + 32 ) - 32.0f * 0.5f;
			const float y1 = ( y + ( bHeight * cellSize ) * 0.5f ) - 16.0f * 0.5f;
			const float x2 = ( x + bWidth * cellSize + 32 ) + 32.0f * 0.5f;
			const float y2 = ( y + ( bHeight * cellSize ) * 0.5f ) + 16.0f * 0.5f;
			const Color color1 = colors[type];
			const Color color2 = next.type == type ? colors[type] : colors[next.type];
			draw_rectangle_gradient( x1, y1, x2, y2, color1, color2, color1, color2 );
		}

		x += bWidth * cellSize + 64;
		y -= 144.0f;
	}

	// Draw next bucket
	if( bucketIDNext != 0 ) { next.draw( label, x, y ); }
}


void ObjectContext::draw( const Delta delta, const float x, const float y )
{
	// Reset Label
	String label = "";

	// Draw Object Buckets
	buckets[1].draw( label, x, y );

	// Draw Mouse Label
	if( label[0] != '\0' )
	{
		const int size = 14;
		const int_v2 bbox = text_dimensions( fnt_iosevka, size, label );
		const float mx = mouse_x + 16.0f;
		const float my = mouse_y + 16.0f;
		draw_rectangle( mx, my, mx + bbox.x + 16, my + bbox.y + 16, c_black, false );
		draw_text( fnt_iosevka, size, mx + 8, my + 8, c_white, label.cstr() );
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////