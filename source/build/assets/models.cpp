#include <build/assets/models.hpp>

#include <core/types.hpp>
#include <core/debug.hpp>
#include <core/list.hpp>
#include <core/json.hpp>
#include <core/checksum.hpp>
#include <core/math.hpp>

#include <build/build.hpp>
#include <build/assets.hpp>

#include <build/assets/materials.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CacheModel
{
	u32 meshCount;
	u32 meshCacheKeys[MODEL_MESH_COUNT_MAX];
	Assets::MeshFormatTypeVertex formatVertex;
	float x1, y1, z1;
	float x2, y2, z2;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VertexPosition
{
	float_v3 position;
};


struct VertexPositionUV
{
	float_v3 position;
	u16_v2 uv;
};


struct VertexPositionNormal
{
	float_v3 position;
	float_v3 normal;
};


struct VertexPositionNormalUV
{
	float_v3 position;
	float_v3 normal;
	u16_v2 uv;
};


struct VertexPositionNormalTangentUV
{
	float_v3 position;
	float_v3 normal;
	float_v3 tangent;
	u16_v2 uv;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Tangents
{
	float_v3 sTangent = float_v3 { 0.0f, 0.0f, 0.0f };
	float_v3 tTangent = float_v3 { 0.0f, 0.0f, 0.0f };
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ModelID Models::register_new( const Model &model )
{
	AssertMsg( models.count() < MODELID_MAX, "Exceeded max number of Models" );
	models.add( model );
	return static_cast<ModelID>( models.count() - 1 );
}


ModelID Models::register_new( Model &&model )
{
	AssertMsg( models.count() < MODELID_MAX, "Exceeded max number of Models" );
	models.add( static_cast<Model &&>( model ) );
	return static_cast<ModelID>( models.count() - 1 );
}


ModelID Models::register_new_from_definition( String name, const char *path )
{
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Register Definition File
	AssetFile fileDefinition;
	if( !asset_file_register( fileDefinition, path ) )
	{
		Error( "Unable to locate model file: %s", path );
		return MODELID_NULL;
	}

	// Open Definition JSON
	String fileDefinitionContents;
	if( !fileDefinitionContents.load( path ) )
	{
		Error( "Unable to open model file: %s", path );
		return MODELID_NULL;
	}
	JSON fileDefinitionJSON { fileDefinitionContents };

	// Parse Model
	static char pathModel[PATH_SIZE];
	String pathModelRelative = fileDefinitionJSON.get_string( "model" );
	ErrorIf( pathModelRelative.is_empty(), "Model '%s' has an invalid (empty) model path", fileDefinition.name );
	snprintf( pathModel, sizeof( pathModel ), "%s" SLASH "%s", pathDirectory, pathModelRelative.cstr() );
	static char nameModel[PATH_SIZE];
	path_remove_extension( nameModel, sizeof( nameModel ), fileDefinition.name );

	// Parse Vertex Format
	String vertexFormatName = fileDefinitionJSON.get_string( "format" );
	Assets::MeshFormatTypeVertex formatVertex = Assets::MESHFORMATTYPEVERTEX_COUNT;
	for( Assets::MeshFormatTypeVertex i = 0; i < Assets::MESHFORMATTYPEVERTEX_COUNT; i++ )
	{
		if( streq_case_insensitive( vertexFormatName.cstr(), Assets::MeshFormatTypeVertexNames[i] ) )
		{
			formatVertex = i;
			break;
		}
	}

	if( formatVertex == Assets::MESHFORMATTYPEVERTEX_COUNT )
	{
		formatVertex = Assets::MeshFormatTypeVertex_Default;
		if( !vertexFormatName.is_empty() )
		{
			char knownFormats[512] = { '\0' };
			for( Assets::MeshFormatTypeVertex i = 0; i < Assets::MESHFORMATTYPEVERTEX_COUNT; i++ )
			{
				strappend( knownFormats, Assets::MeshFormatTypeVertexNames[i] );
				if( i < Assets::MESHFORMATTYPEVERTEX_COUNT - 1 ) { strappend( knownFormats, ", " ); }
			}
			Warning( "Model '%s' has unknown 'format' type: %s\n" TAB "Options are: %s\n",
				nameModel, vertexFormatName.cstr(), knownFormats );
		}
	}

	AssetFile fileModel;
	if( !asset_file_register( fileModel, pathModel ) )
	{
		Error( "Model '%s' - unable to locate model file: %s", name.cstr(), pathModel );
		return MODELID_NULL;
	}

	// Parse Skins
	List<SkinID> skinAssets;
	JSON skinsList = fileDefinitionJSON.array( "skins" );
	const usize skinsCount = skinsList.count();
	static char pathSkin[PATH_SIZE];

	for( usize i = 0; i < skinsCount; i++ )
	{
		String pathSkinRelative = skinsList.get_string_at( i );
		ErrorIf( pathSkinRelative.is_empty(), "Model '%s' has an invalid (empty) skin path", fileDefinition.name );
		snprintf( pathSkin, sizeof( pathSkin ), "%s" SLASH "%s", pathDirectory, pathSkinRelative.cstr() );

		static char nameSkin[PATH_SIZE];
		path_remove_extension( nameSkin, sizeof( nameSkin ), pathSkinRelative.cstr() );

		String skinAssetName = String( nameModel ).append( "_" ).append( nameSkin );
		skinAssets.add( Assets::skins.register_new_from_file( skinAssetName, pathSkin ) );
	}

	char fileExtension[16];
	path_get_extension( fileExtension, sizeof( fileExtension ), pathModel );
	strlower( fileExtension );

	// Register Model
	const ModelID modelID = register_new();
	Model &model = models[modelID];
	model.formatVertex = formatVertex;
	model.skinID = skinAssets.count() == 0 ? U32_MAX : skinAssets[0];
	model.cacheKey = fileModel.cache_id();
	model.name = name;
	model.path = pathModel;

	// Check Cache
	CacheModel cache;
	if( !Assets::cache.contains( model.cacheKey ) ) { Assets::cache.dirty |= true; }

	return MODELID_NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usize Models::gather( const char *path, bool recurse )
{
	List<FileInfo> files;
	directory_iterate( files, path, ".model", recurse );

	static char name[PATH_SIZE];
	for( FileInfo &fileInfo : files )
	{
		Assets::cacheFileCount++;
		path_remove_extension( name, sizeof( name ), fileInfo.name );
		register_new_from_definition( name, fileInfo.path );
	}

	return files.count();
}


void Models::build()
{
	Buffer &binary = Assets::binary;
	String &header = Assets::header;
	String &source = Assets::source;
	const u32 count = static_cast<u32>( models.count() );

	Timer timer;

	// Load
	{
		for( Model &model : models )
		{
			// Load
			if( !model.load_from_cache() )
			{
				static char fileExtension[16];
				path_get_extension( fileExtension, sizeof( fileExtension ), model.path.cstr() );
				strlower( fileExtension );

				if( streq( fileExtension, ".obj" ) )
				{
					model.load_from_obj();
				}
				else
				{
					Error( "Model '%s' has unknown model file type: '%s'", model.name.cstr(), fileExtension );
				}

				Assets::log_asset_build( "Model", model.name.cstr() );
			}
			else
			{
				Assets::log_asset_cache( "Model", model.name.cstr() );
			}

			// Cache
			CacheModel cache;
			cache.meshCount = model.meshes.count();
			for( u32 i = 0; i < cache.meshCount; i++ )
			{
				cache.meshCacheKeys[i] = Assets::meshes[model.meshes[i]].cacheKey;
			}
			cache.formatVertex = model.formatVertex;
			cache.x1 = model.x1;
			cache.y1 = model.y1;
			cache.z1 = model.z1;
			cache.x2 = model.x2;
			cache.y2 = model.y2;
			cache.z2 = model.z2;
			Assets::cache.store( model.cacheKey, cache );
		}
	}

	// Header
	{
		// Group
		assets_group( header );

		// Enums
		header.append( "enum_class\n(\n\tModel, u32,\n\n" );
		for( Model &model : models ) { header.append( "\t" ).append( model.name ).append( ",\n" ); }
		header.append( "\n\tNull = 0,\n" );
		header.append( ");\n\n" );

		// Struct
		header.append( "namespace Assets { struct ModelEntry; }\n\n" );

		// Entries
		header.append( "namespace CoreAssets\n{\n" );
		header.append( "\tconstexpr u32 modelCount = " ).append( count ).append( ";\n" );
		header.append( count > 0 ? "\textern const Assets::ModelEntry models[modelCount];\n" :
			"\textern const Assets::ModelEntry *models;\n" );
		header.append( "}\n\n" );
	}

	// Source
	{
		// Group
		assets_group( source );
		source.append( "namespace CoreAssets\n{\n" );
		if( count > 0 )
		{
			// Assets::ModelEntry Table
			char buffer[PATH_SIZE];
			source.append( "\tconst Assets::ModelEntry models[modelCount] =\n\t{\n" );
			for( Model &model : models )
			{
				source.append( "\t\t// " ).append( model.name ).append( "\n" );
				source.append( "\t\t{\n" );

				const u32 meshCount = static_cast<u32>( model.meshes.count() );
				source.append( "\t\t\t" ).append( meshCount ).append( ",\n" );

				source.append( "\t\t\t{ " );
				for( u32 i = 0; i < MODEL_MESH_COUNT_MAX; i++ )
				{
					source.append( i < model.meshes.count() ? model.meshes[i] : 0 );
					source.append( i < MODEL_MESH_COUNT_MAX - 1 ? ", " : " " );
				}
				source.append( "},\n" );

				snprintf( buffer, sizeof( buffer ), "%u, %ff, %ff, %ff, %ff, %ff, %ff",
					model.formatVertex,
					model.x1,
					model.y1,
					model.z1,
					model.x2,
					model.y2,
					model.z2 );
				source.append( "\t\t\t" ).append( buffer ).append("\n");

				source.append( "\t\t},\n" );
			}
			source.append( "\t};\n" );
		}
		else
		{
			source.append( "\tconst Assets::ModelEntry *models = nullptr;\n" );
		}
		source.append( "}\n\n" );
	}

	if( verbose_output() )
	{
		const usize count = models.count();
		Print( PrintColor_White, TAB TAB "Wrote %d model%s", count, count == 1 ? "" : "s" );
		PrintLn( PrintColor_White, " (%.3f ms)", timer.elapsed_ms() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VertexTuple
{
	u32 positionIndex;
	u32 texcoordIndex;
	u32 normalIndex;
	u32 tangentIndex;
};


static bool is_number( const char c )
{
	return ( c >= '0' && c <= '9' ) || c == '+' || c == '-' || c == '.';
}


static usize seek_to_next_number( const String &file, usize &tell )
{
	// Early out
	if( file[tell] == '\n' || file[tell] == '\0' )
	{
		return USIZE_MAX;
	}

	// Seek to beginning of a number
	usize start = tell;
	while( !is_number( file[start] ) )
	{
		if( file[start] == '\n' || file[start] == '\0' )
		{
			tell = start;
			return USIZE_MAX;
		}

		start++;
	}

	// Seek to end of number
	usize end = start;
	while( is_number( file[end] ) ) { end++; }
	tell = end;

	// Return number position
	return start == end ? USIZE_MAX : start;
}


static bool next_float( const String &file, usize &tell, float &out )
{
	// Seek to beginning of a number
	usize index = seek_to_next_number( file, tell );
	if( index == USIZE_MAX) { out = 0.0f; return false; }

	// Read float
	out = static_cast<float>( atof( file.get_pointer( index ) ) );
	return true;
}


static bool next_u32( const String &file, usize &tell, u32 &out )
{
	// Seek to beginning of a number
	usize index = seek_to_next_number( file, tell );
	if( index == USIZE_MAX ) { out = 0; return false; }

	// Read float
	out = static_cast<u32>( atoi( file.get_pointer( index ) ) );
	return true;
}


static bool next_line( const String &file, usize tell, String &out )
{
	const usize newline = file.find( "\n", tell );
	if( newline <= tell ) { return false; }
	out = file.substr( tell, newline ).trim();
	return true;
}


static usize next_key( const String &file, usize &tell, const char *key )
{
	tell = file.find( key, tell );
	if( tell == USIZE_MAX ) { return tell; }
	tell += strlen( key );
	return tell;
}


static usize try_find_next_key( const String &file, const usize start, const usize end, const char *key )
{
	usize next = file.find( key, start, end );
	if( next == USIZE_MAX ) { return USIZE_MAX; }
	next += strlen( key );
	return next;
}


static bool obj_next_vertex_position( const String &file, usize &tell, List<float_v3> &positions )
{
	tell = next_key( file, tell, "v " );
	if( tell == USIZE_MAX ) { return false; }

	float_v3 &vertex = positions.add( float_v3 { } );
	next_float( file, tell, vertex.x );
	next_float( file, tell, vertex.y );
	next_float( file, tell, vertex.z );
	return true;
};


static bool obj_next_vertex_normal( const String &file, usize &tell, List<float_v3> &normals )
{
	tell = next_key( file, tell, "vn " );
	if( tell == USIZE_MAX ) { return false; }

	float_v3 &normal = normals.add( float_v3 { } );
	next_float( file, tell, normal.x );
	next_float( file, tell, normal.y );
	next_float( file, tell, normal.z );
	return true;
};


static bool obj_next_vertex_texcoord( const String &file, usize &tell, List<float_v2> &texcoords )
{
	tell = next_key( file, tell, "vt " );
	if( tell == USIZE_MAX ) { return false; }

	float_v2 &uv = texcoords.add( float_v2 { } );
	next_float( file, tell, uv.x );
	next_float( file, tell, uv.y );
	// next_float( file, tell, uv.w ); TODO: Is this ever even used?
	return true;
};


static bool obj_next_face( const String &file, usize &tell, usize end,
	const List<float_v3> &positions,
	const List<float_v2> &textureCoords,
	const List<float_v3> &normals,
	List<Tangents> &tangents,
	HashMap<u64, u32> &faces,
	List<VertexTuple> &vertices,
	List<u32> &indices )
{
	tell = next_key( file, tell, "f " );
	if( tell >= end ) { return false; }

	// TODO: Support quads as well
	for( int i = 0; i < 3; i++ )
	{
		VertexTuple tuple;
		next_u32( file, tell, tuple.positionIndex );
		next_u32( file, tell, tuple.texcoordIndex );
		next_u32( file, tell, tuple.normalIndex );
		const u64 key = Hash::hash64_from( tuple );

		if( faces.contains( key ) )
		{
			const u32 index = faces.get( key );
			indices.add( index );
		}
		else
		{
			const u32 index = vertices.count();
			tangents.add( Tangents { } );
			tuple.tangentIndex = tangents.count();
			vertices.add( tuple );
			indices.add( index );
			faces.add( key, index );
		}
	}

	return true;
}


static void obj_parse_vertex_positions( const String &file, List<float_v3> &positions )
{
	usize tell = 0LLU;
	while( obj_next_vertex_position( file, tell, positions ) ) { }
}


static void obj_parse_vertex_texcoords( const String &file, List<float_v2> &texcoords )
{
	usize tell = 0LLU;
	while( obj_next_vertex_texcoord( file, tell, texcoords ) ) { }
}


static void obj_parse_vertex_normals( const String &file, List<float_v3> &normals )
{
	usize tell = 0LLU;
	while( obj_next_vertex_normal( file, tell, normals ) ) { }
}


static void obj_parse_mesh_faces( const String &file, usize start, usize end,
	const List<float_v3> &positions,
	const List<float_v2> &texcoords,
	const List<float_v3> &normals,
	List<Tangents> &tangents,
	HashMap<u64, u32> &faces, List<VertexTuple> &vertices, List<u32> &indices )
{
	while( obj_next_face( file, start, end, positions, texcoords, normals, tangents, faces, vertices, indices ) ) { }
}


static void build_vertex_buffer_position( Buffer &buffer,
	const List<VertexTuple> &vertices,
	const List<float_v3> &positions )
{
	for( usize i = 0; i < vertices.count(); i++ )
	{
		const VertexTuple &vertex = vertices[i];
		const float_v3 &position = positions[vertex.positionIndex - 1];
		buffer.write( VertexPosition
			{
				.position = position
			} );
	}
}


static void build_vertex_buffer_position_uv( Buffer &buffer,
	const List<VertexTuple> &vertices,
	const List<float_v3> &positions,
	const List<float_v2> &texcoords )
{
	for( usize i = 0; i < vertices.count(); i++ )
	{
		const VertexTuple &vertex = vertices[i];
		const float_v3 &position = positions[vertex.positionIndex - 1];
		const float_v2 &texcoord = texcoords[vertex.texcoordIndex - 1];
		buffer.write( VertexPositionUV
			{
				.position = position,
				.uv = u16_v2
					{
						static_cast<u16>( texcoord.x * 0xFFFF ),
						static_cast<u16>( ( 1.0f - texcoord.y ) * 0xFFFF ),
					},
			} );
	}
}


static void build_vertex_buffer_position_normal( Buffer &buffer,
	const List<VertexTuple> &vertices,
	const List<float_v3> &positions,
	const List<float_v3> &normals )
{
	for( usize i = 0; i < vertices.count(); i++ )
	{
		const VertexTuple &vertex = vertices[i];
		const float_v3 &position = positions[vertex.positionIndex - 1];
		const float_v3 &normal = normals[vertex.normalIndex - 1];
		buffer.write( VertexPositionNormal
			{
				.position = position,
				.normal = normal
			} );
	}
}


static void calculate_tangents( List<Tangents> &tangents,
	const List<VertexTuple> &vertices,
	const List<u32> &indices,
	const List<float_v3> &positions,
	const List<float_v2> &texcoords )
{
	Assert( vertices.count() == indices.count() );

	for( usize i = 0; i < indices.count(); i += 3 )
	{
		const VertexTuple &v0 = vertices[indices[i + 0]];
		const VertexTuple &v1 = vertices[indices[i + 1]];
		const VertexTuple &v2 = vertices[indices[i + 2]];

		const float_v3 &p0 = positions[v0.positionIndex - 1];
		const float_v3 &p1 = positions[v1.positionIndex - 1];
		const float_v3 &p2 = positions[v2.positionIndex - 1];
		const float_v2 &w0 = texcoords[v0.texcoordIndex - 1];
		const float_v2 &w1 = texcoords[v1.texcoordIndex - 1];
		const float_v2 &w2 = texcoords[v2.texcoordIndex - 1];

		float x1 = p1.x - p0.x;
		float x2 = p2.x - p0.x;
		float y1 = p1.y - p0.y;
		float y2 = p2.y - p0.y;
		float z1 = p1.z - p0.z;
		float z2 = p2.z - p0.z;

		float s1 = w1.x - w0.x;
		float s2 = w2.x - w0.x;
		float t1 = w1.y - w0.y;
		float t2 = w2.y - w0.y;

		float r = 1.0f / ( s1 * t2 - s2 * t1 );

		float_v3 sdir = float_v3
		{
			( t2 * x1 - t1 * x2 ) * r,
			( t2 * y1 - t1 * y2 ) * r,
			( t2 * z1 - t1 * z2 ) * r,
		};

		float_v3 tdir = float_v3
		{
			( s1 * x2 - s2 * x1 ) * r,
			( s1 * y2 - s2 * y1 ) * r,
			( s1 * z2 - s2 * z1 ) * r,
		};

		tangents[v0.tangentIndex - 1].sTangent += sdir;
		tangents[v1.tangentIndex - 1].sTangent += sdir;
		tangents[v2.tangentIndex - 1].sTangent += sdir;
		tangents[v0.tangentIndex - 1].tTangent += tdir;
		tangents[v1.tangentIndex - 1].tTangent += tdir;
		tangents[v2.tangentIndex - 1].tTangent += tdir;
	}
}


static void build_vertex_buffer_position_normal_uv( Buffer &buffer,
	const List<VertexTuple> &vertices,
	const List<float_v3> &positions,
	const List<float_v3> &normals,
	const List<float_v2> &texcoords )
{
	for( usize i = 0; i < vertices.count(); i++ )
	{
		const VertexTuple &vertex = vertices[i];
		const float_v3 &position = positions[vertex.positionIndex - 1];
		const float_v3 &normal = normals[vertex.normalIndex - 1];
		const float_v2 &texcoord = texcoords[vertex.texcoordIndex - 1];
		buffer.write( VertexPositionNormalUV
			{
				.position = position,
				.normal = normal,
				.uv = u16_v2
					{
						static_cast<u16>( texcoord.x * 0xFFFF ),
						static_cast<u16>( ( 1.0f - texcoord.y ) * 0xFFFF ),
					},
			} );
	}
}


static void build_vertex_buffer_position_normal_tangent_uv( Buffer &buffer,
	const List<VertexTuple> &vertices,
	const List<float_v3> &positions,
	const List<float_v3> &normals,
	const List<Tangents> &tangents,
	const List<float_v2> &texcoords )
{
	for( usize i = 0; i < vertices.count(); i++ )
	{
		const VertexTuple &vertex = vertices[i];
		const float_v3 &position = positions[vertex.positionIndex - 1];
		const float_v3 &normal = normals[vertex.normalIndex - 1];
		const float_v2 &texcoord = texcoords[vertex.texcoordIndex - 1];
		const Tangents &tangentST = tangents[vertex.tangentIndex - 1];

		const float_v3 tangent = float_v3_normalize(
			tangentST.sTangent - normal * float_v3_dot( normal, tangentST.sTangent ) );
		const float handedness = float_v3_dot( float_v3_cross( normal, tangentST.sTangent ), tangentST.tTangent ) < 0.0f ?
			-1.0f : 1.0f;

		buffer.write( VertexPositionNormalTangentUV
			{
				.position = position,
				.normal = normal,
				.tangent = tangent,
				.uv = u16_v2
					{
						static_cast<u16>( texcoord.x * 0xFFFF ),
						static_cast<u16>( ( 1.0f - texcoord.y ) * 0xFFFF ),
					},
			} );
	}
}


bool Model::load_from_obj()
{
	static char pathDirectory[PATH_SIZE];
	path_get_directory( pathDirectory, sizeof( pathDirectory ), path );

	// Load Model File
	String obj;
	if( !obj.load( path ) ) { return false; }

	// Parse Data
	static List<float_v3> positions;
	static List<float_v2> texcoords;
	static List<float_v3> normals;
	static List<Tangents> tangents;

	positions.clear();
	texcoords.clear();
	normals.clear();
	tangents.clear();
	obj_parse_vertex_positions( obj, positions );
	obj_parse_vertex_texcoords( obj, texcoords );
	obj_parse_vertex_normals( obj, normals );

	// Parse Faces
	static HashMap<u64, u32> faces;
	static List<VertexTuple> vertices;
	static List<u32> indices;

	usize offsetFirstFace = obj.find( "f ", 0LLU, USIZE_MAX );
	usize offsetFirstUseMTL = obj.find( "usemtl ", 0LLU, USIZE_MAX );
	usize offsetCurrent = min( offsetFirstFace, offsetFirstUseMTL );
	u32 meshIndex = 0;

	this->x1 = FLOAT_MAX;
	this->y1 = FLOAT_MAX;
	this->z1 = FLOAT_MAX;
	this->x2 = FLOAT_MIN;
	this->y2 = FLOAT_MIN;
	this->z2 = FLOAT_MIN;

	for( ;; )
	{
		// Nothing left to parse, we're done
		if( offsetCurrent == USIZE_MAX ) { break; }

		// Note: Models are broken up into meshes based on materials
		const usize offsetNext = obj.find( "usemtl ", offsetCurrent + 1, USIZE_MAX );
		faces.clear();
		vertices.clear();
		indices.clear();
		obj_parse_mesh_faces( obj, offsetCurrent, offsetNext,
			positions, texcoords, normals, tangents, faces, vertices, indices );

		// Parse Material (usemtl )
		u32 skinSlotIndex = U32_MAX;
		if( skinID != U32_MAX )
		{
			String materialName;
			usize materialNameOffset = try_find_next_key( obj, offsetCurrent, offsetNext, "usemtl " );
			if( materialNameOffset != USIZE_MAX && next_line( obj, materialNameOffset, materialName ) )
			{
				const u32 materialKey = Hash::hash( materialName );
				ErrorIf( !Assets::skins[skinID].contains_material( materialKey ),
					"Model '%s' references undefined material '%s'",
					this->name.cstr(), materialName.cstr() );
				skinSlotIndex = Assets::skins[skinID].get_material_slot( materialKey );
			}
		}

		// Model AABB Min/Max
		float meshX1 = FLOAT_MAX;
		float meshY1 = FLOAT_MAX;
		float meshZ1 = FLOAT_MAX;
		float meshX2 = FLOAT_MIN;
		float meshY2 = FLOAT_MIN;
		float meshZ2 = FLOAT_MIN;
		for( usize i = 0; i < vertices.count(); i++ )
		{
			const VertexTuple &vertex = vertices[i];
			const float_v3 &position = positions[vertex.positionIndex - 1];
			meshX1 = min( position.x, meshX1 );
			this->x1 = min( this->x1, meshX1 );
			meshY1 = min( position.y, meshY1 );
			this->y1 = min( this->y1, meshY1 );
			meshZ1 = min( position.z, meshZ1 );
			this->z1 = min( this->z1, meshZ1 );
			meshX2 = max( position.x, meshX2 );
			this->x2 = max( this->x2, meshX2 );
			meshY2 = max( position.y, meshY2 );
			this->y2 = max( this->y2, meshY2 );
			meshZ2 = max( position.z, meshZ2 );
			this->z2 = max( this->z2, meshZ2 );
		}

		// Build Vertex Buffer
		Buffer vertexBuffer;
		switch( this->formatVertex )
		{
			case Assets::MeshFormatTypeVertex_Position:
				build_vertex_buffer_position( vertexBuffer, vertices, positions );
			break;

			case Assets::MeshFormatTypeVertex_PositionUV:
				build_vertex_buffer_position_uv( vertexBuffer, vertices, positions, texcoords );
			break;

			case Assets::MeshFormatTypeVertex_PositionNormal:
				build_vertex_buffer_position_normal( vertexBuffer, vertices, positions, normals );
			break;

			case Assets::MeshFormatTypeVertex_PositionNormalUV:
				build_vertex_buffer_position_normal_uv( vertexBuffer, vertices, positions, normals, texcoords );
			break;

			case Assets::MeshFormatTypeVertex_PositionNormalTangentUV:
				calculate_tangents( tangents, vertices, indices, positions, texcoords );
				build_vertex_buffer_position_normal_tangent_uv( vertexBuffer, vertices, positions, normals, tangents, texcoords );
			break;
		}

		const CacheKey cacheKeyMesh = Hash::hash32_from( this->cacheKey, meshIndex );
		const MeshID meshID = Assets::meshes.register_new( cacheKeyMesh,
			this->formatVertex, vertexBuffer.data, vertexBuffer.size(),
			Assets::MeshFormatTypeIndex_U32, indices.data, indices.count() * sizeof( u32 ),
			meshX1, meshY1, meshZ1, meshX2, meshY2, meshZ2, skinSlotIndex );
		this->meshes.add( meshID );

		offsetCurrent = offsetNext;
		meshIndex++;
	}

	return true;
}


bool Model::load_from_cache()
{
	CacheModel cache;
	if( !Assets::cache.fetch( this->cacheKey, cache ) ) { return false; }

	for( u32 meshIndex = 0; meshIndex < cache.meshCount; meshIndex++ )
	{
		const MeshID meshID = Assets::meshes.register_new_from_cache( cache.meshCacheKeys[meshIndex] );
		ErrorIf( meshID == MESHID_NULL, "Failed to load Model '%s' Mesh %u from cache!",
			this->name.cstr(), meshIndex );
		this->meshes.add( meshID );
	}

	this->formatVertex = cache.formatVertex;
	this->x1 = cache.x1;
	this->y1 = cache.y1;
	this->z1 = cache.z1;
	this->x2 = cache.x2;
	this->y2 = cache.y2;
	this->z2 = cache.z2;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////