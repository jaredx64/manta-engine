#include <build/assets/sprites.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/json.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sprites::make_new( const Sprite &sprite )
{
	sprites.add( sprite );
}


void Sprites::gather( const char *path, const bool recurse )
{
	// Gather & Load Sprites
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".sprite", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u sprite%s found in: %s", count, count == 1 ? "" : "s", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


void Sprites::load( const char *path )
{
	// Open sprite file
	String spriteFile;
	ErrorIf( !spriteFile.load( path ), "Unable to load sprite file: %s", path );
	JSON spriteJSON { spriteFile };

	// Build Cache
	Assets::assetFileCount++;
	if( !Build::cacheDirtyAssets )
	{
		FileTime time;
		file_time( path, &time );
		Build::cacheDirtyAssets |= file_time_newer( time, Assets::timeCache );
	}

	// Read file (json)
	String name = spriteJSON.get_string( "name" );
	ErrorIf( name.length_bytes() == 0, "Sprite '%s' has an invalid name (required)", path );
	String texture = spriteJSON.get_string( "texture" );
	ErrorIf( texture.length_bytes() == 0, "Sprite '%s' has an invalid texture (required)", path );
	String atlas = spriteJSON.get_string( "atlas" );
	ErrorIf( atlas.length_bytes() == 0, "Sprite '%s' has an invalid atlas texture (required)", path );
	int count = spriteJSON.get_int( "count", 1 );
	ErrorIf( count < 1, "Sprite '%s' has an invalid count", path );
	int xorigin = spriteJSON.get_int( "xorigin", 0 );
	int yorigin = spriteJSON.get_int( "yorigin", 0 );

	// Load texture (try relative path first)
	char pathRelative[PATH_SIZE];
	path_get_directory( pathRelative, sizeof( pathRelative ), path );
	strappend( pathRelative, SLASH );
	strappend( pathRelative, texture.cstr() );
	Texture2DBuffer spriteTexture { pathRelative };
	if( !spriteTexture )
	{
		// Relative path failed -- try absolute path
		spriteTexture.load( texture.cstr() );
		ErrorIf( !spriteTexture, "Unable to load texture for sprite %s (texture: %s)", path, texture.cstr() );
	}

	// Register Sprite
	Sprite sprite;
	sprite.name = name;
	sprite.count = count;
	sprite.width = spriteTexture.width / count;
	sprite.height = spriteTexture.height;
	sprite.xorigin = xorigin;
	sprite.yorigin = yorigin;

	// Pack as atlas
	sprite.textureID = Assets::textures.make_new( atlas );
	sprite.glyphID = GLYPHID_MAX;

	// Split sprite into individual glyphs
	for( u16 i = 0; i < sprite.count; i++ )
	{
		Texture2DBuffer glyphTexture { sprite.width, sprite.height };

		u16 glyphU1 = sprite.width * i;
		u16 glyphV1 = 0;
		u16 glyphU2 = glyphU1 + sprite.width;
		u16 glyphV2 = sprite.height;

		glyphTexture.splice( spriteTexture, glyphU1, glyphV1, glyphU2, glyphV2, 0, 0 );

		Texture &texture = Assets::textures[sprite.textureID];
		GlyphID glyphID = texture.add_glyph( static_cast<Texture2DBuffer &&>( glyphTexture ) );
		if( sprite.glyphID == GLYPHID_MAX ) { sprite.glyphID = glyphID; } // Store the first glyph only
	}

	// Free spriteTexture
	ErrorIf( sprite.glyphID == GLYPHID_MAX, "Failed to split sprite glyphs!" );
	spriteTexture.free();

	// Register Sprite
	sprites.add( sprite );
}


void Sprites::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;

	Timer timer;

	// Binary - do nothing
	{
		// ...
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		assets_struct( header,
			"DiskSprite",
			//"TextureID texture;",
			//"GlyphID glyph;",
			"u32 glyph;",
			"u16 texture;",
			"u16 count;",
			"u16 width;",
			"u16 height;",
			"i16 xorigin;",
			"i16 yorigin;" );

		// Enums
		header.append( "enum\n{\n" );
		for( Sprite &sprite : sprites )
		{
			header.append( "\t" ).append( sprite.name ).append( ",\n" );
		}
		header.append( "};\n\n" );

		// Table
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 spritesCount = " );
		header.append( static_cast<int>( sprites.size() ) ).append( ";\n" );
		header.append( "\textern const DiskSprite sprites[];\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );

		// Table
		char buffer[PATH_SIZE];
		source.append( "\tconst DiskSprite sprites[spritesCount] =\n\t{\n" );
		for( Sprite &sprite : sprites )
		{
			snprintf( buffer, PATH_SIZE,
				"\t\t{ %d, %d, %d, %d, %d, %d, %d }, // %s\n",
				sprite.glyphID,
				sprite.textureID,
				sprite.count,
				sprite.width,
				sprite.height,
				sprite.xorigin,
				sprite.yorigin,

				sprite.name.cstr() );

			source.append( buffer );
		}
		source.append( "\t};\n" );
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = sprites.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d sprite%s", count, count == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////