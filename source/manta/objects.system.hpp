#pragma once

#include <vendor/vendor.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using Object_t = u16;
using ObjectCategory_t = u16;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ObjectInstance;
class ObjectContext;
template <Object_t T> struct ObjectHandle;

class Serializer; // <core/serializer.hpp>
class Deserializer; // <core/serializer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Loop over all active instances of a specified object type
#define foreach_object( objectContext, objectType, handle ) \
	for( ObjectHandle<objectType> handle : objectContext.template iterator_active<objectType>( false ) )

// Loop over all active instances of a specified object type and its derived child types
#define foreach_object_polymorphic( objectContext, objectType, handle ) \
	for( ObjectHandle<objectType> handle : objectContext.template iterator_active<objectType>( true ) )

// Loop over all instances of a specified object type
#define foreach_object_all( objectContext, objectType, handle ) \
	for( ObjectHandle<objectType> handle : objectContext.template iterator_all<objectType>( false ) )

// Loop over all instances of a specified object type and its derived child types
#define foreach_object_polymorphic_all( objectContext, objectType, handle ) \
	for( ObjectHandle<objectType> handle : objectContext.template iterator_all<objectType>( true ) )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __INTERNAL_OBJECT_SYSTEM_BEGIN namespace CoreObjects {
#define __INTERNAL_OBJECT_SYSTEM_END /* namespace */ }

#include <objects.system.generated.hpp>

static_assert( CoreObjects::TYPE_COUNT < ( ( 1 << 14 ) - 1 ), "Exceeded max supported object type count!" );
static_assert( CoreObjects::CATEGORY_COUNT < U16_MAX, "Exceeded max supported object category count!" );

// Implementations: see objects.generated.cpp
namespace CoreObjects
{
	extern const u16 CATEGORY_TYPE_BUCKET[CoreObjects::CATEGORY_COUNT][CoreObjects::TYPE_COUNT];
	extern const u16 CATEGORY_TYPES[CoreObjects::CATEGORY_COUNT][CoreObjects::TYPE_COUNT];
	extern const u16 CATEGORY_TYPE_COUNT[CoreObjects::CATEGORY_COUNT];
	DEBUG( extern const char *CATEGORY_NAME[CoreObjects::CATEGORY_COUNT]; )

	extern const u16 TYPE_SIZE[CoreObjects::TYPE_COUNT];
	DEBUG( extern const char *TYPE_NAME[CoreObjects::TYPE_COUNT]; )
	extern const u16 TYPE_BUCKET_CAPACITY[CoreObjects::TYPE_COUNT];
	extern const u32 TYPE_MAX_COUNT[CoreObjects::TYPE_COUNT];
	extern const u16 TYPE_INHERITANCE_DEPTH[CoreObjects::TYPE_COUNT];
	extern const u32 TYPE_HASH[CoreObjects::TYPE_COUNT];
	extern const bool TYPE_SERIALIZED[CoreObjects::TYPE_COUNT];

	template <Object_t T, typename... Args> struct TYPE_CONSTRUCT_VARIADIC;
	// constexpr void ( *TYPE_CONSTRUCT[] )( void * ) = { ... } // IMP: objects.generated.hpp
	// constexpr void ( *TYPE_DESTRUCT[] )( void * ) = { ... }  // IMP: objects.generated.hpp

	template <Object_t T> struct OBJECT_ENCODER;

	extern bool init();
	extern bool free();

	extern void write( Buffer &buffer, const ObjectContext &context );
	extern void read( Buffer &buffer, ObjectContext &context );
	extern void serialize( Serializer &serializer, const ObjectContext &context );
	extern void deserialize( Deserializer &deserializer, ObjectContext &context );
};

extern bool object_is_parent_of( const Object thisType, const Object otherType );
extern bool object_is_child_of( const Object thisType, const Object otherType );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ObjectInstance
{
public:
	ObjectInstance() : alive{ 0 }, deactivated{ 0 }, type{ 0 }, generation{ 0 }, bucketID{ 0 }, index{ 0 } { }
	ObjectInstance( const u16 type, const u16 generation, const u16 bucket, const u16 index ) :
		alive{ 0 }, deactivated{ 0 }, type{ type }, generation{ generation }, bucketID{ bucket }, index{ index } { }

	u16 alive : 1;       // alive flag
	u16 deactivated : 1; // deactivated flag (skips foreach)
	u16 type : 14;       // object type (id)
	u16 generation;      // age in ObjectBucket
	u16 bucketID;        // index into ObjectContext bucket array
	u16 index;           // object's index within ObjectBucket

	template <Object_t T> ObjectHandle<T> handle( const ObjectContext &context ) const; // impl: objects.generated.cpp
	bool operator==( const ObjectInstance &other ) const { return equals( other ); }
	explicit operator bool() const { return alive; }

#if COMPILE_DEBUG
	u64 id() const;
	void id_string( char *buffer, const usize length ) const;
	operator u64() const { return id(); }
#endif

public:
	class Serialization
	{
	private:
		friend ObjectContext;

		struct InstanceTable
		{
			u32 *keys = nullptr;
			u32 capacity = 0;
			u32 current = 0;
			void init( const u32 reserve );
			void free();
			void add( const u32 key );
			u32 get_key( const u32 instance ) const;
			u32 find_instance( const u32 key ) const;
		};

	public:
		static void init();
		static void free();
		static void prepare( const ObjectContext &context );
		static u32 instance_from_object( const ObjectInstance &object );
		static ObjectInstance object_from_instance( const Object type, const u32 instance );

	private:
		static InstanceTable instances[];
		static const ObjectContext *context;

	public:
		static bool dirty;
	};

	static void serialize( Buffer &buffer, const ObjectInstance &object );
	static void deserialize( Buffer &buffer, ObjectInstance &object );

private:
	struct SerializedObject
	{
		SerializedObject() : hash{ 0 }, instance{ 0 } { }
		SerializedObject( const u32 hash, const u32 instance ) : hash{ hash }, instance{ instance } { }
		u32 hash, instance;
	};

	bool equals( const ObjectInstance &other ) const
	{
		return ( other.alive == alive &&
		//       other.deactivated = deactivated (NOTE: purposefully ignored)
				 other.type == type &&
				 other.generation == generation &&
				 other.bucketID == bucketID &&
				 other.index == index );
	}
};
static_assert( sizeof( ObjectInstance ) == 8, "Object size changed!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ObjectContext
{
private:
	friend ObjectInstance;
	friend ObjectInstance::Serialization;

public:
	ObjectContext() : category { 0 } { };
	ObjectContext( const ObjectCategory category ) : category{ category } { };

	bool init();
	bool free();
	bool initialized() const { return buckets != nullptr; }

	byte *get_object_pointer( const ObjectInstance &object ) const;
	bool exists( const ObjectInstance &object ) const { return get_object_pointer( object ) != nullptr; }

	ObjectInstance create( const Object type ); // Default Constructor

	template <Object_t T, typename... Args> ObjectInstance create( Args... args ) // Custom Constructor
	{
		static_assert( T < CoreObjects::TYPE_COUNT, "Invalid object type!" );

		// Find Available Bucket
		ObjectBucket *bucket = new_object( T );
		if( UNLIKELY( bucket == nullptr ) ) { return ObjectInstance { }; }

		// Create Object
		void *const object = bucket->data + ( bucket->current * CoreObjects::TYPE_SIZE[T] );
		CoreObjects::TYPE_CONSTRUCT_VARIADIC<T, Args...>::CONSTRUCT( object, args... );
		return bucket->new_object( object );
	}

	bool destroy( ObjectInstance &instance );
	void destroy_all();
	void destroy_all_type( const Object type );

	bool activate( ObjectInstance &instance, const bool setActive );
	void activate_all( const bool setActive );
	void activate_all_type( const Object type, const bool setActive );

	u32 count( const Object type ) const;
	u32 count_all() const;

	// impl: objects.generated.cpp
	void event_create();
	void event_destroy();
	void event_initialize();
	void event_frame_start( const Delta delta );
	void event_frame_end( const Delta delta );
	void event_update_custom( const Delta delta );
	void event_update_gui( const Delta delta );
	void event_update( const Delta delta );
	void event_render_custom( const Delta delta );
	void event_render_gui( const Delta delta );
	void event_render( const Delta delta );
	void event_custom( const Delta delta );
	void event_prepare();
	bool event_test();
	void event_sleep( const Delta delta );
	void event_wake( const Delta delta );
	void event_flag( const u64 code );
	void event_partition( void *ptr );
	bool event_network_send( Buffer &buffer );
	bool event_network_receive( Buffer &buffer );

	// impl: objects.generated.cpp
	static void write( Buffer &buffer, const ObjectContext &context );
	static void read( Buffer &buffer, ObjectContext &context );
	static void serialize( Buffer &buffer, const ObjectContext &context );
	static void deserialize( Buffer &buffer, ObjectContext &context );

#if COMPILE_DEBUG
	void draw( const Delta delta, const float x, const float y );
#endif

private:
	struct ObjectBucket
	{
		ObjectBucket() = delete;
		ObjectBucket( ObjectContext &context ) : context( context ) { }

		ObjectContext &context; // parent ObjectContext
		byte *data = nullptr;   // data buffer pointer
		u16 type = 0;           // object type
		u16 bucketIDNext = 0;   // index of next ObjectBucket in ObjectContext
		u16 bucketID = 0;       // index of this ObjectBucket in ObjectContext
		u16 current = 0;        // current insertion index
		u16 bottom = 0;         // lowest 'alive' index
		u16 top = 0;            // highest 'alive' index

		void *new_object_pointer();
		ObjectInstance new_object( void *ptr );

		bool delete_object( const u16 index, const u16 generation );

		byte *get_object_pointer( const u16 index, const u16 generation ) const;
		const ObjectInstance &get_object_id( const u16 index ) const;

		bool init( const u16 type );
		void free();
		void clear();

	#if COMPILE_DEBUG
		void draw( class String &label, float x, float y );
	#endif
	};

	struct ObjectIterator
	{
		ObjectIterator( bool ( *find_object_ptr )( ObjectIterator &, const ObjectBucket *const, const u16 ),
			const ObjectContext &context, u16 type, bool polymorphic ) :
			find_object_ptr{ find_object_ptr },
			context{ context }, ptr{ nullptr }, type{ type }, polymorphic{ polymorphic }, index{ 0 },
			bucketID{ CoreObjects::CATEGORY_TYPE_BUCKET[context.category][type] } { find_first(); } // begin() & end()

		void find_object( const u32 startIndex );
		void find_first() { find_object( 0 ); }
		void find_next() { find_object( index + 1 ); }

		bool ( *find_object_ptr )( ObjectIterator &, const ObjectBucket *const, const u16 );
		const ObjectContext &context; // parent ObjectContext
		byte *ptr;                    // pointer to current object instance data
		u16 type : 15;                // object type to iterate over
		u16 polymorphic : 1;          // if true, interate child types too
		u16 index;                    // index within current ObjectBucket
		u16 bucketID;                 // current ObjectBucket index
	};

	struct ObjectIteratorActive : public ObjectIterator
	{
		ObjectIteratorActive( const ObjectContext &context, u16 type, bool poly ) :
			ObjectIterator{ &find_object_ptr, context, type, poly } { }
		static bool find_object_ptr( ObjectIterator &itr, const ObjectBucket *const bucket, const u16 start );
	};

	struct ObjectIteratorAll : public ObjectIterator
	{
		ObjectIteratorAll( const ObjectContext &context, u16 type, bool poly ) :
			ObjectIterator{ &find_object_ptr, context, type, poly } { }
		static bool find_object_ptr( ObjectIterator &itr, const ObjectBucket *const bucket, const u16 start );
	};

	bool grow();

	u16 new_bucket( const u16 type );
	ObjectBucket *new_object( const u16 type );
	ObjectInstance create_object( const u16 type );

public:
	template <Object_t T> struct IteratorAll
	{
		IteratorAll() = delete;
		IteratorAll( const ObjectContext &context, u16 type, bool poly ) : itr{ context, type, poly } { }
		IteratorAll<T> begin() { return { itr.context, itr.type, static_cast<bool>( itr.polymorphic ) }; }
		IteratorAll<T> end() { return { itr.context, 0, static_cast<bool>( itr.polymorphic ) }; }
		bool operator!=( const IteratorAll<T> &other ) const { return itr.ptr != other.itr.ptr; }
		IteratorAll<T> &operator++() { itr.find_next(); return *this; }
		ObjectHandle<T> operator*() const { return ObjectHandle<T>{ itr.ptr }; }
		ObjectIteratorAll itr;
	};

	template <Object_t T> IteratorAll<T> iterator_all( const bool polymorphic = false ) const
	{
		Assert( buckets != nullptr );
		return IteratorAll<T>{ *this, T, polymorphic };
	}

	template <Object_t T> struct IteratorActive
	{
		IteratorActive() = delete;
		IteratorActive( const ObjectContext &context, u16 type, bool poly ) : itr{ context, type, poly } { }
		IteratorActive<T> begin() { return { itr.context, itr.type, static_cast<bool>( itr.polymorphic ) }; }
		IteratorActive<T> end() { return { itr.context, 0, static_cast<bool>( itr.polymorphic ) }; }
		bool operator!=( const IteratorActive<T> &other ) const { return itr.ptr != other.itr.ptr; }
		IteratorActive<T> &operator++() { itr.find_next(); return *this; }
		ObjectHandle<T> operator*() const { return ObjectHandle<T>{ itr.ptr }; }
		ObjectIteratorActive itr;
	};

	template <Object_t T> IteratorActive<T> iterator_active( const bool polymorphic = false ) const
	{
		Assert( buckets != nullptr );
		return IteratorActive<T>{ *this, T, polymorphic };
	}

	template <Object_t T> ObjectHandle<T> handle( const ObjectInstance &object ) const
	{
		return ObjectHandle<T>{ get_object_pointer( object ) };
	}

private:
	ObjectBucket *buckets = nullptr; // ObjectBucket array (dynamic)
	u16 *bucketCache = nullptr;      // Most recent buckets touched by object create/destroy
	u32 *objectCount = nullptr;      // Instance count for each object type
	u16 capacity = 0;                // Number of allocated ObjectBucket slots
	u16 current = 0;                 // Current ObjectBucket insertion index
	u16 disableEvents : 1;
	u16 __unused : 15;
	const ObjectCategory_t category;
};
static_assert( sizeof( ObjectContext ) == 32, "ObjectContext size changed!" );


namespace CoreObjects
{
	template <Object_t T> struct ObjectContextSerializer
	{
		ObjectContextSerializer( const ObjectContext &context ) : context{ context } { }
		const ObjectContext &context;
		static void serialize( Buffer &buffer, const ObjectContextSerializer &s )
		{
			buffer.write<u32>( s.context.count( T ) );
			foreach_object_all( s.context, T, h ) { ObjectHandle<T>::serialize( buffer, h ); }
		}
	};

	template <Object_t T> struct ObjectContextDeserializerA
	{
		ObjectContextDeserializerA( ObjectContext &context ) : context{ context } { }
		ObjectContext &context;
		static void deserialize( Buffer &buffer, ObjectContextDeserializerA &d )
		{
			const u32 count = buffer.read<u32>();
			for( u32 i = 0; i < count; i++ ) { d.context.create( T ); }
		}
	};

	template <Object_t T> struct ObjectContextDeserializerB
	{
		ObjectContextDeserializerB( ObjectContext &context ) : context{ context } { }
		ObjectContext &context;
		static void deserialize( Buffer &buffer, ObjectContextDeserializerB &d )
		{
			const u32 count = buffer.read<u32>();
			foreach_object_all( d.context, T, h ) { ObjectHandle<T>::deserialize( buffer, h ); }
		}
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////