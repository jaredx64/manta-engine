#pragma once

#include <core/types.hpp>
#include <core/traits.hpp>
#include <core/list.hpp>
#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Geometry
{
	struct EdgeKey
	{
		u32 v1, v2;

		bool operator==( const EdgeKey &o ) const
		{
			return ( v1 == o.v1 && v2 == o.v2 ) || ( v1 == o.v2 && v2 == o.v1 );
		}
	};
}

namespace Hash
{
	inline u32 hash( const Geometry::EdgeKey key ) { return hash( key.v1 ^ key.v2 ); }
	inline bool equals( const Geometry::EdgeKey a, const Geometry::EdgeKey b ) { return a == b; }
	inline bool is_null( const Geometry::EdgeKey a ) { return a == Geometry::EdgeKey { U32_MAX, U32_MAX }; }
	inline void set_null( Geometry::EdgeKey &a ) { a = Geometry::EdgeKey { U32_MAX, U32_MAX }; }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern u32 geometry_generate_sphere_icosahedron( u32 subdivisions, List<float_v3> *positions = nullptr,
	List<float_v3> *normals = nullptr, List<float_v2> *uvs = nullptr, List<u32> *indices = nullptr );

extern u32 geometry_generate_sphere_latlon( u32 resolution, List<float_v3> *positions = nullptr,
	List<float_v3> *normals = nullptr, List<float_v2> *uvs = nullptr, List<u32> *indices = nullptr );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////