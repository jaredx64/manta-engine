#include <build/assets/skins.hpp>

#include <core/list.hpp>
#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/system.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static usize seek_next_key( const String &string, const char *key, const usize start, const usize end )
{
	usize next = string.find( key, start, end );
	if( next == USIZE_MAX ) { return USIZE_MAX; }
	next += strlen( key );
	return next;
}


static bool extract_line( const String &string, usize offset, String &out )
{
	const usize newline = string.find( "\n", offset );
	if( newline <= offset ) { return false; }
	out = string.substr( offset, newline ).trim();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

u32 Skin::add_material( u32 materialKey, MaterialID materialID )
{
	ErrorIf( materials.count() >= SKIN_SLOT_COUNT_MAX, "Exceeded maximum number of materials on a skin!" );

	materials.add( materialID );
	const u32 slotIndex = static_cast<u32>( materials.count() - 1 );
	materialKeyToSkinSlotIndex.add( materialKey, slotIndex );

	return slotIndex;
}

bool Skin::contains_material( u32 materialKey ) const
{
	return materialKeyToSkinSlotIndex.contains( materialKey );
}


u32 Skin::get_material_slot( u32 materialKey )
{
	// TODO: This could fail so maybe return a null/fallback value?
	return materialKeyToSkinSlotIndex.get( materialKey );
}


bool Skin::load_from_mtl( const char *path )
{
	String fileContents;
	if( !fileContents.load( path ) ) { return false; }

	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	auto extract_texture_path = []( const String &string, const char *key,
		usize start, usize end, String &path, String &name ) -> bool
	{
		const usize pathStart = seek_next_key( string, key, start, end );
		if( pathStart == USIZE_MAX ) { return false; }
		const usize pathEnd = string.find( "\n", pathStart );
		if( pathEnd <= pathStart ) { return false; }

		String pathTexture;
		if( !extract_line( string, pathStart, pathTexture ) ) { return false; }
		path = String( pathDirectory ).append( SLASH ).append( pathTexture );

		static char nameTexture[PATH_SIZE];
		path_get_filename( nameTexture, sizeof( nameTexture ), pathTexture.cstr() );
		path_remove_extension( nameTexture, sizeof( nameTexture ) );
		name = nameTexture;

		return true;
	};

	static String pathTexture;
	static String nameTexture;

	usize start = seek_next_key( fileContents, "newmtl ", 0LLU, USIZE_MAX );
	while( start != USIZE_MAX )
	{
		const usize end = seek_next_key( fileContents, "newmtl ", start, USIZE_MAX );

		String nameMaterial;
		if( extract_line( fileContents, start, nameMaterial ) )
		{
			const u32 materialKey = Hash::hash( nameMaterial.cstr() );
			ErrorIf( this->contains_material( materialKey ), "Skin '%s' has duplicate material '%s'",
				this->name.cstr(), nameMaterial.cstr() );

			const MaterialID materialID = Assets::materials.register_new( Material { } );
			this->add_material( materialKey, materialID );

			Material &material = Assets::materials[materialID];
			// TODO: Material name (make it safe -- chars/underscore only)
			material.name = String( "Skin_" ).append( this->name ).append( "_" ).append( materialKey );

			// Diffuse Texture (map_Kd)
			if( extract_texture_path( fileContents, "map_Kd ", start, end, pathTexture, nameTexture ) )
			{
				material.allocate_texture_from_file( MaterialTextureSlot_Diffuse,
					nameTexture.cstr(), pathTexture.cstr(), true );
			}
		}

		start = end;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SkinID Skins::register_new( const Skin &skin )
{
	AssertMsg( skins.count() < SKINID_MAX, "Exceeded max number of Skins" );
	skins.add( skin );
	return static_cast<SkinID>( skins.count() - 1 );
}


SkinID Skins::register_new( Skin &&skin )
{
	AssertMsg( skins.count() < SKINID_MAX, "Exceeded max number of Skins" );
	skins.add( static_cast<Skin &&>( skin ) );
	return static_cast<SkinID>( skins.count() - 1 );
}


SkinID Skins::register_new_from_file( String &name, const char *path )
{
	AssertMsg( skins.count() < SKINID_MAX, "Exceeded max number of Skins" );

	AssetFile file;
	if( !asset_file_register( file, path ) ) { return SKINID_NULL; }

	SkinID skinID = register_new();
	Skin &skin = skins[skinID];
	skin.name = name;
	skin.path = path;

	char fileExtension[16];
	path_get_extension( fileExtension, sizeof( fileExtension ), path );
	strlower( fileExtension );

	if( streq( fileExtension, ".mtl" ) )
	{
		skin.load_from_mtl( path );
	}
	else
	{
		Error( "Skin '%s' has unknown file type: '%s'", path, fileExtension );
	}

	return skinID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Skins::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( skins.count() );

	Timer timer;

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class\n(\n\tSkin, u32,\n\n" );
		for( Skin &skin : skins ) { header.append( "\t" ).append( skin.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct SkinEntry; }\n\n" );

		// Table
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 skinCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::SkinEntry skins[skinCount];\n" :
			"\textern const Assets::SkinEntry *skins;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace CoreAssets\n{\n" );

		// Table
		if( count > 0 )
		{
			// Assets::ModelEntry Table
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::SkinEntry skins[skinCount] =\n\t{\n" );
			for( Skin &skin : skins )
			{
				source.append( "\t\t// " ).append( skin.name ).append( "\n" );
				source.append( "\t\t{\n" );

				source.append( "\t\t\t{ " );
				for( u32 i = 0; i < SKIN_SLOT_COUNT_MAX; i++ )
				{
					source.append( i < skin.materials.count() ? skin.materials[i] : 0 );
					source.append( i < SKIN_SLOT_COUNT_MAX - 1 ? ", " : " " );
				}
				source.append( "},\n" );

				source.append( "\t\t},\n" );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::SkinEntry *skins = nullptr;\n" );
		}

		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = skins.count();
		Print( PrintColor_White, TAB TAB "Wrote %d skin%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////