#include <earth.hpp>

#include <manta/3d.hpp>
#include <manta/window.hpp>
#include <manta/draw.hpp>

#include <view.hpp>
#include <universe.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EARTH_RESOLUTION ( 32 )

namespace Earth
{
	GfxVertexBuffer<GfxVertex::VertexGlobe> vertexBuffer;
	GfxIndexBuffer indexBuffer;
	List<float_v3> collision;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void generate_mesh_collision( List<float_v3> &vertices )
{
	if( vertices.is_initialized() ) { vertices.free(); } vertices.init();

	sphere_generate_geometry_latlon( EARTH_RESOLUTION, &vertices, nullptr, nullptr, nullptr );

	for( float_v3 &vertex : vertices )
	{
		vertex.x *= R_EARTH_EQUATOR;
		vertex.y *= R_EARTH_EQUATOR;
		vertex.z *= R_EARTH_POLE;
	}
}


static void generate_mesh_globe( GfxVertexBuffer<GfxVertex::VertexGlobe> &vertexBuffer,
	GfxIndexBuffer &indexBuffer )
{
	if( vertexBuffer.resource != nullptr ) { vertexBuffer.free(); }
	if( indexBuffer.resource != nullptr ) { indexBuffer.free(); }

	List<float_v3> positions; positions.init();
	List<float_v3> normals; normals.init();
	List<float_v2> uvs; uvs.init();
	List<u32> indices; indices.init();

	const u32 count = sphere_generate_geometry_latlon( EARTH_RESOLUTION, &positions, &normals, &uvs, &indices );

	vertexBuffer.init( count, GfxWriteMode_OVERWRITE );
	vertexBuffer.write_begin();
	for( u32 v = 0; v < count; v++ )
	{
		vertexBuffer.write(
			GfxVertex::VertexGlobe
			{
				.position = positions[v],
				.normal = normals[v],
				.uv = u16_v2 { static_cast<u16>( ( 1.0f - uvs[v].x ) * U16_MAX ),
					static_cast<u16>( uvs[v].y * U16_MAX ) },
			} );
	}
	vertexBuffer.write_end();

	const double indexToVertexRatio = static_cast<double>( indices.count() ) / positions.count();
	indexBuffer.init( indices.data, indices.count() * sizeof( u32 ), indexToVertexRatio, GfxIndexBufferFormat_U32 );

	positions.free();
	normals.free();
	uvs.free();
	indices.free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void earth_init()
{
	generate_mesh_collision( Earth::collision );
	generate_mesh_globe( Earth::vertexBuffer, Earth::indexBuffer );
}


void earth_free()
{
	if( Earth::collision.is_initialized() ) { Earth::collision.free(); }
	if( Earth::vertexBuffer.resource != nullptr ) { Earth::vertexBuffer.free(); }
	if( Earth::indexBuffer.resource != nullptr ) { Earth::indexBuffer.free(); }
}


void earth_update( const Delta delta )
{
	// ...
}


void earth_draw( const Delta delta )
{
	GfxRenderCommand cmd;
	cmd.shader( Shader::sh_globe );
	cmd.raster_cull_mode( GfxCullMode_NONE );
	cmd.depth_function( GfxDepthFunction_LESS );
	cmd.work( GfxWork
		{
			Gfx::sampler_filtering_mode( GfxSamplerFilteringMode_ANISOTROPIC );
			Gfx::sampler_filtering_anisotropy( 8 );

			double_m44 matrixWorld = double_m44_build_identity();
			const double_m44 matrixScale = double_m44_build_scaling( R_EARTH_EQUATOR, R_EARTH_EQUATOR, R_EARTH_POLE );
			matrixWorld = double_m44_multiply( matrixScale, matrixWorld );
			Gfx::set_matrix_mvp( matrixWorld, View::matrixView, View::matrixPerspective );

			GfxUniformBuffer::UniformsGlobe.sun = Universe::sun;
			GfxUniformBuffer::UniformsGlobe.upload();

			Gfx::bind_texture( 0, Texture::tex_earth_color );
			Gfx::bind_texture( 1, Texture::tex_earth_light_height_cloud );
			Gfx::bind_texture( 2, Texture::tex_debug_red );

			Gfx::draw_vertex_buffer_indexed( Earth::vertexBuffer, Earth::indexBuffer );
		} );

	Gfx::render_command_execute( cmd );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double_v3 point_latlon_to_cartesian( double latitude, double longitude, double altitude )
{
	const double a = R_EARTH_EQUATOR;
	const double f = 1.0 / 298.257223563;
	const double e2 = f * ( 2.0 - f );
	double latRadians = latitude * PI / 180.0;
	double lonRadians = longitude * PI / 180.0;

	double sinLat = sin( latRadians );
	double cosLat = cos( latRadians );
	double N = a / sqrt( 1.0 - e2 * sinLat * sinLat );

	// Standard ECEF (X toward 0 E, Y toward 90 E, Z toward North)
	double xSTD = ( N + altitude ) * cosLat * cos( lonRadians );
	double ySTD = ( N + altitude ) * cosLat * sin( lonRadians );
	double zSTD = ( N * ( 1.0 - e2 ) + altitude ) * sinLat;

	// Rotate so that +X points to 180 E instead of 0 E
	double_v3 result;
	result.x = -xSTD;
	result.y = ySTD;
	result.z = zSTD;
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////