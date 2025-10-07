#include <build/assets/fonts.hpp>

#include <vendor/stb/stb_truetype.hpp>

#include <core/list.hpp>
#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>
#include <build/assets/textures.hpp>
#include <build/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheFont
{
	bool hasDefault;
	bool hasItalic;
	bool hasBold;
	bool hasBoldItalic;
};


struct CacheTTF
{
	usize offset;
	usize size;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize Fonts::gather( const char *path, const bool recurse )
{
	// Gather Fonts
	List<FileInfo> files;
	directory_iterate( files, path, ".font", recurse );

	// Process Fonts
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		process( fileInfo.path );
	}

	return files.size();
}


void Fonts::process( const char *path )
{
	// Local Directory
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate font file: %s", path );
		return;
	}

	// Open Definition JSON
	String fileDefinitionContents;
	if( !fileDefinitionContents.load( path ) )
	{
		Error( "Unable to open font file: %s", path );
		return;
	}
	JSON fileDefinitionJSON { fileDefinitionContents };

	// Helper: Register TTF
	auto register_ttf = [&]( const char type, const char *pathTTF, const CacheID cacheIDFont ) -> u16
	{
		const usize ttfIndex = ttfs.count();
		ErrorIf( ttfIndex >= U16_MAX, "Exceeded maximum number of TTFs!" );

		static char cacheIDBuffer[PATH_SIZE * 2];
		memory_set( cacheIDBuffer, 0, sizeof( cacheIDBuffer ) );
		snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ), "ttf_%c %s|%u", type, pathTTF, cacheIDFont );
		const CacheID cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );

		TTF &ttf = ttfs.add( TTF { } );
		ttf.cacheID = cacheID;
		ttf.path = pathTTF;

		return static_cast<u16>( ttfIndex );
	};

	// Parse Definition JSON
	AssetFile fileDefault;
	static char pathDefault[PATH_SIZE];
	String pathDefaultRelative = fileDefinitionJSON.get_string( "default" );
	const bool hasDefault = !pathDefaultRelative.is_empty();
	ErrorIf( !hasDefault, "Font '%s' has an invalid default path (required)", fileDefinition.name );
	snprintf( pathDefault, sizeof( pathDefault ),
		"%s" SLASH "%s", pathDirectory, pathDefaultRelative.cstr() );
	ErrorIf( hasDefault && !asset_file_register( fileDefault, pathDefault ),
		"Font '%s' - Unable to locate \"default\" file: '%s'",
		fileDefinition.name, pathDefault );

	AssetFile fileItalic;
	static char pathItalic[PATH_SIZE];
	String pathItalicRelative = fileDefinitionJSON.get_string( "italic" );
	const bool hasItalic = !pathItalicRelative.is_empty();
	snprintf( pathItalic, sizeof( pathItalic ),
		"%s" SLASH "%s", pathDirectory, pathItalicRelative.cstr() );
	ErrorIf( hasItalic && !asset_file_register( fileItalic, pathItalic ),
		"Font '%s' - Unable to locate \"italic\" file: '%s'",
		fileDefinition.name, pathItalic );

	AssetFile fileBold;
	static char pathBold[PATH_SIZE];
	String pathBoldRelative = fileDefinitionJSON.get_string( "bold" );
	const bool hasBold = !pathBoldRelative.is_empty();
	snprintf( pathBold, sizeof( pathBold ),
		"%s" SLASH "%s", pathDirectory, pathBoldRelative.cstr() );
	ErrorIf( hasBold && !asset_file_register( fileBold, pathBold ),
		"Font '%s' - Unable to locate \"bold\" file: '%s'",
		fileDefinition.name, pathBold );

	AssetFile fileBoldItalic;
	static char pathBoldItalic[PATH_SIZE];
	String pathBoldItalicRelative = fileDefinitionJSON.get_string( "bold_italic" );
	const bool hasBoldItalic = !pathBoldItalicRelative.is_empty();
	snprintf( pathBoldItalic, sizeof( pathBoldItalic ),
		"%s" SLASH "%s", pathDirectory, pathBoldItalicRelative.cstr() );
	ErrorIf( hasBoldItalic && !asset_file_register( fileBoldItalic, pathBoldItalic ),
		"Font '%s' - Unable to locate \"bold_italic\" file: '%s'",
		fileDefinition.name, pathBoldItalic );

	AssetFile fileLicense;
	static char pathLicense[PATH_SIZE];
	String pathLicenseRelative = fileDefinitionJSON.get_string( "license" );
	const bool hasLicense = !pathLicenseRelative.is_empty();
	ErrorIf( !hasLicense, "Font '%s' has an invalid license (required)", fileDefinition.name );
	snprintf( pathLicense, sizeof( pathLicense ),
		"%s" SLASH "%s", pathDirectory, pathLicenseRelative.cstr() );
	ErrorIf( hasLicense && !asset_file_register( fileLicense, pathLicense ),
		"Font '%s' - Unable to locate \"license\" file: '%s'",
		fileDefinition.name, pathLicense );
	static char pathLicenseFilename[PATH_SIZE];
	path_get_filename( pathLicenseFilename, sizeof( pathLicenseFilename ), pathLicenseRelative );

	// Generate Cache ID
	static char cacheIDBuffer[PATH_SIZE * 8];
	memory_set( cacheIDBuffer, 0, sizeof( cacheIDBuffer ) );
	snprintf( cacheIDBuffer, sizeof( cacheIDBuffer ),
		"font %s|%llu %s|%llu %s|%llu %s|%llu %s|%llu %s|%llu",
		fileDefinition.path, fileDefinition.time.as_u64(),
		hasDefault ? fileDefault.path : "", fileDefault.time.as_u64(),
		hasItalic ? fileItalic.path : "", fileItalic.time.as_u64(),
		hasBold ? fileBold.path : "", fileBold.time.as_u64(),
		hasBoldItalic ? fileBoldItalic.path : "", fileBoldItalic.time.as_u64(),
		hasLicense ? fileLicense.path : "null", fileLicense.time.as_u64() );
	const CacheID cacheID = checksum_xcrc32( cacheIDBuffer, sizeof( cacheIDBuffer ), 0 );

	// Check Cache
	CacheFont cacheFont;
	if( !Assets::cache.fetch( cacheID, cacheFont ) )
	{
		Assets::cache.dirty |= true; // Dirty Cache
	}

	// Register Font
	Font &font = fonts.add( Font { } );
	font.name = fileDefinition.name;
	font.ttfDefault = register_ttf( 'd', pathDefault, cacheID );
	font.ttfItalic = hasItalic ? register_ttf( 'i', pathItalic, cacheID ) : font.ttfDefault;
	font.ttfBold = hasBold ? register_ttf( 'b', pathBold, cacheID ) : font.ttfDefault;
	font.ttfBoldItalic = hasBoldItalic ? register_ttf( 't', pathBoldItalic, cacheID ) : font.ttfDefault;
	font.pathLicense = hasLicense ? pathLicense : "";

	// Cache
	cacheFont.hasDefault = hasDefault;
	cacheFont.hasItalic = hasItalic;
	cacheFont.hasBold = hasBold;
	cacheFont.hasBoldItalic = hasBoldItalic;
	Assets::cache.store( cacheID, cacheFont );
}


