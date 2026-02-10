#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/cache.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Skeleton2D
{
public:
	Buffer data;

	CacheKey cacheKey;
	String name;
	String path;
};

using SkeletonID = u32;
#define SKELETONID_MAX ( U32_MAX )
#define SKELETONID_NULL ( SKELETONID_MAX )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Skeleton2Ds
{
public:
	SkeletonID register_new( const Skeleton2D &skeleton = Skeleton2D { } );
	SkeletonID register_new( Skeleton2D &&skeleton );
	SkeletonID register_new_from_definition( String name, const char *path );

	usize gather( const char *path, bool recurse = true );
	void build();

public:
	List<Skeleton2D> skeletons;
	Skeleton2D &operator[]( u32 skeletonID ) { return skeletons[skeletonID]; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////