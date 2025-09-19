#include <build/assets/fonts.hpp>

#include <vendor/stb/stb_truetype.hpp>

#include <core/list.hpp>
#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/json.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Fonts::gather( const char *path, const bool recurse )
{
	// Gather & Load fonts
	Timer timer;
	List<FileInfo> files;
	directory_iterate( files, path, ".font", recurse );
	for( FileInfo &fileInfo : files ) { load( fileInfo.path ); }

	// Log
	if( verbose_output() )
	{
		const u32 count = files.size();
		PrintColor( LOG_CYAN, TAB TAB "%u font%s found in: %s", count, count == 1 ? "" : "s", path );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}


u16 Fonts::load_ttf( const char *pathFont, const char *pathTTF )
{
	// Register TTF
	const usize ttfIndex = ttfs.count();
	ErrorIf( ttfIndex > U16_MAX, "Exceeded maximum number of TTFs!" );
	TTF &ttf = ttfs.add( { } );
	ttf.path = pathTTF;

	// Load TTF (try relative path first)
	char pathRelative[PATH_SIZE];
	path_get_directory( pathRelative, sizeof( pathRelative ), pathFont );
	strappend( pathRelative, SLASH ); strappend( pathRelative, pathTTF );
	if( !ttf.buffer.load( pathRelative ) )
	{
		// Relative path failed -- try absolute path
		ErrorIf( !ttf.buffer.load( pathTTF ), "Failed to load font ttf: %s", pathFont );
	}

	// Return TTF index
	return static_cast<u16>( ttfIndex );
}


static void load_license( Buffer &license, const char *pathFont, const char *pathLicense )
{
	// Load License (try relative path first)
	char pathRelative[PATH_SIZE];
	path_get_directory( pathRelative, sizeof( pathRelative ), pathFont );
	strappend( pathRelative, SLASH ); strappend( pathRelative, pathLicense );
	if( !license.load( pathRelative ) )
	{
		// Relative path failed -- try absolute path
		ErrorIf( !license.load( pathLicense ), "Failed to load font license: %s", pathFont );
	}
}


void Fonts::load( const char *path )
{
	// Open File
	String file;
	ErrorIf( !file.load( path ), "Unable to load font file: %s", path );
	JSON fontJSON { file };

	// Build Cache
	Assets::assetFileCount++;
	if( !Build::cacheDirtyAssets )
	{
		FileTime time;
		file_time( path, &time );
		Build::cacheDirtyAssets |= file_time_newer( time, Assets::timeCache );
	}

	// Register Font
	ErrorIf( fonts.count() >= U8_MAX, "Exceeded maximum number of fonts!" );
	Font &font = fonts.add( { } );
	font.path = path;

	// Extract Font Name
	char filename[PATH_SIZE];
	path_get_filename( filename, sizeof( filename ), path );
	String name = filename;
	font.name = name.substr( 0, name.find( "." ) );

	// Read file (json)
	font.licensePath = fontJSON.get_string( "license" );
	ErrorIf( font.licensePath.length_bytes() == 0, "Font '%s' has an invalid license (required)", path );
	String ttfDefaultPath = fontJSON.get_string( "default" );
	ErrorIf( ttfDefaultPath.length_bytes() == 0, "Font '%s' has an invalid default ttf (required)", path );
	String ttfItalicPath = fontJSON.get_string( "italic" );
	String ttfBoldPath = fontJSON.get_string( "bold" );
	String ttfBoldItalicPath = fontJSON.get_string( "bold_italic" );

	// Load License
	load_license( font.licenseData, path, font.licensePath );

	// Load TTFs
	font.ttfDefault = load_ttf( path, ttfDefaultPath );
	font.ttfItalic = ttfItalicPath ? load_ttf( path, ttfItalicPath ) : font.ttfDefault;
	font.ttfBold = ttfBoldPath ? load_ttf( path, ttfBoldPath ) : font.ttfDefault;
	font.ttfBoldItalic = ttfBoldItalicPath ? load_ttf( path, ttfBoldItalicPath ) : font.ttfBold;
}


void Fonts::write()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;

	Timer timer;

	// Binary
	{
		for( TTF &ttf : ttfs )
		{
			ttf.size = ttf.buffer.size();
			ErrorIf( ttf.size <= 0, "Invalid font ttf %s", ttf.path.cstr() );
			ttf.offset = binary.tell;
			binary.write( ttf.buffer.data, ttf.size );
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		header.append( "struct BinTTF;\n" );
		header.append( "struct BinFont;\n\n" );

		// Tables
		header.append( "namespace Assets\n{\n" );
		header.append( "\tconstexpr u32 ttfsCount = " );
		header.append( static_cast<int>( ttfs.size() ) ).append( ";\n" );
		header.append( "\textern const BinTTF ttfs[];\n" );
		header.append( "\tconstexpr u32 fontsCount = " );
		header.append( static_cast<int>( fonts.size() ) ).append( ";\n" );
		header.append( "\textern const BinFont fonts[];\n" );
		header.append( "}\n\n" );

		// Font Macros
		for( usize i = 0; i < fonts.count(); i++ )
		{
			const Font &font = fonts[i];
			header.append( "#define " ).append( font.name ).append( " Font { " );
			header.append( i ).append( ", " ).append( font.ttfDefault ).append( " }\n" );
		}
		header.append( "\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace Assets\n{\n" );
		char buffer[PATH_SIZE];
		{
			// BinTTF Table
			source.append( "\tconst BinTTF ttfs[ttfsCount] =\n\t{\n" );
			for( TTF &ttf : ttfs )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ %lluULL, %lluULL },\n",
					ttf.offset,
					ttf.size );

				source.append( buffer );
			}
			source.append( "\t};\n\n" );

			// BinFont Table
			source.append( "\tconst BinFont fonts[fontsCount] =\n\t{\n" );
			for( Font &font : fonts )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ { %u, %u, %u, %u }, DEBUG( \"%s\" ) },\n",
					font.ttfDefault,
					font.ttfItalic,
					font.ttfBold,
					font.ttfBoldItalic,
					font.name.cstr() );

				source.append( buffer );
			}
			source.append( "\t};\n\n" );
		}
		source.append( "}\n\n" );
	}

	// Write Licenses
	{
		char pathLicense[PATH_SIZE];
		for( Font &font : fonts )
		{
			strjoin_path( pathLicense, Build::pathOutputRuntimeLicenses, font.licensePath.cstr() );
			ErrorIf( !font.licenseData.save( pathLicense ), "Failed to write font license: %s", pathLicense );
		}
	}

	if( verbose_output() )
	{
		const usize count = fonts.size();
		PrintColor( LOG_CYAN, "\t\tWrote %d font%s", count, count == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////