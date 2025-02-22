#pragma once

#include <vendor/vendor.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Object;
class ObjectContext;
template <int N> struct ObjectHandle;

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

#define __INTERNAL_OBJECT_SYSTEM_BEGIN namespace SysObjects {
#define __INTERNAL_OBJECT_SYSTEM_END /* namespace */ }

#include <objects.system.generated.hpp>

static_assert( OBJECT_TYPE_COUNT < ( ( 1 << 14 ) - 1 ), "Exceeded max supported object type count!" );
static_assert( OBJECT_CATEGORY_COUNT < U16_MAX, "Exceeded max supported object category count!" );

// Implementations: see objects.generated.cpp
namespace SysObjects
{
	extern const u16 CATEGORY_TYPE_BUCKET[OBJECT_CATEGORY_COUNT][OBJECT_TYPE_COUNT];
	extern const u16 CATEGORY_TYPES[OBJECT_CATEGORY_COUNT][OBJECT_TYPE_COUNT];
	extern const u16 CATEGORY_TYPE_COUNT[OBJECT_CATEGORY_COUNT];
	DEBUG( extern const char *CATEGORY_NAME[OBJECT_CATEGORY_COUNT]; )

	extern const u16 TYPE_SIZE[OBJECT_TYPE_COUNT];
	DEBUG( extern const char *TYPE_NAME[OBJECT_TYPE_COUNT]; )
	extern const u16 TYPE_BUCKET_CAPACITY[OBJECT_TYPE_COUNT];
	extern const u32 TYPE_MAX_COUNT[OBJECT_TYPE_COUNT];
	extern const u16 TYPE_INHERITANCE_DEPTH[OBJECT_TYPE_COUNT];
	extern const u32 TYPE_HASH[OBJECT_TYPE_COUNT];
	extern const bool TYPE_SERIALIZED[OBJECT_TYPE_COUNT];

	template <int N, typename... Args> struct TYPE_CONSTRUCT_VARIADIC;
	// constexpr void ( *TYPE_CONSTRUCT[] )( void * ) = { ... } // IMP: objects.generated.hpp
	// constexpr void ( *TYPE_DESTRUCT[] )( void * ) = { ... }  // IMP: objects.generated.hpp

	template <int N> struct OBJECT_ENCODER;

	extern bool init();
	extern bool free();

	extern void write( Buffer &buffer, const ObjectContext &context );
	extern void read( Buffer &buffer, ObjectContext &context );
	extern void serialize( Serializer &serializer, const ObjectContext &context );
	extern void deserialize( Deserializer &deserializer, ObjectContext &context );
};

extern bool object_is_parent_of( const u16 thisType, const u16 otherType );
extern bool object_is_child_of( const u16 thisType, const u16 otherType );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Object
{
public:
	Object() : alive{ 0 }, deactivated{ 0 }, type{ 0 }, generation{ 0 }, bucketID{ 0 }, index{ 0 } { }
	Object( const u16 type, const u16 generation, const u16 bucket, const u16 index ) :
		alive{ 0 }, deactivated{ 0 }, type{ type }, generation{ generation }, bucketID{ bucket }, index{ index } { }

	u16 alive : 1;       // alive flag
	u16 deactivated : 1; // deactivated flag (skips foreach)
	u16 type : 14;       // object type (id)
	u16 generation;      // age in ObjectBucket
	u16 bucketID;        // index into ObjectContext bucket array
	u16 index;           // object's index within ObjectBucket

	template <int N> ObjectHandle<N> handle( const ObjectContext &context ) const; // impl: objects.generated.cpp
	bool operator==( const Object &other ) const { return equals( other ); }
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
		static u32 instance_from_object( const Object &object );
		static Object object_from_instance( const u16 type, const u32 instance );

	private:
		static InstanceTable instances[];
		static const ObjectContext *context;

	public:
		static bool dirty;
	};

	static void serialize( Buffer &buffer, const Object &object );
	static void deserialize( Buffer &buffer, Object &object );

