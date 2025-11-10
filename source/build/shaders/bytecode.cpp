#include <build/shaders/bytecode.hpp>
#include <build/build.hpp>

#include <core/types.hpp>
#include <core/memory.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool compile_shader_bytecode_d3d11( String &source, Buffer &bytecode )
{
	// ...
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool compile_shader_bytecode_metal( String &source, Buffer &bytecode )
{
	static String pathMetal = String( Build::pathOutputGeneratedShaders ).append( SLASH ).append( "sh.metal" );
	static String pathAir = String( Build::pathOutputGeneratedShaders ).append( SLASH ).append( "sh.air" );
	static String pathMetallib = String( Build::pathOutputGeneratedShaders ).append( SLASH ).append( "sh.metallib" );
	static String compile =
		String( "xcrun -sdk macosx metal -c " ).append( pathMetal ).append( " -o " ).append( pathAir );
	static String link =
		String( "xcrun -sdk macosx metallib " ).append( pathAir ).append( " -o " ).append( pathMetallib );

	file_delete( pathMetal );
	file_delete( pathAir );
	file_delete( pathMetallib );

	if( !source.save( pathMetal ) ) { return false; }
	ErrorReturnIf( system( compile.cstr() ) != 0, false, "Failed to compile metal bytecode!" );
	ErrorReturnIf( system( link.cstr() ) != 0, false, "Failed to link metal bytecode!" );

	ErrorReturnIf( !bytecode.load( pathMetallib ), false, "Failed to load metal bytecode for packing!" );
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////