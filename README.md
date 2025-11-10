## Note: This is an early fork of the engine for showcase purposes only! Manta is a personal learning project not meant for open use. That said, the source is available here to browse as you please!

# Engine Overview:

Manta is C++ game engine & build system currently under development designed to achieve the following:

- Provide a simple library for creating games quickly
- Provide abstraction layers for cross-platform compilation (Windows/MacOS/Linux) and graphics API backends (OpenGL, DirectX, Metal, Vulkan)
- Provide a custom build system that prioritizes fast compile times and build caching
- Employ build-time code generation for engine systems (boilerplate internals of object, asset, and render systems, etc.)
- Have an “IDE-like” experience facilitated by a Visual Studio Code plugin (build + compile + run, debugging, project/workspace navigation, IntelliSense)

# Showcase Projects

Globe Renderer (https://www.youtube.com/watch?v=0EJK37nCA-0)

<sub>* Source included in repo: projects/globe (boot -project=globe)"</sub>

[![Youtube Redirect](https://img.youtube.com/vi/0EJK37nCA-0/0.jpg)](https://www.youtube.com/watch?v=0EJK37nCA-0)



Vitality (https://www.youtube.com/watch?v=y05wb1y-OQc) (Work in progress indie game)

[![Youtube Redirect](https://img.youtube.com/vi/y05wb1y-OQc/0.jpg)](https://www.youtube.com/watch?v=y05wb1y-OQc)

# Setup:

### 1. Cloning Repository
```
git clone https://github.com/jaredx64/manta-engine
cd manta-engine
```

Note: Manta uses Ninja for efficient compilation & linking (https://ninja-build.org/). The executables are included in the repository (`manta-engine\.manta\ninja-build`). If Ninja fails, it may need manual setup: https://github.com/ninja-build/ninja/releases

### 2. Building + Running Project:

Building, compiling, and running is driven by invoking the `manta-engine\boot` script.
Below are steps for each OS:

<details>
<summary><span>Windows</span></summary>

```
boot.bat -project=globe
```
Note: by default, boot.bat builds with MSVC (C++20). You can also pass `-toolchain=llvm` or `-toolchain=gnu` if you have LLVM or GNU installed.

If using MSVC, you may need to edit the generated `manta-engine\boot-vcvars64.txt` to point to the appropriate location on your machine. Loading vcvars64.bat is (annoyingly) required for running MSVC in a terminal.

</details>

<details>
<summary><span>Linux / MacOS</span></summary>

```
chmod +x boot.sh
./boot.sh -project=globe
```
Note: by default, boot.sh builds with LLVM. You can also pass `-toolchain=gnu`. You need either LLVM or GNU installed on the machine (C++20).

</details>

### 3. (Optional) Manta VS Code Plugin
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

VSCManta is essentially a wrapper around explicit calls to the `manta-engine\boot` script. It scans the engine & project repository and generates build commands. It can also be bound to hotkeys in VS Code.

Windows Only: If using the RenderDoc button (`boot -run=2`), RenderDoc must be added to system PATH environment variables (e.g. `C:\Program Files\RenderDoc`)

# Engine Repository Structure:

1. `manta-engine\.manta\`
	- Contains a "template" project and "vscmanta" VS Code plugin
2. `manta-engine\source\assets\`
	- Built-in engine assets (default font, sprite, shader, etc.)
3. `manta-engine\source\boot\` (boot.exe)
	- Source files for the boostrap executable
4. `manta-engine\source\build\` (build.exe)
	- Source files for the build tool library
5. `manta-engine\source\manta\` (project.exe - engine)
	- Source files for the manta runtime library
6. `manta-engine\source\core\`
	- Source files shared between /build, /manta, and projects/*project*
7. `manta-engine\source\vendor\*`
	- Headers for external libraries and wrappers around C headers
	- As a compile time optimization (for development builds), Manta supports a "USE_OFFICIAL_HEADERS" macro which (when false) uses trimmed C headers
8. `manta-engine\projects\...` (project.exe - application)
	- Location for project repositories (either copy template project here, or use VSCManta plugin to create a new project)

In general terms, `source\boot`, `source\build`, and `source\manta` are separate programs that handle each stage of the pipeline: boot -> build -> runtime.


# Per-Project Repository Structure:

1. `manta-engine\projects\<project>\assets\`
	- Assets for the project (*.font, *.sprite, *.shader, etc.)
	- Asset files are typically JSON or raw data (i.e., a sprite is a .sprite (json) and a .png)
2. `manta-engine\projects\<project>\build\`
	- Project-specific sources for the build executable (implements main)
	- See: *project*/build/build.cpp and *project*/build/build.hpp
		- For project-specific build logic, create a class that extends 'BuilderCore' from "#include <build/build.hpp>" (engine build library) and override desired stages
3. `manta-engine\projects\<project>\runtime\`
	- Project-specific sources for the runtime executable (implements main)
	- All game/project logic is programmed here. Manta library functions/systems can be accessed through "#include <manta/...>"
	- Entry point typically fills out a "ProjectCallbacks' struct containing init(), free(), and update() functions for the project and calls Engine::main()
		- See: .manta/template/runtime/main.cpp
		- While this is not strictly required, it allows the engine to init, update, and free itself automatically
4. `manta-engine\projects\<project>\configs.json`
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

Manta does not use precompiled headers or static/dynamic linkage for the engine "library." Rather, sources are compiled directly as a part of the project executable.

The reasons for this are:

- Caching of builds is already done in the boot.exe and build.exe stages (with C++ compilation caching provided by ninja)
- The build tool generates C++ boilerplate for various runtime systems (assets, objects, graphics backend) which would cause frequent rebuilds of precompiled headers
- The engine library code will likely be updated continously alongside project code (until the engine matures)
- Compile times are already quite fast due to restrictions on C++ STL includes, "unofficial" C headers for development builds (USE_OFFICIAL_HEADERS = 0), and general project structure
- It is easier to reason about, understand, and maintain a system when it is simpler

This may change in later versions of the engine, but for now it is structured this way.

Anecdotally (on my machine) I can do a clean build + compile (no cache) of boot.exe, build.exe, and the game.exe in roughly 500ms (~100 translation units) with both MSVC and Clang++ on the "release" configuration.


# Graphics System

### I. Shader Language

To seamlessly support multiple graphics backends without requiring duplicated shaders, Manta implements its own "common" shader language suffixed `.shader`.

Shader files are translated at build time into either GLSL, HLSL, or Metal depending on the target graphics API. Boilerplate C++ code is also generated by the build tool from the shader files to assist with shader stage input layouts and resource bindings.

The generated shader code (hlsl/glsl/metal) is written to `projects\<project>\output\generated\shaders\`, along with the associated graphics system C++ boilerplate in `projects\<project>\output\generated\gfx.generated.hpp`.

In an editor, basic syntax highlighting and IntelliSense is possible by optionally including `#include <shader_api.hpp>` in the shader file. That header is compatible with a typical C++ language server and provides a reference to the features/keywords of the shader language.

**Example `.shader` code (Simple Phong):**
```c++
vertex_input VertexInput
{
	float3 position packed_as( FLOAT32 );
	float3 normal packed_as( FLOAT32 );
	float2 uv packed_as( UNORM16 );
};

vertex_output VertexOutput
{
	float4 positionNDC position_out;
	float4 positionWorld;
	float4 normal;
	float2 uv;
};

uniform_buffer( 0 ) UniformsPipeline
{
	float4x4 matrixModel;
	float4x4 matrixMVP;
};

void vertex_main( VertexInput In, VertexOutput Out, UniformsPipeline Pipeline )
{
	Out.positionNDC = mul( Pipeline.matrixMVP, float4( In.position, 1.0 ) );
	Out.positionWorld = mul( Pipeline.matrixModel, float4( In.position, 1.0 ) );
	Out.normal = mul( Pipeline.matrixModel, float4( In.normal, 0.0 ) );
	Out.uv = In.uv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

fragment_input FragmentInput
{
	float4 positionNDC position_in;
	float4 positionWorld;
	float4 normal;
	float2 uv;
};

fragment_output FragmentOutput
{
	float4 color target( 0, COLOR );
};

uniform_buffer( 1 ) UniformsPhong
{
	float3 pointEye;
	float3 colorLight;
	float3 colorAmbient;
	float3 directionLight;
};

texture2D( 0, float4 ) texture;

float3 calculate_phong( float3 position, float3 normal, float2 uv, UniformsPhong Phong )
{
	float3 N = normalize( normal );
	float3 L = normalize( Phong.directionLight );
	float3 V = normalize( Phong.pointEye - position );
	float3 R = reflect( -L, N );

	float3 albedo = texture_sample_2d( texture, uv ).rgb;
	float3 ambient = Phong.colorAmbient * albedo;

	float d = max( dot( N, L ), 0.0 );
	float3 diffuse = Phong.colorLight * albedo * d;

	float s = 0.0;
	if( d > 0.0 )
	{
		s = pow( max( dot( R, V ), 0.0 ), 32.0 );
	}
	float3 specular = Phong.colorLight * s;

	return ambient + diffuse + specular;
}

void fragment_main( FragmentInput In, FragmentOutput Out, UniformsPhong Phong )
{
	float3 phong = calculate_phong( In.positionWorld.xyz, In.normal.xyz, In.uv, Phong );
	Out.color = float4( phong, 1.0 );
}
```

**C++ Render Command (Game Code, GPU):**
```c++
GfxRenderCommand drawPhong;
drawPhong.shader( Shader::sh_phong );
drawPhong.raster_cull_mode( GfxCullMode_BACK );
Gfx::render_command_execute( drawPhong, [=]()
	{
		GfxUniformBuffer::UniformsPipeline.matrixModel = matrixModel;
		GfxUniformBuffer::UniformsPipeline.matrixMVP = matrixMVP;
		GfxUniformBuffer::UniformsPipeline.upload();

		GfxUniformBuffer::UniformsPhong.pointEye = eye;
		GfxUniformBuffer::UniformsPhong.colorLight = float_v3 { 1.0f, 0.5f, 0.5f };
		GfxUniformBuffer::UniformsPhong.colorAmbient = float_v3 { 0.1f, 0.1f, 0.1f };
		GfxUniformBuffer::UniformsPhong.directionLight = float_v3 { 1.0f, 0.0f, 0.0f };
		GfxUniformBuffer::UniformsPhong.upload();

		Gfx::bind_texture( 0, Texture::TEXTURE_DEFAULT );
		Gfx::draw_vertex_buffer( vertexBufferSphere );
	} );
```

**Generated HLSL/GLSL/Metal Shaders:**
<details>
<summary><span style="color: yellow;">HLSL</span></summary>

```c++
struct Global
{
	uint vertexID;
	uint instanceID;
};

struct t_VertexInput
{
	float3 v_position : V0;
	float3 v_normal : V1;
	float2 v_uv : V2;
};

struct t_VertexOutput
{
	float4 v_positionNDC : SV_POSITION;
	float4 v_positionWorld : V0;
	float4 v_normal : V1;
	float2 v_uv : V2;
};

cbuffer t_UniformsPipeline : register( b0 )
{
	float4x4 t_UniformsPipeline_v_matrixModel;
	float4x4 t_UniformsPipeline_v_matrixMVP;
};

void vs_main( in t_VertexInput v_In, out t_VertexOutput v_Out,
	uint vertexID : SV_VertexID,
	uint instanceID : SV_InstanceID )
{
	Global global = { vertexID, instanceID };
	{
		v_Out.v_positionNDC = mul( t_UniformsPipeline_v_matrixMVP, float4( v_In.v_position, 1.0 ) );
		v_Out.v_positionWorld = mul( t_UniformsPipeline_v_matrixModel, float4( v_In.v_position, 1.0 ) );
		v_Out.v_normal = mul( t_UniformsPipeline_v_matrixModel, float4( v_In.v_normal, 0.0 ) );
		v_Out.v_uv = v_In.v_uv;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Global
{
	uint primitiveID;
	uint sampleID;
	bool isFrontFace;
};

struct t_FragmentInput
{
	float4 v_positionNDC : SV_POSITION;
	float4 v_positionWorld : V0;
	float4 v_normal : V1;
	float2 v_uv : V2;
};

struct t_FragmentOutput
{
	float4 v_color : SV_TARGET0;
};

cbuffer t_UniformsPhong : register( b1 )
{
	float3 t_UniformsPhong_v_pointEye;
	float3 t_UniformsPhong_v_colorLight;
	float3 t_UniformsPhong_v_colorAmbient;
	float3 t_UniformsPhong_v_directionLight;
};

Texture2D<float4> v_texture : register( t0 );
SamplerState GlobalSampler : register( s0 );

float3 f_calculate_phong( Global global, float3 v_position, float3 v_normal, float2 v_uv )
{
	float3 v_N = normalize( v_normal );
	float3 v_L = normalize( t_UniformsPhong_v_directionLight );
	float3 v_V = normalize( t_UniformsPhong_v_pointEye - v_position );
	float3 v_R = reflect( -v_L, v_N );
	float3 v_albedo = v_texture.Sample( GlobalSampler, v_uv ).rgb;
	float3 v_ambient = t_UniformsPhong_v_colorAmbient * v_albedo;
	float v_d = max( dot( v_N, v_L ), 0.0 );
	float3 v_diffuse = t_UniformsPhong_v_colorLight * v_albedo * v_d;
	float v_s = 0.0;

	if( v_d > 0.0 )
	{
		v_s = pow( max( dot( v_R, v_V ), 0.0 ), 32.0 );
	}

	float3 v_specular = t_UniformsPhong_v_colorLight * v_s;
	return v_ambient + v_diffuse + v_specular;
}

void ps_main( in t_FragmentInput v_In, out t_FragmentOutput v_Out,
	uint primitiveID : SV_PrimitiveID,
	uint sampleID : SV_SampleIndex,
	bool isFrontFace : SV_IsFrontFace )
{
	Global global = { primitiveID, sampleID, isFrontFace };
	{
		float3 v_phong = f_calculate_phong( global, v_In.v_positionWorld.xyz, v_In.v_normal.xyz, v_In.v_uv );
		v_Out.v_color = float4( v_phong, 1.0 );
	}
}
```
</details>

<details>
<summary><span style="color: yellow;">GLSL</span></summary>

```c++
#version 410 core

layout( location = 0 ) in vec3 t_VertexInput_v_position;
layout( location = 1 ) in vec3 t_VertexInput_v_normal;
layout( location = 2 ) in vec2 t_VertexInput_v_uv;

#define varying_v_positionNDC gl_Position
layout( location = 1 ) out vec4 varying_v_positionWorld;
layout( location = 2 ) out vec4 varying_v_normal;
layout( location = 3 ) out vec2 varying_v_uv;

layout( std140 ) uniform t_UniformsPipeline
{
	mat4 t_UniformsPipeline_v_matrixModel;
	mat4 t_UniformsPipeline_v_matrixMVP;
};

void main()
{
	varying_v_positionNDC = ( ( t_UniformsPipeline_v_matrixMVP ) * ( vec4( t_VertexInput_v_position, 1.0 ) ) );
	varying_v_positionWorld = ( ( t_UniformsPipeline_v_matrixModel ) * ( vec4( t_VertexInput_v_position, 1.0 ) ) );
	varying_v_normal = ( ( t_UniformsPipeline_v_matrixModel ) * ( vec4( t_VertexInput_v_normal, 0.0 ) ) );
	varying_v_uv = t_VertexInput_v_uv;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 410 core

#define varying_v_positionNDC gl_FragCoord
layout( location = 1 ) in vec4 varying_v_positionWorld;
layout( location = 2 ) in vec4 varying_v_normal;
layout( location = 3 ) in vec2 varying_v_uv;

layout( location = 0 ) out vec4 t_FragmentOutput_v_color;

layout( std140 ) uniform t_UniformsPhong
{
	vec3 t_UniformsPhong_v_pointEye;
	vec3 t_UniformsPhong_v_colorLight;
	vec3 t_UniformsPhong_v_colorAmbient;
	vec3 t_UniformsPhong_v_directionLight;
};

uniform sampler2D u_texture0;

vec3 f_calculate_phong( vec3 v_position, vec3 v_normal, vec2 v_uv )
{
	vec3 v_N = normalize( v_normal );
	vec3 v_L = normalize( t_UniformsPhong_v_directionLight );
	vec3 v_V = normalize( t_UniformsPhong_v_pointEye - v_position );
	vec3 v_R = reflect( -v_L, v_N );
	vec3 v_albedo = texture( u_texture0, vec2( ( v_uv ).x, 1.0 - ( v_uv ).y ) ).rgb;
	vec3 v_ambient = t_UniformsPhong_v_colorAmbient * v_albedo;
	float v_d = max( dot( v_N, v_L ), 0.0 );
	vec3 v_diffuse = t_UniformsPhong_v_colorLight * v_albedo * v_d;
	float v_s = 0.0;

	if( v_d > 0.0 )
	{
		v_s = pow( max( dot( v_R, v_V ), 0.0 ), 32.0 );
	}

	vec3 v_specular = t_UniformsPhong_v_colorLight * v_s;
	return v_ambient + v_diffuse + v_specular;
}

void main()
{
	vec3 v_phong = f_calculate_phong( varying_v_positionWorld.xyz, varying_v_normal.xyz, varying_v_uv );
	t_FragmentOutput_v_color = vec4( v_phong, 0.0, 1.0 );
}
```
</details>

<details>
<summary><span style="color: yellow;">Metal</span></summary>

```c++
#include <metal_stdlib>
using namespace metal;

struct Global
{
	uint vertexID;
	uint instanceID;
};

struct t_VertexInput
{
	float3 v_position [[attribute(0)]];
	float3 v_normal [[attribute(1)]];
	float2 v_uv [[attribute(2)]];
};

struct t_VertexOutput
{
	float4 v_positionNDC [[position]];
	float4 v_positionWorld;
	float4 v_normal;
	float2 v_uv;
};

struct t_UniformsPipeline
{
	float4x4 v_matrixModel;
	float4x4 v_matrixMVP;
};

vertex t_VertexOutput vs_main( t_VertexInput v_In [[stage_in]],
	uint vertexID [[vertex_id]],
	uint instanceID [[instance_id]],
	constant t_UniformsPipeline &v_Pipeline [[buffer(2)]] )
{
	Global global = { vertexID, instanceID };
	t_VertexOutput v_Out;
	{
		v_Out.v_positionNDC = ( ( v_Pipeline.v_matrixMVP ) * ( float4( v_In.v_position, 1.0 ) ) );
		v_Out.v_positionWorld = ( ( v_Pipeline.v_matrixModel ) * ( float4( v_In.v_position, 1.0 ) ) );
		v_Out.v_normal = ( ( v_Pipeline.v_matrixModel ) * ( float4( v_In.v_normal, 0.0 ) ) );
		v_Out.v_uv = v_In.v_uv;
	}
	return v_Out;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <metal_stdlib>
using namespace metal;

struct Global
{
	uint primitiveID;
	uint sampleID;
	bool isFrontFace;
	sampler GlobalSampler;
	texture2d<float> v_texture;
};

struct t_FragmentInput
{
	float4 v_positionNDC [[position]];
	float4 v_positionWorld;
	float4 v_normal;
	float2 v_uv;
};

struct t_FragmentOutput
{
	float4 v_color [[color(0)]];
};

struct t_UniformsPhong
{
	float3 v_pointEye;
	float3 v_colorLight;
	float3 v_colorAmbient;
	float3 v_directionLight;
};

float3 f_calculate_phong( Global global, float3 v_position, float3 v_normal, float2 v_uv, t_UniformsPhong v_Phong )
{
	float3 v_N = normalize( v_normal );
	float3 v_L = normalize( v_Phong.v_directionLight );
	float3 v_V = normalize( v_Phong.v_pointEye - v_position );
	float3 v_R = reflect( -v_L, v_N );
	float3 v_albedo = global.v_texture.sample( global.GlobalSampler, v_uv ).rgb;
	float3 v_ambient = v_Phong.v_colorAmbient * v_albedo;
	float v_d = max( dot( v_N, v_L ), 0.0 );
	float3 v_diffuse = v_Phong.v_colorLight * v_albedo * v_d;
	float v_s = 0.0;

	if( v_d > 0.0 )
	{
		v_s = pow( max( dot( v_R, v_V ), 0.0 ), 32.0 );
	}

	float3 v_specular = v_Phong.v_colorLight * v_s;
	return v_ambient + v_diffuse + v_specular;
}

fragment t_FragmentOutput fs_main( t_FragmentInput v_In [[stage_in]],
	uint primitiveID [[primitive_id]],
	uint sampleID [[sample_id]],
	bool isFrontFace [[front_facing]],
	sampler GlobalSampler [[sampler(0)]],
	texture2d<float> v_texture [[texture(0)]],
	constant t_UniformsPhong &v_Phong [[buffer(3)]] )
{
	Global global = { primitiveID, sampleID, isFrontFace, GlobalSampler, v_texture };
	t_FragmentOutput v_Out;
	{
		float3 v_phong = f_calculate_phong( global, v_In.v_positionWorld.xyz, v_In.v_normal.xyz, v_In.v_uv, v_Phong );
		v_Out.v_color = float4( v_phong, 1.0 );
	}
	return v_Out;
}
```
</details>

### 2. Vertex Formats

As seen in the code above, shaders can explicitly specify a `vertex_input`.

Defining new vertex formats for use within the engine (C++) is as simple as specifying a `vertex_input` in _any_ `.shader` file within the project. These vertex definitions are parsed by the build tool and used to generate the appropriate boilerplate needed for various engine C++ code. Vertex formats can be reused between shaders without needing to redefine them, making it possible to have a single `.shader` file solely for specifying the vertex formats of a project.

**Example `.shader` vertex format definition:**
```c++
// sh_example.shader
vertex_input VertexFormat
{
	float3 position packed_as( FLOAT32 );
	float2 uv packed_as( UNORM16 );
	float4 color packed_as( UNORM8 );
};
```

Which gets parsed and generated to the following C++ struct for easy CPU access:
```c++
// output/generated/gfx.generated.hpp:
namespace GfxVertex
{
	struct VertexFormat
	{
		float_v3 position;
		u16_v2 uv;
		u8_v4 color;
	};
}
```

And can be used anywhere in project C++ code:

```c++
// Init
GfxVertexBuffer<GfxVertex::VertexFormat> vertexBuffer;
vertexBuffer.init( 4, GfxWriteMode_ONCE );

// Write
vertexBuffer.write_begin();
	vertexBuffer.write( GfxVertex::VertexFormat { ... } );
	vertexBuffer.write( GfxVertex::VertexFormat { ... } );
	vertexBuffer.write( GfxVertex::VertexFormat { ... } );
	vertexBuffer.write( GfxVertex::VertexFormat { ... } );
vertexBuffer.write_end();

// Draw
GfxRenderCommand cmd;
cmd.shader( Shader::sh_example );
Gfx::render_command_execute( cmd, GfxWorkCapture
	{
		Gfx::draw_vertex_buffer( vertexBuffer, GfxPrimitiveType_TriangleStrip );
	} );
```

### 3. Buffers / Uniforms

Similar to vertex formats, buffers (uniform buffers, constant buffers, structured buffers, etc.) are definied directly in shader code.

The syntax within a `.shader` is:

```c++
// .shader file
uniform_buffer( 1 ) UniformsExample
{
	float_v4 color;
    float time;
};
```

Which is parsed and generated into a CPU accessible structure:
```c++
// output/generated/gfx.generated.hpp
namespace CoreGfxUniformBuffer
{
	// ...
	struct alignas( 16 ) UniformsExample_t
	{
		float_v4 color; // offset: 0
		float time; // offset: 16

		void zero();
		void upload() const;
		bool operator==( const UniformsExample_t &other ) const;
		bool operator!=( const UniformsExample_t &other ) const;
	};
	// ...
}

// output/generated/gfx.generated.cpp
namespace GfxUniformBuffer
{
	// ...
	CoreGfxUniformBuffer::UniformsExample_t UniformsExample; // Globally Accessible Hook
	// ...
}
```

And can be used anywhere in project code like:
```c++
GfxUniformBuffer::UniformsExample.color = float_v4 { 1.0, 0.0, 0.0, 1.0 };
GfxUniformBuffer::UniformsExample.time = globalTime;
GfxUniformBuffer::UniformsExample.upload();
```


# Object System

### I. Object Definitions
Manta features a custom object system utilizing its own C++ "scripting language" object definition files, suffixed *.object*

The goal is to make the authoring and maintenance of object code as straightforward as possible with minimal compromise on runtime performance.

In short, *.object* files contain psuedo-C++ code that describe an "object" class (data members, functions, events, etc.). The *.object* files are parsed by the build tool and translated into engine-ready generated C++ header and source implementations.

To aid with IntelliSense, *.object* files can optionally `#include <object_api.hpp>`

`object_api.hpp` provides a reference to the available keywords of the "scripting" language and allows C++ IntelliSense to work with *.object* code without the need for a custom language server. You can view the available keywords in that file.

Objects use a single-inheritence model. Objects can inherit/extend other objects by specifying the `PARENT( name )` keyword.

This meets most needs for games that I create. Additionally, the data structure where object data is managed ensures sufficient locality for most gameplay needs: per-instance object data is allocated contiguously in memory within designated memory blocks for each object type.

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
	friend struct CoreObjects::OBJECT_ENCODER<Object::obj_rabbit>;
	enum RabbitState
	{
		RabbitState_Stand,
		RabbitState_Walk,
	};
	RabbitState state = RabbitState_Stand;
	void process_death();
};
__INTERNAL_OBJECT_SYSTEM_END

template <typename... Args> struct CoreObjects::ObjectConstructor<Object::obj_rabbit, Args...>
{
	static void construct( Args... args )
	{
		new ( CoreObjects::CONSTRUCTOR_BUFFER +
		      CoreObjects::CONSTRUCTOR_BUFFER_OFFSET[Object::obj_rabbit] ) CoreObjects::obj_rabbit_t( args... );
	}
};

template <> struct ObjectHandle<Object::obj_rabbit>
{
	CoreObjects::obj_rabbit_t *data = nullptr;
	CoreObjects::obj_rabbit_t *operator->() const { Assert( data != nullptr ); return data; }
	explicit operator bool() const { return data != nullptr; }
	ObjectHandle( void *object ) { data = reinterpret_cast<CoreObjects::obj_rabbit_t *>( object ); }
};

// ... file continued
```
</details>

<details>
<summary><span style="color: yellow;">objects.generated.cpp</span></summary>

```c++
// ...

template <> struct CoreObjects::OBJECT_ENCODER<Object::obj_rabbit>
{
	OBJECT_ENCODER( void *data ) : object{ *reinterpret_cast<obj_rabbit_t *>( data ) } { } obj_rabbit_t &object;
};

CoreObjects::obj_rabbit_t::obj_rabbit_t( float spawnX, float spawnY )
{
	x = spawnX;
	y = spawnY;
}

void CoreObjects::obj_rabbit_t::EVENT_UPDATE( const Delta delta )
{
	CoreObjects::obj_animal_t::EVENT_UPDATE( delta );
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

void CoreObjects::obj_rabbit_t::EVENT_RENDER( const Delta delta )
{
	CoreObjects::obj_animal_t::EVENT_RENDER( delta );
	draw_sprite( spr_rabbit, x, y );
}

void CoreObjects::obj_rabbit_t::process_death()
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

1. `ObjectContext`
   - A context for object creation, destruction, and handling. Implements the underlying datastructures and manages object lifetimes.
2. `ObjectInstance`
   - A unique identifier for every instantiated object in an ObjectContext
3. `ObjectHandle<Object>`
   - A handle to a specific object instance data (i.e., used like a pointer)

In the generated code above, you may notice that the object class names are suffixed with \***_t** and wrapped within an internal namespace.

The reason for this is "raw" object types are not exposed directly in game code. Rather, object types are represented as __Object__ enums. This allows them to be stored in variables (integers) and manipulated at runtime.

The table looks something like:
```c++
// objects.generated.hpp

enum Object
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
const Object type = spawnRabbit ? Object::obj_rabbit : Object::obj_chicken;
context.create( type ); // Create an obj_rabbit or obj_chicken
```

Here are some more examples:
```c++
#include <manta/objects.hpp> // Object system library

// ObjectContext Initialization
ObjectContext context;
context.init(); // There can be multiple contexts in a program (ui, scene, etc.)

// Object Creation
ObjectInstance rabbit1 = context.create<Object::obj_rabbit>( 64.0f, 64.0f ); // Create with constructor parameters
ObjectInstance rabbit2 = context.create( Object::obj_rabbit ); // Create with default parameters
ObjectInstance animal1 = context.create( choose( Object::obj_chicken, Object::obj_rabbit ) ); // Creation of chicken OR rabbit (runtime randomness)

// ObjectHandle Retrieval
ObjectHandle<Object::obj_rabbit> rabbitHandle = rabbit1.handle<Object::obj_rabbit>( context ); // or: auto rabbitHandle = ...

// ObjectHandle Validation (handle can be null if the Object does not exist)
if( rabbitHandle )
{
	rabbitHandle->x = 0.0f;
	rabbitHandle->y = 0.0f;
}

// Looping over all obj_rabbit instances
for( auto rabbitHandle : context.iterator<Object::obj_rabbit>() )
{
	rabbitHandle->x = 0.0f;
	rabbitHandle->y = 0.0f;
}

// Looping over all obj_animal instances (includes obj_rabbits & obj_chicken)
for( auto animalHandle : context.iterator_polymorphic<Object::obj_animal>( true ) )
{
	animalHandle->x = 0.0f;
	animalHandle->y = 0.0f;
}

// Object Destruction
context.destroy( rabbit1 );

// ObjectContext freeing
context.free();
```


# Future Roadmap

Since engine development is primarily driven by project needs, there is currently no precise roadmap. However, the following are areas I hope to complete over the next 6-12 months:

### Graphics
- Upgrade to OpenGL 4.5+ (currently limited to 4.1 by macOS OpenGL deprecation)
- Implement Vulkan
- Switch from D3D11 to D3D12

### Networking
- Finish the networking module (UDP/TCP implementations across Windows/macOS/Linux)

### Scripting
- Integrate simple scripting support to the engine (LUA?)

### Platforms
- iOS
- Android
- Web (via WASM & WebGPU)
