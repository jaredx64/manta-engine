#include <universe.hpp>

#include <manta/geometry.hpp>
#include <manta/input.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <view.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Universe
{
	GfxVertexBuffer<GfxVertex::BuiltinVertex> vertexBuffer;
	GfxIndexBuffer indexBuffer;

	float_v3 sun = float_v3_normalize( float_v3 { -1.0f, -0.4f, -0.3f } );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void generate_mesh_skydome( GfxVertexBuffer<GfxVertex::BuiltinVertex> &vertexBuffer,
	GfxIndexBuffer &indexBuffer )
{
	List<float_v3> positions; positions.init();
	List<float_v2> uvs; uvs.init();
	List<u32> indices; indices.init();

	const u32 count = geometry_generate_sphere_latlon( 20, &positions, nullptr, &uvs, &indices );

	if( vertexBuffer.resource != nullptr ) { vertexBuffer.free(); }
	if( indexBuffer.resource != nullptr ) { indexBuffer.free(); }

	vertexBuffer.init( count, GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	vertexBuffer.write_begin();
	for( u32 v = 0; v < count; v++ )
	{
		vertexBuffer.write(
			GfxVertex::BuiltinVertex
			{
				.position = positions[v],
				.uv = u16_v2 { static_cast<u16>( ( 1.0f - uvs[v].x ) * U16_MAX ),
					static_cast<u16>( uvs[v].y * U16_MAX ) },
				.color = u8_v4 { 255, 255, 255, 255 },
			} );
	}
	vertexBuffer.write_end();

	const double indexToVertexRatio = static_cast<double>( indices.count() ) / positions.count();
	indexBuffer.init( indices.data, indices.count() * sizeof( u32 ), indexToVertexRatio, GfxIndexBufferFormat_U32 );

	positions.free();
	uvs.free();
	indices.free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void universe_init()
{
	generate_mesh_skydome( Universe::vertexBuffer, Universe::indexBuffer );
}


void universe_free()
{
	if( Universe::vertexBuffer.resource != nullptr ) { Universe::vertexBuffer.free(); }
	if( Universe::indexBuffer.resource != nullptr ) { Universe::indexBuffer.free(); }
}


void universe_update( const Delta delta )
{
	// Sun
	if( Mouse::check( mb_right ) && View::pickPointValid )
	{
		Universe::sun = double_v3_normalize( View::pickPoint );
	}
}


void universe_draw( const Delta delta )
{
	Gfx::shader_bind( Shader::SHADER_DEFAULT );
	Gfx::set_cull_mode( GfxCullMode_FRONT );
	Gfx::set_filtering_mode( GfxFilteringMode_ANISOTROPIC );
	Gfx::set_filtering_anisotropy( 8 );
	{
		const double_m44 matrixPerspective = double_m44_build_perspective( View::fov, View::aspect, 0.01, 128.0 );
		Gfx::set_matrix_mvp( View::matrixWorld, View::matrixLook, matrixPerspective );

		CoreGfx::textures[Texture::tex_stars_color].bind( 0 );

		Gfx::draw_vertex_buffer_indexed( Universe::vertexBuffer, Universe::indexBuffer );

		CoreGfx::textures[Texture::tex_stars_color].release();
	}
	Gfx::set_cull_mode( GfxCullMode_BACK );
	Gfx::shader_release();

	Gfx::clear_depth();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////