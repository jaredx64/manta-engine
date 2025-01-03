#include <manta/models.hpp>

#include <config.hpp>

#include <core/debug.hpp>

#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Model::init( const u32 meshID, u16 materialID )
{
	Assert( meshID < Assets::meshCount );
	const DiskMesh &diskMesh = Assets::meshes[meshID];

	vertexBuffer.init( diskMesh.vertexCount, GfxCPUAccessMode_WRITE_NO_OVERWRITE );
	material = materialID;

	GfxCore::rb_vertex_buffer_write_begin( vertexBuffer.resource );
	GfxCore::rb_vertex_buffer_write( vertexBuffer.resource, Assets::binary.data + diskMesh.vertexBufferOffset, diskMesh.vertexBufferSize );
	GfxCore::rb_vertex_buffer_write_end( vertexBuffer.resource );

	return true;
}


bool Model::free()
{
	//if( !mesh.free() ) { return false; }
	// TODO...
	return true;
}


void Model::draw( float x, float y, float z, float scale, float rotation )
{
	GfxState &state = Gfx::state();
	Matrix matrixModelCache = Gfx::get_matrix_model();

	Matrix matrixWorld = matrix_build_identity();
	Matrix matrixScale = matrix_build_scaling( scale, scale, scale );
	matrixWorld = matrix_multiply( matrixScale, matrixWorld );
	Matrix matrixRotationY = matrix_build_rotation_x( -90.0f * DEG2RAD );
	Matrix matrixRotationZ = matrix_build_rotation_z( rotation * DEG2RAD );
	matrixWorld = matrix_multiply( matrixRotationY, matrixWorld );
	matrixWorld = matrix_multiply( matrixRotationZ, matrixWorld );
	Matrix matrixTranslation = matrix_build_translation( x, y, z );
	matrixWorld = matrix_multiply( matrixTranslation, matrixWorld );

	Gfx::set_matrix_model( matrixWorld );
	{
		GfxCore::textures[Assets::materials[material].textureColor].bind( 0 );
		GfxCore::rb_vertex_buffer_draw( vertexBuffer.resource );
	}
	Gfx::set_matrix_model( matrixModelCache );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////