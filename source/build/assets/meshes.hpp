#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/buffer.hpp>
#include <core/string.hpp>

#include <build/objloader.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Mesh
{
	MeshObj meshFile;
	usize vertexBufferOffset = 0;
	usize indexBufferOffset = 0;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	String filepath;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Meshes
{
	void make_new( const Mesh &material );
	void gather( const char *path, const bool recurse = true );
	void load( const char *path );
	void write();

	Mesh &operator[]( const u32 meshID ) { return meshes[meshID]; }

	List<Mesh> meshes;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////