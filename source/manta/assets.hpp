#pragma once

#include <assets.generated.hpp>

#include <config.hpp>

#include <manta/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysAssets
{
	extern bool init();
	extern bool free();
}

namespace Assets
{
	extern char binaryPath[PATH_SIZE];
	extern File binary;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BinTexture
{
	u32 offset;
	u16 width;
	u16 height;
	u16 levels;
};


struct BinGlyph
{
	u16 u1, v1;
	u16 u2, v2;
};


struct BinSprite
{
	u32 glyph;
	u16 texture;
	u16 count;
	u16 width;
	u16 height;
	i16 xorigin;
	i16 yorigin;
};


struct BinMaterial
{
	u16 textureColor;
	u16 textureNormal;
};


struct BinTTF
{
	usize offset;
	usize size;
};


struct BinFont
{
	u16 ttfs[4];
	DEBUG( const char *name );
};


struct BinSound
{
	bool streamed;
	bool compressed;
	int channels;
	usize sampleOffset;
	usize sampleCount;
	DEBUG( const char *name );
};


struct BinMesh
{
	usize vertexBufferOffset;
	usize vertexBufferSize;
	usize vertexCount;
	usize indexBufferOffset;
	usize indexBufferSize;
	usize indexCount;
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;
};


struct BinSkeleton2D
{
	const char *name;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////