private:
	struct SerializedObject
	{
		SerializedObject() : hash{ 0 }, instance{ 0 } { }
		SerializedObject( const u32 hash, const u32 instance ) : hash{ hash }, instance{ instance } { }
		u32 hash, instance;
	};

	bool equals( const Object &other ) const
	{
		return ( other.alive == alive &&
		//       other.deactivated = deactivated (NOTE: purposefully ignored)
				 other.type == type &&
				 other.generation == generation &&
				 other.bucketID == bucketID &&
				 other.index == index );
	}
};
static_assert( sizeof( Object ) == 8, "Object size changed!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ObjectContext
{
private:
	friend Object;
	friend Object::Serialization;

public:
	ObjectContext() : category { 0 } { };
	ObjectContext( const u16 category ) : category{ category } { };

	bool init();
	bool free();
	bool initialized() const { return buckets != nullptr; }

	byte *get_object_pointer( const Object &object ) const;
	bool exists( const Object &object ) const { return get_object_pointer( object ) != nullptr; }

	Object create( const u16 type ); // Default Constructor

	template <int N, typename... Args> Object create( Args... args ) // Custom Constructor
	{
		static_assert( N < OBJECT_TYPE_COUNT, "Invalid object type!" );

		// Find Available Bucket
		ObjectBucket *bucket = new_object( N );
		if( UNLIKELY( bucket == nullptr ) ) { return Object { }; }

		// Create Object
		void *const object = bucket->data + ( bucket->current * SysObjects::TYPE_SIZE[N] );
		SysObjects::TYPE_CONSTRUCT_VARIADIC<N, Args...>::CONSTRUCT( object, args... );
		return bucket->new_object( object );
	}

	bool destroy( Object &object );
	void destroy_all();
	void destroy_all_type( const u16 type );

	bool activate( Object &object, const bool setActive );
	void activate_all( const bool setActive );
	void activate_all_type( const u16 type, const bool setActive );

	u32 count( const u16 type ) const;
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
		u16 bucketIDNext = 0;   // index of next ObjectBucket in ObjectContext
		u16 bucketID = 0;       // index of this ObjectBucket in ObjectContext
		u16 current = 0;        // current insertion index
		u16 type = 0;           // object type
		u16 bottom = 0;         // lowest 'alive' index
		u16 top = 0;            // highest 'alive' index

		void *new_object_pointer();
		Object new_object( void *ptr );

		bool delete_object( const u16 index, const u16 generation );

		byte *get_object_pointer( const u16 index, const u16 generation ) const;
		const Object &get_object_id( const u16 index ) const;

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
			bucketID{ SysObjects::CATEGORY_TYPE_BUCKET[context.category][type] } { find_first(); } // begin() & end()

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
	Object create_object( const u16 type );

public:
	template <int N> struct IteratorAll
	{
		IteratorAll() = delete;
		IteratorAll( const ObjectContext &context, u16 type, bool poly ) : itr{ context, type, poly } { }
		IteratorAll<N> begin() { return { itr.context, itr.type, static_cast<bool>( itr.polymorphic ) }; }
		IteratorAll<N> end() { return { itr.context, 0, static_cast<bool>( itr.polymorphic ) }; }
		bool operator!=( const IteratorAll<N> &other ) const { return itr.ptr != other.itr.ptr; }
		IteratorAll<N> &operator++() { itr.find_next(); return *this; }
		ObjectHandle<N> operator*() const { return ObjectHandle<N>{ itr.ptr }; }
		ObjectIteratorAll itr;
	};

	template <int N> IteratorAll<N> iterator_all( const bool polymorphic = false ) const
	{
		Assert( buckets != nullptr );
		return IteratorAll<N>{ *this, N, polymorphic };
	}

	template <int N> struct IteratorActive
	{
		IteratorActive() = delete;
		IteratorActive( const ObjectContext &context, u16 type, bool poly ) : itr{ context, type, poly } { }
		IteratorActive<N> begin() { return { itr.context, itr.type, static_cast<bool>( itr.polymorphic ) }; }
		IteratorActive<N> end() { return { itr.context, 0, static_cast<bool>( itr.polymorphic ) }; }
		bool operator!=( const IteratorActive<N> &other ) const { return itr.ptr != other.itr.ptr; }
		IteratorActive<N> &operator++() { itr.find_next(); return *this; }
		ObjectHandle<N> operator*() const { return ObjectHandle<N>{ itr.ptr }; }
		ObjectIteratorActive itr;
	};

	template <int N> IteratorActive<N> iterator_active( const bool polymorphic = false ) const
	{
		Assert( buckets != nullptr );
		return IteratorActive<N>{ *this, N, polymorphic };
	}

	template <int N> ObjectHandle<N> handle( const Object &object ) const
	{
		return ObjectHandle<N>{ get_object_pointer( object ) };
	}

private:
	ObjectBucket *buckets = nullptr; // ObjectBucket array (dynamic)
	u16 *bucketCache = nullptr;      // Most recent buckets touched by object create/destroy
	u32 *objectCount = nullptr;      // Instance count for each object type
	u16 capacity = 0;                // Number of allocated ObjectBucket slots
	u16 current = 0;                 // Current ObjectBucket insertion index
	u16 disableEvents : 1;
	u16 __unused : 15;
	const u16 category;
};
static_assert( sizeof( ObjectContext ) == 32, "ObjectContext size changed!" );


namespace SysObjects
{
	template <int N> struct ObjectContextSerializer
	{
		ObjectContextSerializer( const ObjectContext &context ) : context{ context } { }
		const ObjectContext &context;
		static void serialize( Buffer &buffer, const ObjectContextSerializer &s )
		{
			buffer.write<u32>( s.context.count( N ) );
			foreach_object_all( s.context, N, h ) { ObjectHandle<N>::serialize( buffer, h ); }
		}
	};

	template <int N> struct ObjectContextDeserializerA
	{
		ObjectContextDeserializerA( ObjectContext &context ) : context{ context } { }
		ObjectContext &context;
		static void deserialize( Buffer &buffer, ObjectContextDeserializerA &d )
		{
			const u32 count = buffer.read<u32>();
			for( u32 i = 0; i < count; i++ ) { d.context.create( N ); }
		}
	};

	template <int N> struct ObjectContextDeserializerB
	{
		ObjectContextDeserializerB( ObjectContext &context ) : context{ context } { }
		ObjectContext &context;
		static void deserialize( Buffer &buffer, ObjectContextDeserializerB &d )
		{
			const u32 count = buffer.read<u32>();
			foreach_object_all( d.context, N, h ) { ObjectHandle<N>::deserialize( buffer, h ); }
		}
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////