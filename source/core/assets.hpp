#pragma once

#include <core/types.hpp>

// NOTE: This header defines asset macros/contants shared between build and runtime codebases.

namespace Assets
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( TextureColorFormat, u8 )
{
	TextureColorFormat_R8G8B8A8 = 0,
	TextureColorFormat_R8G8,
	TextureColorFormat_R8,
	TextureColorFormat_R16G16B16A16,
	TextureColorFormat_R16G16,
	TextureColorFormat_R16,
	TextureColorFormat_R32G32B32A32,
	TextureColorFormat_R32G32,
	TextureColorFormat_R32,
	TextureColorFormat_R10G10B10A2,
	// ...
	TEXTURECOLORFORMAT_COUNT,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( MaterialTextureSlot, int )
{
	MaterialTextureSlot_Color = 0,
	MaterialTextureSlot_Normal,
	MaterialTextureSlot_Roughness,
	MaterialTextureSlot_Metallic,
	MaterialTextureSlot_Emissive,
	MaterialTextureSlot_Specular,
	MaterialTextureSlot_AmbientOcclusion,
	// ...
	MATERIALTEXTURESLOT_COUNT
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( MeshFormatTypeVertex, u32 )
{
	MeshFormatTypeVertex_Position = 0,
	MeshFormatTypeVertex_PositionUV,
	MeshFormatTypeVertex_PositionNormal,
	MeshFormatTypeVertex_PositionNormalUV,
	MeshFormatTypeVertex_PositionNormalTangentUV,
	// ...

	MESHFORMATTYPEVERTEX_COUNT,
	MeshFormatTypeVertex_Default = MeshFormatTypeVertex_PositionNormalTangentUV,
};


// NOTE: These map 'MeshFormatTypeVertex' to manta/shaders/vertex_formats definitions, to be used in *.model files
constexpr const char *MeshFormatTypeVertexNames[MESHFORMATTYPEVERTEX_COUNT] =
{
	"VertexPosition",
	"VertexPositionUV",
	"VertexPositionNormal",
	"VertexPositionNormalUV",
	"VertexPositionNormalTangentUV",
};


enum_type( MeshFormatTypeIndex, u32 )
{
	MeshFormatTypeIndex_U32 = 0,
	MeshFormatTypeIndex_U16,
	// ...
	MESHFORMATTYPEINDEX_COUNT
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum_type( ShaderStageFlag, u8 )
{
	ShaderStageFlag_Vertex = ( 1 << 0 ),
	ShaderStageFlag_Fragment = ( 1 << 1 ),
	ShaderStageFlag_Compute = ( 1 << 2 ),
	// ...
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}