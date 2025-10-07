#include <atmosphere.hpp>

#include <manta/geometry.hpp>
#include <manta/input.hpp>
#include <manta/console.hpp>

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
	bool clouds = true;
	int cloudsMip = 0;
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

	Console::command_init( "clouds_mip <level>", "Mip level to sample cloud data (larger = faster)",
		CONSOLE_COMMAND_LAMBDA
		{
			Atmosphere::cloudsMip = Console::get_parameter_int( 0, Atmosphere::cloudsMip );
			Atmosphere::cloudsMip = clamp( Atmosphere::cloudsMip, 0,
				Assets::texture( Texture::tex_earth_light_height_cloud ).levels - 1 );
			Console::Log( c_lime, "cloud_mip %d", Atmosphere::cloudsMip );
		} );
}


void atmosphere_free()
{
	if( Atmosphere::vertexBuffer.resource != nullptr ) { Atmosphere::vertexBuffer.free(); }
	if( Atmosphere::indexBuffer.resource != nullptr ) { Atmosphere::indexBuffer.free(); }
}


void atmosphere_update( const Delta delta )
{
	Atmosphere::time = wrap( Atmosphere::time + delta * 0.005, 0.0, 1.0 );
	if( Keyboard::check_pressed( vk_c ) ) { Atmosphere::clouds = !Atmosphere::clouds; }
}


void atmosphere_draw( const Delta delta )
{
	Gfx::shader_bind( Atmosphere::clouds ? Shader::sh_atmosphere_clouds : Shader::sh_atmosphere );
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
		GfxUniformBuffer::UniformsAtmosphere.cloudsMip = Atmosphere::cloudsMip;
		GfxUniformBuffer::UniformsAtmosphere.upload();

		CoreGfx::textures[Texture::tex_aurora].bind( 0 );
		if( Atmosphere::clouds ) { CoreGfx::textures[Texture::tex_earth_light_height_cloud].bind( 1 ); }

		Gfx::draw_vertex_buffer_indexed( Atmosphere::vertexBuffer, Atmosphere::indexBuffer );

		if( Atmosphere::clouds ) { CoreGfx::textures[Texture::tex_earth_light_height_cloud].release(); }
		CoreGfx::textures[Texture::tex_aurora].release();
	}
	Gfx::shader_release();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////