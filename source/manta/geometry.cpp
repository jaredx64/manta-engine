#include <manta/geometry.hpp>

#include <core/types.hpp>
#include <core/traits.hpp>
#include <core/math.hpp>
#include <core/hashmap.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 geometry_generate_sphere_icosahedron( u32 subdivisions, List<float_v3> *positions,
	List<float_v3> *normals, List<float_v2> *uvs, List<u32> *indices )
{
	const bool generatePositions = positions != nullptr;
	Assert( !generatePositions || positions->initialized() );
	const bool generateNormals = normals != nullptr;
	Assert( !generateNormals || normals->initialized() );
	const bool generateUVs = uvs != nullptr;
	Assert( !generateUVs || uvs->initialized() );
	const bool generateIndices = indices != nullptr;
	Assert( !generateIndices || indices->initialized() );

	if( UNLIKELY( !generatePositions && !generateNormals && !generateUVs && !generateIndices ) ) { return 0; }

	subdivisions = clamp( subdivisions, 0U, 8U );

	const float X = 0.525731112119133606f; // Golden ratio
	const float Z = 0.850650808352039932f; // Golden ratio

	const float basePositions[12][3] =
	{
		{ -X, 0, Z }, {  X, 0,  Z }, { -X,  0, -Z }, {  X,  0, -Z },
		{  0, Z, X }, {  0, Z, -X }, {  0, -Z,  X }, {  0, -Z, -X },
		{  Z, X, 0 }, { -Z, X,  0 }, {  Z, -X,  0 }, { -Z, -X,  0 }
	};

	const int baseIndices[20][3] =
	{
		{ 0x0, 0x4, 0x1 }, { 0x0, 0x9, 0x4 }, { 0x9, 0x5, 0x4 }, { 0x4, 0x5, 0x8 }, { 0x4, 0x8, 0x1 },
		{ 0x8, 0xA, 0x1 }, { 0x8, 0x3, 0xA }, { 0x5, 0x3, 0x8 }, { 0x5, 0x2, 0x3 }, { 0x2, 0x7, 0x3 },
		{ 0x7, 0xA, 0x3 }, { 0x7, 0x6, 0xA }, { 0x7, 0xB, 0x6 }, { 0xB, 0x0, 0x6 }, { 0x0, 0x1, 0x6 },
		{ 0x6, 0x1, 0xA }, { 0x9, 0x0, 0xB }, { 0x9, 0xB, 0x2 }, { 0x9, 0x2, 0x5 }, { 0x7, 0x2, 0xB }
	};

	// Helper: Compute UV
	auto compute_uv = [&]( const float_v3 &position ) -> float_v2
	{
		float_v2 uv;
		uv.x = ( atan2f( position.y, position.x ) + PI_F)  / ( 2.0f * PI_F );
		uv.y = acosf( position.z ) / PI_F;
		return uv;
	};

	// Helper: Approximate hashmap size
	auto hashmap_reserve = [&]( const u32 subdivisions ) -> u32
	{
		u32 reserve = 30;
		for( u32 i = 0; i < subdivisions; i++ ) { reserve = reserve * 2 + 20 * 3 * ( 1 << ( 2 * i ) ); }
		return reserve;
	};

	// Temporary data structures
	List<float_v3> _xyz;
	List<float_v3> *xyz = nullptr;
	if( generatePositions ) { xyz = positions; } else { _xyz.init(); xyz = &_xyz; }
	HashMap<Geometry::EdgeKey, u32> midpoints;
	if( generateIndices ) { midpoints.init( hashmap_reserve( subdivisions ) ); }

	// Initial Icosahedron
	if( generateIndices )
	{
		// Vertices (Indexed)
		for( int i = 0; i < 12; i++ )
		{
			const float_v3 v0 = float_v3 { basePositions[i][0], basePositions[i][1],
				basePositions[i][2] }.normalize();

			xyz->add( v0 );
			if( generateNormals ) { normals->add( v0 ); }
			if( generateUVs ) { uvs->add( compute_uv( v0 ) ); }
		}

		// Indices
		for( int i = 0; i < 20; i++ )
		{
			indices->add( baseIndices[i][0] );
			indices->add( baseIndices[i][1] );
			indices->add( baseIndices[i][2] );
		}
	}
	else
	{
		// Vertices (Non-Indexed)
		for( int i = 0; i < 20; i++ )
		{
			const int *triangle = baseIndices[i];

			const float_v3 v0 = float_v3 { basePositions[triangle[0]][0], basePositions[triangle[0]][1],
				basePositions[triangle[0]][2] }.normalize();
			const float_v3 v1 = float_v3 { basePositions[triangle[1]][0], basePositions[triangle[1]][1],
				basePositions[triangle[1]][2] }.normalize();
			const float_v3 v2 = float_v3 { basePositions[triangle[2]][0], basePositions[triangle[2]][1],
				basePositions[triangle[2]][2] }.normalize();

			xyz->add( v0 );
			xyz->add( v1 );
			xyz->add( v2 );

			if( generateNormals )
			{
				normals->add( v0 );
				normals->add( v1 );
				normals->add( v2 );
			}

			if( generateUVs )
			{
				uvs->add( compute_uv( v0 ) );
				uvs->add( compute_uv( v1 ) );
				uvs->add( compute_uv( v2 ) );
			}
		}
	}

	// Subdivide Icosahedron
	for( u32 s = 0; s < subdivisions; s++ )
	{
		if( generateIndices )
		{
			List<u32> indicesSubdivided;
			indicesSubdivided.init( indices->count() * 4 );

			auto split_edge = [&]( const u32 i1, const u32 i2 ) -> u32
			{
				Geometry::EdgeKey key { i1, i2 };
				if( midpoints.contains( key ) ) { return midpoints.get( key ); }

				const u32 index = static_cast<u32>( xyz->count() );
				midpoints.set( key, index );

				const float_v3 position = float_v3 { ( xyz->at( i1 ) + xyz->at( i2 ) ) * 0.5f }.normalize();
				xyz->add( position );
				if( generateNormals ) { normals->add( position ); }
				if( generateUVs ) { uvs->add( compute_uv( position ) ); }

				return index;
			};

			for( usize i = 0; i + 2 < indices->count(); i += 3 )
			{
				const u32 i1 = indices->at( i + 0 );
				const u32 i2 = indices->at( i + 1 );
				const u32 i3 = indices->at( i + 2 );
				const u32 a = split_edge( i1, i2 );
				const u32 b = split_edge( i2, i3 );
				const u32 c = split_edge( i3, i1 );
				indicesSubdivided.add( i1 );
				indicesSubdivided.add( a );
				indicesSubdivided.add( c );
				indicesSubdivided.add( i2 );
				indicesSubdivided.add( b );
				indicesSubdivided.add( a );
				indicesSubdivided.add( i3 );
				indicesSubdivided.add( c );
				indicesSubdivided.add( b );
				indicesSubdivided.add( a );
				indicesSubdivided.add( b );
				indicesSubdivided.add( c );
			}

			indices->move( static_cast<List<u32> &&>( indicesSubdivided ) );
		}
		else
		{
			List<float_v3> xyzSubdivided;
			List<float_v3> normalsSubdivided;
			List<float_v2> uvsSubdivided;
			xyzSubdivided.init( xyz->count() * 4 );
			if( generateNormals ) { normalsSubdivided.init( normals->count() * 4 ); }
			if( generateUVs ) { uvsSubdivided.init( uvs->count() * 4 ); }

			auto add_triangle = [&]( const float_v3 &p0, const float_v3 &p1, const float_v3 &p2 )
			{
				xyzSubdivided.add( p0 );
				xyzSubdivided.add( p1 );
				xyzSubdivided.add( p2 );

				if( generateNormals )
				{
					normalsSubdivided.add( p0 );
					normalsSubdivided.add( p1 );
					normalsSubdivided.add( p2 );
				}

				if( generateUVs )
				{
					uvsSubdivided.add( compute_uv( p0 ) );
					uvsSubdivided.add( compute_uv( p1 ) );
					uvsSubdivided.add( compute_uv( p2 ) );
				}
			};

			for( usize i = 0; i + 2 < xyz->count(); i += 3 )
			{
				const float_v3 &v0 = xyz->at( i + 0 );
				const float_v3 &v1 = xyz->at( i + 1 );
				const float_v3 &v2 = xyz->at( i + 2 );

				const u32 a = static_cast<u32>( xyzSubdivided.count() );
				const u32 b = a + 1;
				const u32 c = a + 2;

				const float_v3 m0 = float_v3 { ( v0 + v1 ) * 0.5f }.normalize();
				const float_v3 m1 = float_v3 { ( v1 + v2 ) * 0.5f }.normalize();
				const float_v3 m2 = float_v3 { ( v2 + v0 ) * 0.5f }.normalize();

				add_triangle( v0, m0, m2 );
				add_triangle( v1, m1, m0 );
				add_triangle( v2, m2, m1 );
				add_triangle( m0, m1, m2 );
			}

			xyz->move( static_cast<List<float_v3> &&>( xyzSubdivided ) );
			if( generateNormals ) { normals->move( static_cast<List<float_v3> &&>( normalsSubdivided ) ); }
			if( generateUVs ) { uvs->move( static_cast<List<float_v2> &&>( uvsSubdivided ) ); }
		}
	}

	// Free temporary data structures
	const u32 vertexCount = xyz->count();
	if( !generatePositions ) { _xyz.free(); }
	if( generateIndices ) { midpoints.free(); }

	return vertexCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 geometry_generate_sphere_latlon( u32 resolution, List<float_v3> *positions,
	List<float_v3> *normals, List<float_v2> *uvs, List<u32> *indices )
{
	const bool generatePositions = positions != nullptr;
	Assert( !generatePositions || positions->initialized() );
	const bool generateNormals = normals != nullptr;
	Assert( !generateNormals || normals->initialized() );
	const bool generateUVs = uvs != nullptr;
	Assert( !generateUVs || uvs->initialized() );
	const bool generateIndices = indices != nullptr;
	Assert( !generateIndices || indices->initialized() );

	if( UNLIKELY( !generatePositions && !generateNormals && !generateUVs && !generateIndices ) ) { return 0; }

	resolution = max( resolution, 3u );
	const u32 lonCount = resolution * 2;
	const u32 latCount = resolution;
	const float lonCountInv = 1.0f / lonCount;
	const float latCountInv = 1.0f / latCount;

	// Temporary data structures
	List<float_v3> _xyz;
	List<float_v3> *xyz = nullptr;
	if( generatePositions ) { xyz = positions; } else { _xyz.init(); xyz = &_xyz; }

	if( generateIndices )
	{
		const u32 vertexCount = ( latCount + 1 ) * ( lonCount + 1 );
		xyz->reserve( vertexCount );
		if( generateNormals ) { normals->reserve( vertexCount ); }
		if( generateUVs ) { uvs->reserve( vertexCount ); }

		// Generate vertices
		for( u32 lat = 0; lat <= latCount; ++lat )
		{
			// Latitude angle from 0 to PI
			const float latAngle = lat * latCountInv * PI_F;
			const float z = cosf( latAngle ); // Z ranges from 1 (north) to -1 (south)
			const float radius = sinf( latAngle );

			for( u32 lon = 0; lon <= lonCount; ++lon )
			{
				// Longitude angle from 0 to 2 * PI
				const float lonAngle = lon * lonCountInv * 2.0f * PI_F;
				const float x = radius * cosf( lonAngle );
				const float y = radius * sinf( lonAngle );

				const float_v3 position = float_v3 { x, y, z };
				xyz->add( position );

				if( generateNormals )
				{
					normals->add( float_v3_normalize( position ) );
				}

				if( generateUVs )
				{
					uvs->add( float_v2 { lon * lonCountInv, lat * latCountInv } );
				}
			}
		}

		const u32 indexCount = latCount * lonCount * 6;
		indices->reserve( indexCount );

		// Generate indices
		for( u32 lat = 0; lat < latCount; ++lat )
		{
			for( u32 lon = 0; lon < lonCount; ++lon )
			{
				const u32 first = lat * ( lonCount + 1 ) + lon;
				const u32 second = ( lat + 1 ) * ( lonCount + 1 ) + lon;

				// Two triangles per quad
				indices->add( first );
				indices->add( second );
				indices->add( first + 1 );

				indices->add( first + 1 );
				indices->add( second );
				indices->add( second + 1 );
			}
		}
	}
	else
	{
		const u32 vertexCount = latCount * lonCount * 6;
		xyz->reserve( vertexCount );
		if( generateNormals ) { normals->reserve( vertexCount ); }
		if( generateUVs ) { uvs->reserve( vertexCount ); }

		// Generate vertices directly as triangles
		for( u32 lat = 0; lat < latCount; ++lat )
		{
			// Latitude angles
			const float latAngle0 = lat * latCountInv * PI_F;
			const float lonAngle1 = ( lat + 1 ) * latCountInv * PI_F;

			const float z0 = cosf( latAngle0 );
			const float z1 = cosf( lonAngle1 );

			const float radius0 = sinf( latAngle0 );
			const float radius1 = sinf( lonAngle1 );

			for( u32 lon = 0; lon < lonCount; ++lon )
			{
				// Longitude angles
				const u32 nextLon = ( lon + 1 ) % lonCount;
				const float lonAngle0 = lon * lonCountInv * 2.0f * PI_F;
				const float lonAngle1 = nextLon * lonCountInv * 2.0f * PI_F;

				const float_v3 v0 = float_v3 { radius0 * cosf( lonAngle0 ), radius0 * sinf( lonAngle0 ), z0 };
				const float_v3 v1 = float_v3 { radius1 * cosf( lonAngle0 ), radius1 * sinf( lonAngle0 ), z1 };
				const float_v3 v2 = float_v3 { radius0 * cosf( lonAngle1 ), radius0 * sinf( lonAngle1 ), z0 };
				const float_v3 v3 = float_v3 { radius1 * cosf( lonAngle1 ), radius1 * sinf( lonAngle1 ), z1 };

				float_v2 uv0 = float_v2 { lon * lonCountInv, lat * latCountInv };
				float_v2 uv1 = float_v2 { lon * lonCountInv, ( lat + 1 ) * latCountInv };
				float_v2 uv2 = float_v2 { nextLon * lonCountInv, lat * latCountInv };
				float_v2 uv3 = float_v2 { nextLon * lonCountInv, ( lat + 1 ) * latCountInv };

				// UV.x seam: when nextLon is 0, uv.x must be 1.0 (voids backwards interpolation)
				if( nextLon == 0 )
				{
					uv2.x = 1.0f;
					uv3.x = 1.0f;
				}

				// First triangle
				xyz->add( v0 );
				xyz->add( v1 );
				xyz->add( v2 );

				if( generateNormals )
				{
					normals->add( float_v3_normalize( v0 ) );
					normals->add( float_v3_normalize( v1 ) );
					normals->add( float_v3_normalize( v2 ) );
				}

				if( generateUVs )
				{
					uvs->add( uv0 );
					uvs->add( uv1 );
					uvs->add( uv2 );
				}

				// Second triangle
				xyz->add( v2 );
				xyz->add( v1 );
				xyz->add( v3 );

				if( generateNormals )
				{
					normals->add( float_v3_normalize( v2 ) );
					normals->add( float_v3_normalize( v1 ) );
					normals->add( float_v3_normalize( v3 ) );
				}

				if( generateUVs )
				{
					uvs->add( uv2 );
					uvs->add( uv1 );
					uvs->add( uv3 );
				}
			}
		}
	}

	// Free temporary data structures
	const u32 vertexCount = xyz->count();
	if( !generatePositions ) { _xyz.free(); }

	return vertexCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////