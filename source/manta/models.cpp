#include <manta/models.hpp>

#include <config.hpp>

#include <core/debug.hpp>

#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Model::init( const u32 meshID, u16 materialID )
{
	Assert( meshID < Assets::meshCount );
	const BinMesh &binMesh = Assets::meshes[meshID];

	vertexBuffer.init( binMesh.vertexCount, GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	material = materialID;

	GfxCore::rb_vertex_buffered_write_begin( vertexBuffer.resource );
	GfxCore::rb_vertex_buffered_write( vertexBuffer.resource, Assets::binary.data +
		binMesh.vertexBufferOffset, binMesh.vertexBufferSize );
	GfxCore::rb_vertex_buffered_write_end( vertexBuffer.resource );

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
	doublem44 matrixModelCache = Gfx::get_matrix_model();

	doublem44 matrixWorld = doublem44_build_identity();
	doublem44 matrixScale = doublem44_build_scaling( scale, scale, scale );
	matrixWorld = doublem44_multiply( matrixScale, matrixWorld );
	doublem44 matrixRotationY = doublem44_build_rotation_x( -90.0 * DEG2RAD );
	doublem44 matrixRotationZ = doublem44_build_rotation_z( rotation * DEG2RAD );
	matrixWorld = doublem44_multiply( matrixRotationY, matrixWorld );
	matrixWorld = doublem44_multiply( matrixRotationZ, matrixWorld );
	doublem44 matrixTranslation = doublem44_build_translation( x, y, z );
	matrixWorld = doublem44_multiply( matrixTranslation, matrixWorld );

	Gfx::set_matrix_model( matrixWorld );
	{
		GfxCore::textures[Assets::materials[material].textureColor].bind( 0 );
		GfxCore::rb_vertex_buffer_draw( vertexBuffer.resource );
	}
	Gfx::set_matrix_model( matrixModelCache );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////