#include <manta/models.hpp>

#include <config.hpp>

#include <core/debug.hpp>

#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Model::init( const u32 meshID, u16 materialID )
{
	Assert( meshID < CoreAssets::meshCount );
	const Assets::MeshEntry &binMesh = Assets::mesh( meshID );

	vertexBuffer.init( binMesh.vertexCount, GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	material = materialID;

	CoreGfx::rb_vertex_buffer_write_begin( vertexBuffer.resource );
	CoreGfx::rb_vertex_buffer_write( vertexBuffer.resource, Assets::binary.data +
		binMesh.vertexBufferOffset, binMesh.vertexBufferSize );
	CoreGfx::rb_vertex_buffer_write_end( vertexBuffer.resource );

	return true;
}


bool Model::free()
{
	//if( !mesh.free() ) { return false; }
	// TODO...
	return true;
}


void Model::draw( double x, double y, double z, double scale, double rotation )
{
	GfxState &state = Gfx::state();
	double_m44 matrixModelCache = Gfx::get_matrix_model();

	double_m44 matrixWorld = double_m44_build_identity();
	double_m44 matrixScale = double_m44_build_scaling( scale, scale, scale );
	matrixWorld = double_m44_multiply( matrixScale, matrixWorld );
	double_m44 matrixRotationY = double_m44_build_rotation_x( -90.0 * DEG2RAD );
	double_m44 matrixRotationZ = double_m44_build_rotation_z( rotation * DEG2RAD );
	matrixWorld = double_m44_multiply( matrixRotationY, matrixWorld );
	matrixWorld = double_m44_multiply( matrixRotationZ, matrixWorld );
	double_m44 matrixTranslation = double_m44_build_translation( x, y, z );
	matrixWorld = double_m44_multiply( matrixTranslation, matrixWorld );

	Gfx::set_matrix_model( matrixWorld );
	{
		CoreGfx::textures[Assets::material( material ).textureColor].bind( 0 );
		CoreGfx::rb_vertex_buffer_draw( vertexBuffer.resource, GfxPrimitiveType_TriangleList );
	}
	Gfx::set_matrix_model( matrixModelCache );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////