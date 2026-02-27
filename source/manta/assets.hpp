#pragma once

#include <assets.generated.hpp>

#include <config.hpp>

#include <core/assets.hpp>
#include <core/memory.hpp>

#include <manta/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreAssets
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

class DataAsset; // u32

namespace Assets
{
	struct DataAssetEntry
	{
		usize offset;
		usize size;
	};

	inline u32 data_count() { return CoreAssets::dataAssetCount; }
	extern const Assets::DataAssetEntry &data_asset( DataAsset asset );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Texture; // u32

namespace Assets
{
	struct TextureEntry
	{
		usize offset;
		u16 width;
		u16 height;
		u16 depth;
		u16 layers; // arrays
		u16 levels; // mips
		Assets::TextureColorFormat format;
	};

	inline u32 texture_count() { return CoreAssets::textureCount; }
	extern const Assets::TextureEntry &texture( Texture texture );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Glyph; // u32

namespace Assets
{
	struct GlyphEntry
	{
		u16 u1, v1;
		u16 u2, v2;
	};

	inline u32 glyph_count() { return CoreAssets::glyphCount; }
	extern const Assets::GlyphEntry &glyph( const u32 glyph );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sprite; // u32

namespace Assets
{
	struct SpriteEntry
	{
		u32 glyph;
		u16 texture;
		u16 count;
		u16 width;
		u16 height;
		i16 xorigin;
		i16 yorigin;
	};

	inline u32 sprite_count() { return CoreAssets::spriteCount; }
	extern const Assets::SpriteEntry &sprite( Sprite sprite );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Material; // u32

namespace Assets
{
	struct MaterialEntry
	{
		Assets::MaterialTextureSlot textures[MATERIALTEXTURESLOT_COUNT];
	};

	inline u32 material_count() { return CoreAssets::materialCount; }
	extern const Assets::MaterialEntry &material( Material material );
	extern const Texture material_get_texture( Material material, Assets::MaterialTextureSlot slot );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mesh; // u32

namespace Assets
{
	struct MeshEntry
	{
		Assets::MeshFormatTypeVertex formatVertex;
		usize offsetVertex;
		usize sizeVertex;
		Assets::MeshFormatTypeVertex formatIndex;
		usize offsetIndex;
		usize sizeIndex;
		float x1, y1, z1;
		float x2, y2, z2;
		u32 skinSlotIndex;
	};

	inline u32 mesh_count() { return CoreAssets::meshCount; }
	extern const Assets::MeshEntry &mesh( u32 mesh );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Model; // u32

namespace Assets
{
	struct ModelEntry
	{
		u32 meshCount;
		u32 meshes[32];
		Assets::MeshFormatTypeVertex formatVertex;
		float x1, y1, z1;
		float x2, y2, z2;
		DEBUG( const char *name );
	};

	inline u32 model_count() { return CoreAssets::modelCount; }
	extern const Assets::ModelEntry &model( Model model );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Skin; // u32

namespace Assets
{
	struct SkinEntry
	{
		u32 materials[32];
	};

	inline u32 skin_count() { return CoreAssets::skinCount; }
	extern const Assets::SkinEntry &skin( Skin skin );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TTF; // u32

namespace Assets
{
	struct TTFEntry
	{
		usize offset;
		usize size;
	};

	inline u32 ttf_count() { return CoreAssets::ttfCount; }
	extern const Assets::TTFEntry &ttf( const u32 ttf );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Font; // u32

namespace Assets
{
	struct FontEntry
	{
		u16 ttfs[4];
		DEBUG( const char *name );
	};

	inline u32 font_count() { return CoreAssets::fontCount; }
	extern const Assets::FontEntry &font( const u32 font );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sound; // u32

namespace Assets
{
	struct SoundEntry
	{
		bool streamed;
		bool compressed;
		int channels;
		usize sampleOffset;
		usize sampleCount;
		DEBUG( const char *name );
	};

	inline u32 sound_count() { return CoreAssets::soundCount; }
	extern const Assets::SoundEntry &sound( Sound sound );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Skeleton; // u32

namespace Assets
{
	struct Skeleton2DEntry
	{
		const char *name;
	};

	inline u32 skeleton_2d_count() { return CoreAssets::skeleton2DCount; }
	extern const Assets::Skeleton2DEntry &skeleton_2d( Skeleton skeleton2D );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
class Skeleton3D; // u32

namespace Assets
{
	struct Skeleton3DEntry
	{
		const char *name;
	};

	inline u32 skeleton_3d_count() { return CoreAssets::skeleton3DCount; }
	extern const Assets::Skeleton3DEntry &skeleton_3d( Assets::Skeleton3D skeleton3D );
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////