void Fonts::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 countTTF = static_cast<u32>( ttfs.size() );
	const u32 countFont = static_cast<u32>( fonts.size() );

	Timer timer;

	// Load
	{
		for( TTF &ttf : ttfs )
		{
			char path[PATH_SIZE];
			path_get_filename( path, sizeof( path ), ttf.path.cstr() );

			CacheTTF cacheTTF;
			if( Assets::cache.fetch( ttf.cacheID, cacheTTF ) )
			{
				// Load TTF from cached binary
				ttf.data.write_from_file( Build::pathOutputRuntimeBinary,
					Assets::cacheReadOffset + cacheTTF.offset, cacheTTF.size );
				Assets::log_asset_cache( "Font", path );
			}
			else
			{
				// Load TTF data from file
				ErrorIf( !ttf.data.load( ttf.path ), "Failed to load ttf: %s", ttf.path.cstr() );
				Assets::log_asset_build( "Font", path );
			}
		}

		for( Font &font : fonts )
		{
			// Load license (Note: license file isn't cached)
			ErrorIf( !font.dataLicense.load( font.pathLicense ),
				"Font '%s' - failed to load license file: %s", font.name.cstr(), font.pathLicense.cstr() );
		}
	}

	// Binary
	{
		for( TTF &ttf : ttfs )
		{
			// Binary
			ErrorIf( ttf.data.size() <= 0, "Invalid font ttf %s %llu", ttf.path.cstr() );
			ttf.offset = binary.write( ttf.data.data, ttf.data.size() );
			ttf.size = ttf.data.size();

			// Cache
			CacheTTF cacheTTF;
			cacheTTF.offset = ttf.offset;
			cacheTTF.size = ttf.size;
			Assets::cache.store( ttf.cacheID, cacheTTF );
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Struct
		header.append( "namespace Assets { struct TTFEntry; }\n" );
		header.append( "namespace Assets { struct FontEntry; }\n\n" );

		// Tables
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 ttfCount = " ).append( countTTF ).append( ";\n" );
		header.append( countTTF > 0 ? "\textern const Assets::TTFEntry ttfs[ttfCount];\n" :
			"\textern const Assets::TTFEntry *ttfs;\n" );

		header.append( "\tconstexpr u32 fontCount = " ).append( countFont ).append( ";\n" );
		header.append( countFont > 0 ? "\textern const Assets::FontEntry fonts[fontCount];\n" :
			"\textern const Assets::FontEntry *fonts;\n" );
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
		source.append( "namespace CoreAssets\n{\n" );

		if( countTTF > 0 )
		{
			// Assets::TTFEntry Table
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::TTFEntry ttfs[ttfCount] =\n\t{\n" );
			for( TTF &ttf : ttfs )
			{
				snprintf( buffer, PATH_SIZE,
					"\t\t{ BINARY_OFFSET_ASSETS + %lluLLU, %lluLLU },\n",
					ttf.offset,
					ttf.size );

				source.append( buffer );
			}
			source.append( "\t};\n\n" );
		}
		else
		{
			source.append( "\tconst Assets::TTFEntry *ttfs = nullptr;\n" );
		}

		if( countFont > 0 )
		{
			// Assets::FontEntry Table
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::FontEntry fonts[fontCount] =\n\t{\n" );
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
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::FontEntry *fonts = nullptr;\n" );
		}
		source.append( "}\n\n" );
	}

	// Write Licenses
	{
		static char pathLicenseFilename[PATH_SIZE];
		static char pathLicenseOutput[PATH_SIZE];

		for( Font &font : fonts )
		{
			if( font.pathLicense.is_empty() ) { continue; }

			path_get_filename( pathLicenseFilename, sizeof( pathLicenseFilename ), font.pathLicense );
			strjoin_path( pathLicenseOutput, Build::pathOutputRuntimeLicenses, pathLicenseFilename );

			ErrorIf( !font.dataLicense.save( pathLicenseOutput ),
				"Font '%s' - failed to write font license: %s", font.name.cstr(), font.pathLicense.cstr() );
		}
	}

	if( verbose_output() )
	{
		const usize count = fonts.size();
		PrintColor( LOG_CYAN, TAB TAB "Wrote %d font%s", count, count == 1 ? "" : "s" );
		PrintLnColor( LOG_WHITE, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////