#pragma once

#include <shader_api.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Meant to be included in *.shader files. Supplies common built-in vertex format type definitions.

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input VertexPosition
{
	float3 position packed_as( FLOAT32 );
};


vertex_input VertexPositionUV
{
	float3 position packed_as( FLOAT32 );
	float2 uv packed_as( UNORM16 );
};


vertex_input VertexPositionNormal
{
	float3 position packed_as( FLOAT32 );
	float3 normal packed_as( FLOAT32 );
};


vertex_input VertexPositionNormalUV
{
	float3 position packed_as( FLOAT32 );
	float3 normal packed_as( FLOAT32 );
	float2 uv packed_as( UNORM16 );
};


vertex_input VertexPositionNormalTangentUV
{
	float3 position packed_as( FLOAT32 );
	float3 normal packed_as( FLOAT32 );
	float4 tangent packed_as( FLOAT32 );
	float2 uv packed_as( UNORM16 );
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////