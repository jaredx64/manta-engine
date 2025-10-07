#include <atmosphere.hpp>

#include <manta/geometry.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <earth.hpp>
#include <universe.hpp>
#include <view.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MESH_RESOLUTION ( 32 )

namespace Atmosphere
{
	GfxVertexBuffer<GfxVertex::VertexAtmosphere> vertexBuffer;
	GfxIndexBuffer indexBuffer;

	double time = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void generate_mesh_atmosphere( GfxVertexBuffer<GfxVertex::VertexAtmosphere> &vertexBuffer,
	GfxIndexBuffer &indexBuffer )
{
	if( vertexBuffer.resource != nullptr ) { vertexBuffer.free(); }
	List<float_v3> positions; positions.init();
	if( indexBuffer.resource != nullptr ) { indexBuffer.free(); }
	List<u32> indices; indices.init();

	const u32 count = geometry_generate_sphere_latlon( MESH_RESOLUTION, &positions, nullptr, nullptr, &indices );

	vertexBuffer.init( count, GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	vertexBuffer.write_begin();
	for( u32 v = 0; v < count; v++ ) { vertexBuffer.write( GfxVertex::VertexAtmosphere { positions[v] } ); }
	vertexBuffer.write_end();

	const double indexToVertexRatio = static_cast<double>( indices.count() ) / positions.count();
	indexBuffer.init( indices.data, indices.count() * sizeof( u32 ), indexToVertexRatio, GfxIndexBufferFormat_U32 );

	positions.free();
	indices.free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void atmosphere_init()
{
	generate_mesh_atmosphere( Atmosphere::vertexBuffer, Atmosphere::indexBuffer );
}


void atmosphere_free()
{
	if( Atmosphere::vertexBuffer.resource != nullptr ) { Atmosphere::vertexBuffer.free(); }
	if( Atmosphere::indexBuffer.resource != nullptr ) { Atmosphere::indexBuffer.free(); }
}


void atmosphere_update( const Delta delta )
{
	Atmosphere::time = wrap( Atmosphere::time + delta * 0.005, 0.0, 1.0 );
}


void atmosphere_draw( const Delta delta )
{
	Gfx::shader_bind( Shader::sh_atmosphere );
	Gfx::reset_blend_mode();
	Gfx::set_cull_mode( GfxCullMode_BACK );
	Gfx::clear_depth();
	Gfx::set_depth_test_mode( GfxDepthFormat_NONE );
	{
		double_m44 matrixWorld = double_m44_build_identity();
		const double radius = R_EARTH_POLE + R_ATMOSPHERE;
		const double_m44 matrixScale = double_m44_build_scaling( radius, radius, radius );
		matrixWorld = double_m44_multiply( matrixScale, matrixWorld );
		Gfx::set_matrix_mvp( matrixWorld, View::matrixView, View::matrixPerspective );

		GfxUniformBuffer::UniformsAtmosphere.matrixLookInverse = float_m44_inverse( View::matrixLook );
		GfxUniformBuffer::UniformsAtmosphere.eye = View::position;
		GfxUniformBuffer::UniformsAtmosphere.near = static_cast<float>( View::near );
		GfxUniformBuffer::UniformsAtmosphere.forward = View::forward;
		GfxUniformBuffer::UniformsAtmosphere.far = static_cast<float>( View::far );
		GfxUniformBuffer::UniformsAtmosphere.sun = Universe::sun;
		GfxUniformBuffer::UniformsAtmosphere.time = Atmosphere::time;
		GfxUniformBuffer::UniformsAtmosphere.viewport = View::dimensions;
		GfxUniformBuffer::UniformsAtmosphere.upload();

		CoreGfx::textures[Texture::tex_aurora].bind( 0 );

		Gfx::draw_vertex_buffer_indexed( Atmosphere::vertexBuffer, Atmosphere::indexBuffer );

		CoreGfx::textures[Texture::tex_aurora].release();
	}
	Gfx::shader_release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////