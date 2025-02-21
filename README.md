## Note: This is an early fork of the engine for showcase purposes only!

# Engine Overview:

Manta is C++ game engine & build system currently under development designed to achieve the following:

- Provide a simple library for creating games quickly
- Provide abstraction layers for cross-platform compilation (Windows/MacOS/Linux) and graphics API backends (OpenGL, DirectX, Vulkan, Metal)
- Provide a custom build system that prioritizes fast compile times and build caching
- Employ build-time code generation for engine systems (boilerplate internals of object, asset, and render systems, etc.)
- Have an “IDE-like” experience facilitated by a Visual Studio Code plugin (build + compile + run, debugging, project/workspace navigation, IntelliSense)

Manta is a personal project not meant for open use. That said, the source is available here to browse as you please.

# Showcase Projects

Voxel Renderer (https://www.youtube.com/watch?v=Q8Q8c5H9kaU)

[![Youtube Redirect](https://img.youtube.com/vi/Q8Q8c5H9kaU/0.jpg)](https://www.youtube.com/watch?v=Q8Q8c5H9kaU)

Vitality (https://www.youtube.com/watch?v=_YdTRmEN9xs)

[![Youtube Redirect](https://img.youtube.com/vi/_YdTRmEN9xs/0.jpg)](https://www.youtube.com/watch?v=_YdTRmEN9xs)

# Setup:

### 1. Cloning Repository
```
git clone https://github.com/jaredx64/manta-engine
cd manta-engine
```

### 2. Ninja Build
You may need to install Ninja on your machine (and get the ninja-build extension in VS Code)

Link: https://github.com/ninja-build/ninja/releases

On Windows, you must add the path to ninja.exe to your system PATH environment variables.

### 3. Building + Running Project:

<details>
<summary><span>Windows</span></summary>

```
boot.bat -project=asteroids
```
Note: by default, boot.bat builds with MSVC (C++20). You can also pass `-toolchain=llvm` or `-toolchain=gnu` if you have LLVM or GNU installed.

If using MSVC, you may need to edit the generated `manta-engine\boot-vcvars64.txt` to point to the appropriate location on your machine. Loading vcvars64.bat is (annoyingly) required for running MSVC in a terminal.

</details>

<details>
<summary><span>Linux / MacOS</span></summary>

```
chmod +x boot.sh
./boot.sh -project=asteroids
```
Note: by default, boot.sh builds with LLVM. You can also pass `-toolchain=gnu`. You need either LLVM or GNU installed on the machine (C++20).

</details>

### 4. Manta VS Code Plugin (Optional)
```
VS Code -> Extensions -> Install from VSIX -> manta-engine\.manta\vscmanta\output\vscmanta.vsix
```
<details>
<summary><span>Helpful keybindings (add to keybindings.json)</span></summary>

```
{ "key": "f5", "when": "resourceExists('.manta')", "command": "vscmanta.buildAndRun" },
{ "key": "f6", "when": "resourceExists('.manta')", "command": "vscmanta.debugRuntime" },
{ "key": "f7", "when": "resourceExists('.manta')", "command": "vscmanta.renderdoc" },
{ "key": "f8", "when": "resourceExists('.manta')", "command": "vscmanta.build" },
{ "key": "f9", "when": "resourceExists('.manta')", "command": "vscmanta.insertCommentBreak" },
{ "key": "ctrl+shift+c", "when": "resourceExists('.manta')", "command": "vscmanta.cleanProject" },
```

</details><br>

Note: Source for plugin can be found at `manta-engine\.manta\vscmanta\extension.js`

VSCManta is essentially a wrapper around explicit calls to boot.sh/bat. It scans the engine/project repository and generates build commands. It can also be bound to hotkeys in VS Code.




If using the RenderDoc button, the path to the RenderDoc executable must be added to system PATH environment variables (Windows Only).

# Repository Structure:

1. .manta/
	- Contains a "template" project and "vscmanta" VS Code plugin
2. source/assets/
	- Built-in engine assets (default font, sprite, shader, etc.)
3. source/boot/
	- (`boot.exe`) Source files for the boostrap executable
4. source/build/
	- (`build.exe`) Source files for the manta build library
5. source/manta/
	- (`<project>.exe`) Source files for the manta runtime library
5. source/core/
	- Source files shared between /build, /manta, and projects/*project*
6. source/vendor/
	- Headers for external libraries and wrappers around C headers
	- As a compile time optimization (for development builds), Manta supports a "USE_OFFICIAL_HEADERS" macro which (when false) uses trimmed C headers
7. projects/...
	- Location for project repositories (either copy template project here, or use VSCManta plugin to create a new project)

In general terms, `source/boot`, `source/build`, and `source/manta` are three separate programs with different requirements and goals.

As an example, boot.exe and build.exe are less strict about memory allocations. The container implementations available to those programs are compiled with RAII memory allocation/freeing and implicit copy/move semantics. This is fine (and very convenient) for a build tool running on a developer machine, but is less ideal for game code with performance and memory budget constraints.


# Project Structure:

1. projects/*project*/assets
	- Assets for the project (*.font, *.sprite, *.shader, etc.)
	- Asset files are typically JSON or raw data (i.e., a sprite is a .sprite (json) and a .png)
2. projects/*project*/build
	- Project-specific sources for the build executable (implements main)
	- See: *project*/build/build.cpp and *project*/build/build.hpp
		- For project-specific build logic, create a class that extends 'BuilderCore' from "#include <build/build.hpp>" (engine build library) and override desired stages
3. projects/*project*/runtime
	- Project-specific sources for the runtime executable (implements main)
	- All game/project logic is programmed here. Manta library functions/systems can be accessed through "#include <manta/...>"
	- Entry point typically fills out a "ProjectCallbacks' struct containing init(), free(), and update() functions for the project and calls Engine::main()
		- See: .manta/template/runtime/main.cpp
		- While this is not strictly required, it allows the engine to init, update, and free itself automatically
4. projects/*project*/configs.json
	- Customizable compiler and linker flags for the runtime executable build (-config=debug, -config=release, -config=yourcustomconfig, etc.)


# Build & Run Pipeline:

1. boot.bat/sh
	- Args:
		- -project=*project name* (REQUIRED)
		- -os=*windows/linux/macos*
		- -architecture=*x64/arm64*
		- -toolchain=*msvc/llvm/gnu*
		- -config=*config name*
		- -gfx=*opengl/d3d11/d3d12/vulkan/metal*
		- -clean=*0/1*
		- -run=*0/1*
	- Compiles boot.exe if it doesn't exist
	- Runs boot.exe

2. boot.exe
	- Generates projects/*project*/generated/pipeline.generated.hpp
		- This contains platform/config specific macro definitions used by the engine
	- Compiles build.exe from projects/*project*/build and source/build sources
	- Runs build.exe

3. build.exe
	- Processes project & engine assets (source/assets and projects/*project*/assets)
	- Generates C++ code (written to *project*/output/generated)
	- Writes *project*.bin binary file
	- Compiles *project*.exe
	- Runs *project*.exe (if -run=1)

**Example Build Command:**
> boot.bat -project=pacman -toolchain=llvm -gfx=d3d11 -config=release -run=1


# Additional Build Information:

Manta does not use precompiled headers or static/dynamic linkage for the engine "library." Rather, Manta sources are compiled directly as a part of the project executable.

The reasons for this are:

- Caching of builds and compiles is already done in boot.exe and build.exe (compiles cached with .ninja builds)
- The build tool employs code generation for various systems (assets, objects, graphics backend) which would force frequent rebuilds of engine PCH
- The engine library code will likely be updated continously alongside project code (until the engine matures)
- Compile times are already quite fast due to restrictions on C++ STL includes, "unofficial" C headers for development builds (USE_OFFICIAL_HEADERS 0), and general project structure
- There are rarely any times when I would want to update an engine .dll without also wanting to update the application executable itself
- It is easier to reason about, understand, and maintain a system when it is simpler

This may change in later versions of the engine, but for now it is structured this way.

Anecdotally (on my machine) I can do a clean build + compile (no cache) of boot.exe, build.exe, and the game.exe in roughly 500ms (~100 translation units) with both MSVC and Clang++ on the "release" configuration.


# Object System

### I. Object Definitions
Manta features a custom object system utilizing its own C++ "scripting language" object definition files, suffixed *.object*

The goal is to make the authoring and maintenance of object code as straightforward as possible with minimal compromise on runtime performance.

In short, *.object* files contain psuedo-C++ code that describe an "object" class (data members, functions, events, etc.). The *.object* files are parsed by the build tool and transformed into engine-ready generated C++ header and source implementations.

To aid with IntelliSense, *.object* files can optionally "#include <object_api.hpp>"

object_api.hpp provides both a reference to the available keywords of the "scripting" language, as well as enables C++ IntelliSense to work with *.object* code without the need for a custom language server. You can view the available keywords in that file.

Objects in this engine use a single-inheritence model. Objects can inherit/extend other objects by specifying the __PARENT( name )__ keyword.

This meets most needs for games that I create. Additionally, the data structure where object data is stored ensures sufficient locality for most gameplay needs: per-instance object data is allocated contiguously in memory within designated memory blocks for each object type.

If I ever desire more performance (i.e., must guarantee the _best_ cache locality possible) I would question whether utilizing the object system is the right solution for the problem at hand.

**Sample object definition code: (obj_rabbit.object)**
```c++
#include <object_api.hpp>

HEADER_INCLUDES // Headers to go in objects.generated.hpp
#include <manta/scene.hpp>

SOURCE_INCLUDES // Headers to go in objects.generated.cpp
#include <manta/draw.hpp>

// Object
OBJECT( obj_rabbit )
PARENT( obj_animal ) // obj_rabbit extends obj_animal
CATEGORY( OBJECT_CATEGORY_CREATURE )

// Data
PUBLIC float x = 0.0f;
PUBLIC float y = 0.0f;
PUBLIC float speed_x = 0.0f;
PUBLIC float speed_y = 0.0f;
PUBLIC float health = 100.0f;

PRIVATE enum RabbitState
{
	RabbitState_Stand,
	RabbitState_Walk,
	...
};

PRIVATE RabbitState state = RabbitState_Stand;

// Constructors
CONSTRUCTOR( float spawnX, float spawnY )
{
	x = spawnX;
	y = spawnY;
}

// Events & Functions
EVENT_UPDATE
{
	// State Machine
	switch( state )
	{
		// ...
	}

	// Health
	if( health <= 0.0f )
	{
		process_death();
	}
}

EVENT_RENDER
{
	draw_sprite( spr_rabbit, x, y );
}

PRIVATE void process_death()
{
	// Destroy ourselves
	sceneObjects.destroy( id ); // sceneObjects from scene.hpp
	return;
}
```

**Generated C++ engine code:**
<details>
<summary><span style="color: yellow;">objects.generated.hpp</span></summary>

```c++
// ...

__INTERNAL_OBJECT_SYSTEM_BEGIN
class obj_rabbit_t : public obj_animal_t
{
_PUBLIC:
	obj_rabbit_t() = default;
	obj_rabbit_t( float spawnX, float spawnY );
	float x = 0.0f;
	float y = 0.0f;
	float speed_x = 0.0f;
	float speed_y = 0.0f;
	float health = 100.0f;
	virtual void EVENT_UPDATE( const Delta delta );
	virtual void EVENT_RENDER( const Delta delta );
_PRIVATE:
	friend struct SysObjects::OBJECT_ENCODER<obj_rabbit>;
	enum RabbitState
	{
		RabbitState_Stand,
		RabbitState_Walk,
	};
	RabbitState state = RabbitState_Stand;
	void process_death();
};
__INTERNAL_OBJECT_SYSTEM_END

template <typename... Args> struct SysObjects::ObjectConstructor<obj_rabbit, Args...>
{
	static void construct( Args... args )
	{
		new ( SysObjects::CONSTRUCTOR_BUFFER +
		      SysObjects::CONSTRUCTOR_BUFFER_OFFSET[obj_rabbit] ) SysObjects::obj_rabbit_t( args... );
	}
};

template <> struct ObjectHandle<obj_rabbit>
{
	SysObjects::obj_rabbit_t *data = nullptr;
	SysObjects::obj_rabbit_t *operator->() const { Assert( data != nullptr ); return data; }
	explicit operator bool() const { return data != nullptr; }
	ObjectHandle( void *object ) { data = reinterpret_cast<SysObjects::obj_rabbit_t *>( object ); }
};

// ... file continued
```
</details>

<details>
<summary><span style="color: yellow;">objects.generated.cpp</span></summary>

```c++
// ...

template <> struct SysObjects::OBJECT_ENCODER<obj_rabbit>
{
	OBJECT_ENCODER( void *data ) : object{ *reinterpret_cast<obj_rabbit_t *>( data ) } { } obj_rabbit_t &object;
};

SysObjects::obj_rabbit_t::obj_rabbit_t( float spawnX, float spawnY )
{
	x = spawnX;
	y = spawnY;
}

void SysObjects::obj_rabbit_t::EVENT_UPDATE( const Delta delta )
{
	SysObjects::obj_animal_t::EVENT_UPDATE( delta );
	// State Machine
	switch( state )
	{
		// ...
	}

	// Health
	if( health <= 0.0f )
	{
		process_death();
	}
}

void SysObjects::obj_rabbit_t::EVENT_RENDER( const Delta delta )
{
	SysObjects::obj_animal_t::EVENT_RENDER( delta );
	draw_sprite( spr_rabbit, x, y );
}

void SysObjects::obj_rabbit_t::process_death()
{
	// Destroy ourselves
	sceneObjects.destroy( id ); // sceneObjects from scene.hpp
	return;
}

/// ... file continued
```
</details>

### II. Object System / Gameplay Programming

The object system works off three fundamental types:

1. **ObjectContext**
   - The data structure and interface for object creation, destruction, data lookup, and looping
2. **Object**
   - A unique "identifier" for every instantiated object in an ObjectContext
3. **ObjectHandle\<_ObjectType_>**
   - A handle to a specific object instance data

In the generated code above, you may notice that the object class names are suffixed with **_t** and wrapped within an internal namespace. The reason for this is "raw" object types are not to be used directly in game code. Rather, object types in game code are represented as __ObjectType__ enums. This allows them to be stored in variables and passed as runtime function parameters.

The table looks something like:
```c++
// objects.generated.hpp

enum ObjectType
{
	...
	obj_animal,  // internal::obj_animal_t class
	obj_rabbit,  // internal::obj_rabbit_t class
	obj_chicken, // internal::obj_chicken_t class
	...
};
```

This allows for game code like:
```c++
const ObjectType type = spawnRabbit ? obj_rabbit : obj_chicken;
context.create( type ); // Create an obj_rabbit or obj_chicken
```

Here are some more examples:
```c++
#include <manta/objects.hpp> // Object system library

// ObjectContext Initialization
ObjectContext context;
context.init(); // There can be multiple contexts in a program (ui, scene, etc.)

// Object Creation
Object rabbit1 = context.create<obj_rabbit>( 64.0f, 64.0f ); // Create with constructor parameters
Object rabbit2 = context.create( obj_rabbit ); // Create with default parameters
Object animal1 = context.create( choose( obj_chicken, obj_rabbit ) ); // Creation of chicken OR rabbit (runtime randomness)

// ObjectHandle Retrieval
ObjectHandle<obj_rabbit> rabbitHandle = rabbit1.handle<obj_rabbit>( context ); // or: auto rabbitHandle = ...

// ObjectHandle Validation (handle can be null if the Object does not exist)
if( rabbitHandle )
{
	rabbitHandle->x = 0.0f;
	rabbitHandle->y = 0.0f;
}

// Looping over all obj_rabbit instances
for( auto rabbitHandle : context.objects<obj_rabbit>() )
{
	rabbitHandle->x = 0.0f;
	rabbitHandle->y = 0.0f;
}

// Looping over all obj_animal instances (includes obj_rabbits & obj_chicken)
for( auto animalHandle : context.objects<obj_animal>( true ) )
{
	animalHandle->x = 0.0f;
	animalHandle->y = 0.0f;
}

// Object Destruction
context.destroy( rabbit1 );

// ObjectContext freeing
context.free();
```

# Graphics System

### I. Shader Language

To seamlessly support multiple graphics backends without requiring duplicated shaders, Manta implements its own "common" shader language suffixed *.shader*. The *.shader* files are compiled/translated at build time into either .glsl or .hlsl (and eventually .metal) depending on the target graphics API.

The shader language also provides a seamless way to globally define vertex buffer layouts (vertex format structs) and constrant/structured buffers, as well as enables preprocessor directives (such as #include) within shader files.

The generated output hlsl/glsl is placed in `projects/<project>/output/generated/shaders/`, along with associated graphics system boilerplate (for initializing vertex formats, buffers, etc.) in `projects/<project>/output/generated/gfx.generated.hpp`.

Syntax highlighting and IntelliSense is possible by including `#include <shader_api.hpp>` at the top of a .shader file. This makes the file compatible with a typical C++ language server. The language API can also be referenced by looking at the contents of that header file.

**Sample .shader code (simple shadow mapping):**
```c++
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vertex_input VertexInput
{
	float3 position semantic( POSITION ) format( FLOAT32 );
	float2 uv semantic( TEXCOORD ) format( UNORM16 );
	float4 color semantic( COLOR ) format( UNORM8 );
};

vertex_output VertexOutput
{
	float4 position semantic( POSITION );
	float2 uv semantic( TEXCOORD );
	float3 uvShadows semantic( TEXCOORD );
	float4 color semantic( COLOR );
};

fragment_input FragmentInput
{
	float4 position semantic( POSITION );
	float2 uv semantic( TEXCOORD );
	float3 uvShadows semantic( TEXCOORD );
	float4 color semantic( COLOR );
};

fragment_output FragmentOutput
{
	float4 color0 semantic( COLOR ) target( 0 );
};

cbuffer( 0 ) ShaderGlobals
{
	float4x4 matrixModel;
	float4x4 matrixView;
	float4x4 matrixPerspective;
	float4x4 matrixMVP;
};

cbuffer( 1 ) ShadowGlobals
{
	float4x4 matrixViewShadows;
	float4x4 matrixPerspectiveShadows;
};

texture2D( 0 ) textureColor;
texture2D( 1 ) textureShadowMap;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void vertex_main( BuiltinVertex In, VertexOutput Out, ShaderGlobals globals, ShadowGlobals shadows )
{
	// Out Position/UV/Color
	Out.position = mul( globals.matrixMVP, float4( In.position, 1.0 ) );
	Out.uv = In.uv;
	Out.color = In.color;

	// Shadow Map UVs
	float4x4 matrixMVPShadows = mul( shadows.matrixPerspectiveShadows,
		mul( shadows.matrixViewShadows, globals.matrixModel ) );
	float4 positionShadows = mul( matrixMVPShadows, float4( In.position, 1.0 ) );
	float3 shadowUVs = float3( positionShadows.xyz / positionShadows.w );
	Out.uvShadows = float3( shadowUVs.x * 0.5 + 0.5, 1.0 - ( shadowUVs.y * 0.5 + 0.5 ), shadowUVs.z );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EPSILON ( 0.00001 )

void fragment_main( FragmentInput In, FragmentOutput Out )
{
	// Sample Base Texture
	float4 baseColor = sample_texture2D( textureColor, In.uv );
	if( baseColor.a <= 0.01 ) { discard; }

	// Sample Shadow Map
	float depthShadowMap = sample_texture2D( textureShadowMap, In.uvShadows.xy ).r;
	float depthCamera = In.uvShadows.z - EPSILON;
	float shade = 1.0 - ( depthShadowMap < depthCamera ) * 0.65;

	// Output
	Out.color0 = float4( baseColor.rgb * In.color.rgb * shade, 1.0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
```

**Generated HLSL/GLSL Shaders:**
<details>
<summary><span style="color: yellow;">HLSL</span></summary>

```c++
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct t_BuiltinVertex
{
	float3 v_position : POSITION;
	float2 v_uv : TEXCOORD0;
	float4 v_color : COLOR0;
};

struct t_VertexOutput
{
	float4 v_position : SV_POSITION;
	float2 v_uv : TEXCOORD0;
	float3 v_uvShadows : TEXCOORD1;
	float4 v_color : COLOR0;
};

cbuffer t_ShaderGlobals : register( b0 )
{
	float4x4 t_ShaderGlobals_v_matrixModel;
	float4x4 t_ShaderGlobals_v_matrixView;
	float4x4 t_ShaderGlobals_v_matrixPerspective;
	float4x4 t_ShaderGlobals_v_matrixMVP;
};

cbuffer t_ShadowGlobals : register( b1 )
{
	float4x4 t_ShadowGlobals_v_matrixViewShadows;
	float4x4 t_ShadowGlobals_v_matrixPerspectiveShadows;
};

void vs_main( in t_BuiltinVertex v_In, out t_VertexOutput v_Out )
{
	v_Out.v_position = mul( t_ShaderGlobals_v_matrixMVP, float4( v_In.v_position, 1.000000 ) );
	v_Out.v_uv = v_In.v_uv;
	v_Out.v_color = v_In.v_color;
	float4x4 v_matrixMVPShadows = mul( t_ShadowGlobals_v_matrixPerspectiveShadows, mul( t_ShadowGlobals_v_matrixViewShadows, t_ShaderGlobals_v_matrixModel ) );
	float4 v_positionShadows = mul( v_matrixMVPShadows, float4( v_In.v_position, 1.000000 ) );
	float3 v_shadowUVs = float3( v_positionShadows.xyz / v_positionShadows.w );
	v_Out.v_uvShadows = float3( v_shadowUVs.x * 0.500000 + 0.500000,
		1.000000 - ( v_shadowUVs.y * 0.500000 + 0.500000 ), v_shadowUVs.z );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct t_FragmentInput
{
	float4 v_position : SV_POSITION;
	float2 v_uv : TEXCOORD0;
	float3 v_uvShadows : TEXCOORD1;
	float4 v_color : COLOR0;
};

struct t_FragmentOutput
{
	float4 v_color0 : SV_TARGET0;
};

Texture2D v_textureColor : register( t0 );
SamplerState GlobalSampler : register( s0 );

Texture2D v_textureShadowMap : register( t1 );

void ps_main( in t_FragmentInput v_In, out t_FragmentOutput v_Out )
{
	float4 v_baseColor = v_textureColor.Sample( GlobalSampler, v_In.v_uv );

	if( v_baseColor.a <= 0.010000 )
	{
		discard;
	}

	float v_depthShadowMap = v_textureShadowMap.Sample( GlobalSampler, v_In.v_uvShadows.xy ).r;
	float v_depthCamera = v_In.v_uvShadows.z - 0.000010;
	float v_shade = 1.000000 - ( v_depthShadowMap < v_depthCamera ) * 0.650000;
	v_Out.v_color0 = float4( v_baseColor.rgb * v_In.v_color.rgb * v_shade, 1.000000 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
```
</details>

<details>
<summary><span style="color: yellow;">GLSL</span></summary>

```c++
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 410 core

layout( location = 0 ) in vec3 t_BuiltinVertex_v_position;
layout( location = 1 ) in vec2 t_BuiltinVertex_v_uv;
layout( location = 2 ) in vec4 t_BuiltinVertex_v_color;

// layout( location = 0 ) out vec4 gl_Position;
layout( location = 1 ) out vec2 pipelineIntermediate_v_uv;
layout( location = 2 ) out vec3 pipelineIntermediate_v_uvShadows;
layout( location = 3 ) out vec4 pipelineIntermediate_v_color;

layout( std140 ) uniform t_ShaderGlobals
{
	mat4 t_ShaderGlobals_v_matrixModel;
	mat4 t_ShaderGlobals_v_matrixView;
	mat4 t_ShaderGlobals_v_matrixPerspective;
	mat4 t_ShaderGlobals_v_matrixMVP;
};

layout( std140 ) uniform t_ShadowGlobals
{
	mat4 t_ShadowGlobals_v_matrixViewShadows;
	mat4 t_ShadowGlobals_v_matrixPerspectiveShadows;
};

void main()
{
	gl_Position = ( ( t_ShaderGlobals_v_matrixMVP ) * ( vec4( t_BuiltinVertex_v_position, 1.000000 ) ) );
	pipelineIntermediate_v_uv = t_BuiltinVertex_v_uv;
	pipelineIntermediate_v_color = t_BuiltinVertex_v_color;
	mat4 v_matrixMVPShadows = ( ( t_ShadowGlobals_v_matrixPerspectiveShadows ) * ( ( ( t_ShadowGlobals_v_matrixViewShadows ) * ( t_ShaderGlobals_v_matrixModel ) ) ) );
	vec4 v_positionShadows = ( ( v_matrixMVPShadows ) * ( vec4( t_BuiltinVertex_v_position, 1.000000 ) ) );
	vec3 v_shadowUVs = vec3( v_positionShadows.xyz / v_positionShadows.w );
	pipelineIntermediate_v_uvShadows = vec3( v_shadowUVs.x * 0.500000 + 0.500000, 1.000000 - ( v_shadowUVs.y * 0.500000 + 0.500000 ), v_shadowUVs.z );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 410 core

// layout( location = 0 ) in vec4 gl_FragCoord;
layout( location = 1 ) in vec2 pipelineIntermediate_v_uv;
layout( location = 2 ) in vec3 pipelineIntermediate_v_uvShadows;
layout( location = 3 ) in vec4 pipelineIntermediate_v_color;

layout( location = 0 ) out vec4 t_FragmentOutput_v_color0;

uniform sampler2D u_texture0;

uniform sampler2D u_texture1;

void main()
{
	vec4 v_baseColor = texture( u_texture0, pipelineIntermediate_v_uv );

	if( v_baseColor.a <= 0.010000 )
	{
		discard;
	}

	float v_depthShadowMap = texture( u_texture1, pipelineIntermediate_v_uvShadows.xy ).r;
	float v_depthCamera = pipelineIntermediate_v_uvShadows.z - 0.000010;
	float v_shade = 1.000000 - ( v_depthShadowMap < v_depthCamera ) * 0.650000;
	t_FragmentOutput_v_color0 = vec4( v_baseColor.rgb * pipelineIntermediate_v_color.rgb * v_shade, 1.000000 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
```
</details>

### 2. Vertex Formats

As seen in the code above, shaders can explicitly specify a `vertex_input`.

Defining new vertex formats for use within the engine (C++) is as simple as specifying a `vertex_input` in _any_ .shader file within the project. These vertex definitions are parsed by the build tool and used to generate the appropriate boilerplate needed for various engine C++ code. Vertex formats can be reused between shaders without needing to redefine them, making it possible to have a single .shader file solely for specifying the vertex formats of a project.

In the example above, we see a custom format called `VertexFormat`:
```c++
// .shader file
vertex_input VertexFormat
{
	float3 position semantic( POSITION ) format( FLOAT32 );
	float2 uv semantic( TEXCOORD ) format( UNORM16 );
	float4 color semantic( COLOR ) format( UNORM8 );
};
```

Which gets parsed and generated to the following C++ struct for easy CPU access:
```c++
// output/generated/gfx.generated.hpp:
namespace GfxVertex
{
	struct VertexFormat
	{
		floatv3 position;
		u16v2 uv;
		u8v4 color;
	};
}
```

And can be used anywhere in project C++ code as follows:

```c++
// Create a vertex buffer with the custom format
GfxVertexBuffer<GfxVertex::VertexFormat> vertexBuffer;

// Vertex buffer containing 8 vertices with WRITE permission
vertexBuffer.init( 8, GfxCPUAccessMode_WRITE );

// Write 8 verticies to the buffer
vertexBuffer.write_begin();
for( int i = 0; i < 8; i++ )
{
	vertexBuffer.write( GfxVertex::VertexFormat { ... } );
}
vertexBuffer.write_end();

// Submit draw call
vertexBuffer.draw();
```

### 3. Constant Buffers / Uniforms

Similar to vertex formats, constant buffers (uniform buffers) are definied directly in shader code.

The syntax within a .shader is:

```c++
// .shader file
cbuffer( 1 ) ExampleUniformBuffer
{
	floatv4 color;
    float time;
};
```

Which is parsed and generated into a CPU accessible structure:
```c++
// output/generated/gfx.generated.hpp
namespace GfxCoreCBuffer
{
	// ...
	struct alignas( 16 ) ExampleUniformBuffer_t
	{
		floatv4 color;
		float time;

		void zero();
		void upload() const;

		bool operator==( const ExampleUniformBuffer_t &other )
		{
			return ( memory_compare( this, &other, sizeof( ExampleUniformBuffer_t ) ) == 0 );
		}

		bool operator!=( const ExampleUniformBuffer_t &other )
		{
			return ( memory_compare( this, &other, sizeof( ExampleUniformBuffer_t ) ) != 0 );
		}
	};
	// ...
}

// output/generated/gfx.generated.cpp
namespace GfxCoreCBuffer
{
	// ...
	void ExampleUniformBuffer_t::zero()
	{
		memory_set( this, 0, sizeof( ExampleUniformBuffer_t ) );
	}

	void ExampleUniformBuffer_t::upload() const
	{
		auto *&resource = GfxCore::gfxCBufferResources[2];
		GfxCore::rb_constant_buffer_write_begin( resource );
		GfxCore::rb_constant_buffer_write( resource, this );
		GfxCore::rb_constant_buffer_write_end( resource );
	}
	// ...
}

// output/generated/gfx.generated.cpp
namespace GfxCBuffer
{
	// ...
	GfxCoreCBuffer::ExampleUniformBuffer_t ExampleUniformBuffer; // Globaly accessible API
	// ...
}
```

And can be used anywhere in project code like:
```c++
GfxCBuffer::ExampleUniformBuffer.color = floatv4 { 1.0, 0.0, 0.0, 1.0 }; // red
GfxCBuffer::ExampleUniformBuffer.time = globalTime;
GfxCBuffer::ExampleUniformBuffer.upload();
```
