const vscode = require( "vscode" );
const path = require( "path" );
const fs = require( "fs" );
const child_process = require( "child_process" );

var WORKSPACE_IS_PROJECT = false;
var PLATFORM = process.platform;


// Paths
var PATH_WORKSPACE = vscode.workspace.workspaceFolders[0].uri.fsPath;
var PATH_ENGINE = "";
var PATH_BOOT_SCRIPT = "";
var PATH_ENGINE_SOURCES = "";
var PATH_ENGINE_CONFIGS = "";
var PATH_TEMPLATE_PROJECT = "";
var PATH_PROJECTS = "";
var PATH_PROJECT = "";
var PATH_PROJECT_BUILD_SOURCES = "";
var PATH_PROJECT_RUNTIME_SOURCES = "";
var PATH_PROJECT_CONFIGS = "";
var PATH_PROJECT_OUTPUT = "";
var PATH_PROJECT_OUTPUT_BOOT = "";
var PATH_PROJECT_OUTPUT_BUILD = "";
var PATH_PROJECT_OUTPUT_GENERATED = "";
var PATH_PROJECT_OUTPUT_RUNTIME = "";
var PATH_PROJECT_EXECUTABLE = "";
var PATH_DOT_VSCODE = "";
var PATH_DOT_MANTA = "";
var PATH_CACHE_ENGINE = "";
var PATH_CACHE_PROJECT = "";


// Build Configurations
var WIDGET_PROJECT = vscode.window.createStatusBarItem();
var WIDGET_PROJECT_OPTIONS = [ { label: "" } ];

var WIDGET_CONFIG = vscode.window.createStatusBarItem();
var WIDGET_CONFIG_OPTIONS = [];

var WIDGET_OS = vscode.window.createStatusBarItem();
var WIDGET_OS_OPTIONS = [];

var WIDGET_ARCHITECTURE = vscode.window.createStatusBarItem();
var WIDGET_ARCHITECTURE_OPTIONS = [];

var WIDGET_TOOLCHAIN = vscode.window.createStatusBarItem();
var WIDGET_TOOLCHAIN_OPTIONS = [];

var WIDGET_GRAPHICS = vscode.window.createStatusBarItem();
var WIDGET_GRAPHICS_OPTIONS = [];

// Status Bar Actions
var WIDGET_BUILD = vscode.window.createStatusBarItem();
var WIDGET_BUILD_RUN = vscode.window.createStatusBarItem();
var WIDGET_BEBUG = vscode.window.createStatusBarItem();
var WIDGET_RENDERDOC = vscode.window.createStatusBarItem();

var WIDGET_CLEAN = vscode.window.createStatusBarItem();
var WIDGET_NEW_PROJECT = vscode.window.createStatusBarItem();
var WIDGET_WORKSPACE = vscode.window.createStatusBarItem();


/**
 * @param {vscode.ExtensionContext} context
 */
async function activate( context )
{
	console.log( "'vscmanta' activated." );

	// Detect Workspace
	await WorkspaceDetect();

	// Close Terminals
	CloseTerminals();

	// Register Commands
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.about", CommandAbout ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.newProject", CommandNewProject ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.cleanProject", CommandClean ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.selectProject", CommandSelectProject ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.setWorkspace", CommandSetWorkspace ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.selectPlatform", CommandSelectOS ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.selectArchitecture", CommandSelectArchitecture ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.selectToolchain", CommandSelectToolchain ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.selectConfig", CommandSelectConfig ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.selectGraphics", CommandSelectRender ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.buildAndRun", CommandBuildAndRun ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.build", CommandBuild ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.run", CommandAbout ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.package", CommandAbout ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.debugRuntime", CommandDebugRuntime ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.renderdoc", CommandRenderDoc ) );
	context.subscriptions.push( vscode.commands.registerCommand( "vscmanta.insertCommentBreak", CommandInsertCommentBreak ) );

	// Build Configuration Widgets
	WidgetCreate( context, WIDGET_WORKSPACE, "[]", "vscmanta.setWorkspace" );
	WIDGET_WORKSPACE.tooltip = "Set workspace";

	WidgetCreate( context, WIDGET_PROJECT, "[]", "vscmanta.selectProject", "Yellow" );
	WIDGET_PROJECT.tooltip = "Select a project";

	WidgetCreate( context, WIDGET_CONFIG, "[]", "vscmanta.selectConfig" );
	WIDGET_CONFIG.tooltip = "Select a configuration";

	WidgetCreate( context, WIDGET_OS, "[]", "vscmanta.selectPlatform" );
	WIDGET_OS.tooltip = "Select a platform";

	WidgetCreate( context, WIDGET_ARCHITECTURE, "[]", "vscmanta.selectArchitecture" );
	WIDGET_ARCHITECTURE.tooltip = "Select an architecture";

	WidgetCreate( context, WIDGET_TOOLCHAIN, "[]", "vscmanta.selectToolchain" );
	WIDGET_TOOLCHAIN.tooltip = "Select a toolchain";

	WidgetCreate( context, WIDGET_GRAPHICS, "[]", "vscmanta.selectGraphics" );
	WIDGET_GRAPHICS.tooltip = "Select a graphics API";

	// Action Widgets
	WidgetCreate( context, WIDGET_BUILD, "🔵 Build", "vscmanta.build", "#75acff" );
	WIDGET_BUILD.tooltip = "Build & compile the project";

	WidgetCreate( context, WIDGET_BUILD_RUN, "🟢 Build & Run", "vscmanta.buildAndRun", "Lime" );
	WIDGET_BUILD_RUN.tooltip = "Build, compile, & run the project";

	WidgetCreate( context, WIDGET_BEBUG, "🔴 Debug", "vscmanta.debugRuntime", "#ff5959" );
	WIDGET_BEBUG.tooltip = "Build, compile, & debug the runtime";

	WidgetCreate( context, WIDGET_RENDERDOC, "🖼️ RenderDoc", "vscmanta.renderdoc", "#60f7be" );
	WIDGET_RENDERDOC.tooltip = "Build, compile, & run with RenderDoc";

	WidgetCreate( context, WIDGET_CLEAN, "🔅 Clean", "vscmanta.cleanProject", "Orange" );
	WIDGET_CLEAN.tooltip = "Delete cache for build, engine, and project";

	WidgetCreate( context, WIDGET_NEW_PROJECT, "➕ New Project", "vscmanta.newProject", "Violet" );
	WIDGET_NEW_PROJECT.tooltip = "Create a new project";

	// Update Workspace
	WorkspaceUpdate( false );
}


function deactivate()
{
}


module.exports = {
	activate,
	deactivate,
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


async function WorkspaceDetect()
{
	var dotMantaPath = path.join( PATH_WORKSPACE, ".manta" );
	if( fs.existsSync( dotMantaPath ) )
	{
		if( IsDirectory( dotMantaPath ) )
		{
			// Workspace is root -- we're working on manta
			WORKSPACE_IS_PROJECT = false;
			PATH_ENGINE = PATH_WORKSPACE;
			PATH_PROJECTS = path.join( PATH_ENGINE, "projects" );
			PATH_CACHE_ENGINE = path.join( PATH_ENGINE, ".manta", ".cache" );
			await LoadProjectCache( true );
		}
		else
		{
			// Workspace is in root/projects/ -- we're a project
			WORKSPACE_IS_PROJECT = true;
			PATH_PROJECT = PATH_WORKSPACE;
			PATH_PROJECTS = path.dirname( PATH_PROJECT );
			PATH_ENGINE = path.dirname( PATH_PROJECTS );
			PATH_CACHE_PROJECT = path.join( PATH_PROJECT, ".manta" );
			await LoadProjectCache( false );
		}
	}
	else
	{
		throw new Error( "ERROR: Workspace is not a valid manta project! (vscmanta)" );
	}
}


function WorkspaceUpdate( switchProject = true )
{
	// Update Workspace Widget
	WIDGET_WORKSPACE.text = WORKSPACE_IS_PROJECT ? "🔺" : "🔻";
	WIDGET_WORKSPACE.tooltip = WORKSPACE_IS_PROJECT ? "Open engine workspace" : "Open project workspace";

	// Switch Workspaces?
	if( WORKSPACE_IS_PROJECT && switchProject )
	{
		// Jump to different project (restarts the plugin)
		const projectWorkspace = vscode.Uri.file( path.join( PATH_PROJECTS, WIDGET_PROJECT.name ) );
		vscode.workspace.updateWorkspaceFolders( 0, 1, { uri: projectWorkspace } );
	}
	else
	{
		// Load Cache
		if( switchProject ) { UpdatePaths(); LoadProjectCache( false ); }

		// Verify Project
		CollectProjects();
		WidgetValueSet( WIDGET_PROJECT, WIDGET_PROJECT_OPTIONS, WIDGET_PROJECT.name, false );
		UpdatePaths();

		// Verify OS Settings
		DetectPlatform();
		WidgetValueSet( WIDGET_OS, WIDGET_OS_OPTIONS, WIDGET_OS.name, false );

		// Verify Project Config Options
		LoadProjectConfigJSON();
		WidgetValueSet( WIDGET_CONFIG, WIDGET_CONFIG_OPTIONS, WIDGET_CONFIG.name, false );

		// Verify OS-Specific Configurations
		LoadMantaConfigJSON();
		WidgetValueSet( WIDGET_ARCHITECTURE, WIDGET_ARCHITECTURE_OPTIONS, WIDGET_ARCHITECTURE.name, false );
		WidgetValueSet( WIDGET_TOOLCHAIN, WIDGET_TOOLCHAIN_OPTIONS, WIDGET_TOOLCHAIN.name, false );
		WidgetValueSet( WIDGET_GRAPHICS, WIDGET_GRAPHICS_OPTIONS, WIDGET_GRAPHICS.name, false );

		// Update VSCode settings
		SaveVSCodeSettings();

		// Save Cache
		SaveProjectCache();
	}
}


function UpdatePaths()
{
	var projectName = WIDGET_PROJECT.name;
	var projectExe = projectName + ( PlatformIsWindows() ? ".exe" : "" );

	//var PATH_WORKSPACE - set at top of file
	//var PATH_ENGINE - set in detectWorkspace()
	PATH_BOOT_SCRIPT = path.join( PATH_ENGINE, ( PlatformIsWindows() ? "boot.bat" : "boot.sh" ) );
	PATH_ENGINE_SOURCES = path.join( PATH_ENGINE, "source" );
	PATH_ENGINE_CONFIGS = path.join( PATH_ENGINE, ".manta", "configs.json" );

	PATH_PROJECTS = path.join( PATH_ENGINE, "projects" );
	PATH_PROJECT = path.join( PATH_PROJECTS, projectName );
	PATH_PROJECT_BUILD_SOURCES = path.join( PATH_PROJECT, "build" );
	PATH_PROJECT_RUNTIME_SOURCES = path.join( PATH_PROJECT, "runtime" );
	PATH_PROJECT_CONFIGS = path.join( PATH_PROJECT, "configs.json" );
	PATH_PROJECT_OUTPUT = path.join( PATH_PROJECT, "output" );
	PATH_PROJECT_OUTPUT_BOOT = path.join( PATH_PROJECT_OUTPUT, "boot" );
	PATH_PROJECT_OUTPUT_BUILD = path.join( PATH_PROJECT_OUTPUT, "build" );
	PATH_PROJECT_OUTPUT_GENERATED = path.join( PATH_PROJECT_OUTPUT, "generated" );
	PATH_PROJECT_OUTPUT_RUNTIME = path.join( PATH_PROJECT_OUTPUT, "runtime" );
	PATH_PROJECT_EXECUTABLE = path.join( PATH_PROJECT_OUTPUT_RUNTIME, projectExe );

	PATH_DOT_VSCODE = path.join( WORKSPACE_IS_PROJECT ? PATH_PROJECT : PATH_ENGINE, ".vscode" );
	PATH_DOT_MANTA = path.join( PATH_ENGINE, ".manta" );
	PATH_CACHE_ENGINE = path.join( PATH_DOT_MANTA, ".cache" );
	PATH_CACHE_PROJECT = path.join( PATH_PROJECT, ".manta" );
}


function LoadProjectConfigJSON()
{
	try {
		const configFileData = fs.readFileSync( PATH_PROJECT_CONFIGS, 'utf-8' );
		const configJSON = JSON.parse( configFileData );
		const configNames = Object.keys( configJSON/*.runtime*/ );
		WIDGET_CONFIG_OPTIONS = configNames.map( name => ( { label: name } ) );
	} catch ( error ) { throw new Error( `ERROR: Failed to read project configs.json file! (${PATH_PROJECT_CONFIGS})` ); }

	WidgetValueSet( WIDGET_CONFIG, WIDGET_CONFIG_OPTIONS, WIDGET_CONFIG.name, false );
}


function LoadMantaConfigJSON()
{
	// Load build configs json
	var configOptions = { }
	try {
		const configFileData = fs.readFileSync( PATH_ENGINE_CONFIGS, 'utf-8' );
		configOptions = JSON.parse( configFileData );
	} catch ( error ) { throw new Error( `ERROR: Failed to read engine configs.json file! (${PATH_ENGINE_CONFIGS})` ); }

	// Store Options
	WIDGET_ARCHITECTURE_OPTIONS = configOptions[WIDGET_OS.name]?.["-architecture="] || [];
	WIDGET_TOOLCHAIN_OPTIONS = configOptions[WIDGET_OS.name]?.["-toolchain="] || [];
	WIDGET_GRAPHICS_OPTIONS = configOptions[WIDGET_OS.name]?.["-gfx="] || [];
}


function SaveProjectCache()
{
	// Save Build Configurations
	const configDataEngine =
	{
		project: WIDGET_PROJECT.name,
		config: WIDGET_CONFIG.name,
		platform: WIDGET_OS.name,
		architecture: WIDGET_ARCHITECTURE.name,
		toolchain: WIDGET_TOOLCHAIN.name,
		gfx: WIDGET_GRAPHICS.name,
	};

	// Save Cache
	try {
		var data = JSON.stringify( configDataEngine, null, 2 );
		fs.writeFileSync( PATH_CACHE_PROJECT, data, "utf-8" );
		fs.writeFileSync( PATH_CACHE_ENGINE, data, "utf-8" );
	} catch ( error ) { }
}


async function LoadProjectCache( engineCache = false )
{
	// Load Config Cache
	try {
		const fileData = fs.readFileSync( engineCache ? PATH_CACHE_ENGINE : PATH_CACHE_PROJECT, 'utf-8' );
		const loadedValues = JSON.parse( fileData );

		if( engineCache ) { WIDGET_PROJECT.name = loadedValues.project };
		WIDGET_CONFIG.name = loadedValues.config;
		WIDGET_OS.name = loadedValues.platform;
		WIDGET_ARCHITECTURE.name = loadedValues.architecture;
		WIDGET_TOOLCHAIN.name = loadedValues.toolchain;
		WIDGET_GRAPHICS.name = loadedValues.gfx;
	}
	catch ( error )	{ }

	// Make sure a project is selected
	if( !WIDGET_PROJECT.name && WORKSPACE_IS_PROJECT ) { WIDGET_PROJECT.name = path.basename( PATH_PROJECT ); }
	CollectProjects();
	WidgetValueSet( WIDGET_PROJECT, WIDGET_PROJECT_OPTIONS, WIDGET_PROJECT.name, false );
	if( !WIDGET_PROJECT.name ) { await CreateNewProject( "TemplateProject" ); }

	// Update Paths
	UpdatePaths();
	if( WORKSPACE_IS_PROJECT && PATH_PROJECT !== PATH_WORKSPACE ) { WorkspaceUpdate( true ); }
}


function CollectProjects()
{
	try {
		// Read the contents of the "projects" directory
		const filesAndFolders = fs.readdirSync( PATH_PROJECTS );

		// Filter only directories (projects) from the contents
		const projects = filesAndFolders
			.filter( item => fs.statSync( path.join( PATH_PROJECTS, item ) ).isDirectory() )
			.map( item => {
				// Get the modification date of the .manta file
				const dotMantaPath = path.join( PATH_PROJECTS, item, ".manta" );
				if( fs.existsSync( dotMantaPath ) )
				{
					const dotMantaStat = fs.statSync( dotMantaPath );
					const modificationDate = dotMantaStat.mtime;
					return { label: item, modificationDate };
				}
				else
				{
					return { label: item, modificationDate: null };
				}
			} );

		// Sort projects by modification date in descending order
		projects.sort( ( a, b ) => b.modificationDate - a.modificationDate );
		WIDGET_PROJECT_OPTIONS = projects;
	} catch( error ) { WIDGET_PROJECT_OPTIONS = [ { label: "" } ]; }
}


function PlatformIsWindows() { return PLATFORM == "win32"; }
function PlatformIsMacOS() { return PLATFORM == "darwin"; }
function PlatformIsLinux() { return PLATFORM == "linux"; }


function DetectPlatform()
{
	if( PlatformIsWindows() ) { WIDGET_OS_OPTIONS = [ { label: "windows" } ]; } else
	if( PlatformIsMacOS() )   { WIDGET_OS_OPTIONS = [ { label: "macOS" } ]; } else
	if( PlatformIsLinux() )   { WIDGET_OS_OPTIONS = [ { label: "linux" } ]; } else
							  { WIDGET_OS_OPTIONS = [ { label: "unknown" } ]; }
}


function WidgetCreate( context, widget, text, command, color = "White" )
{
	widget.command = command;
	widget.text = text;
	widget.color = color;
	widget.show();
	context.subscriptions.push( widget );
}


function WidgetValueSet( widget, options, value, saveCache = true )
{
	// Update Value
	widget.name = value;

	// Check if the value is valid
	var found = false;
	for( let i = 0; i < options.length; i++ )
	{
		if( options[i].label === widget.name ) { found = true; break; }
	}
	if( !found ) { widget.name = options.length > 0 ? options[0].label : ""; }

	// Update Widget Text
	widget.text = `[${widget.name}]`;

	// Cache Configuration State
	if( saveCache == true ) { SaveProjectCache(); }
}


function WidgetValueChoosePrompt( widget, options, saveCache, OnChangeCallback = null )
{
	if( options.length > 0 )
	{
		const quickPick = vscode.window.createQuickPick();
		quickPick.items = options;
		quickPick.placeholder = widget.tooltip;
		quickPick.show();

		// Update Status Bar Widget
		quickPick.onDidChangeSelection( items => {
			if( items.length > 0 )
			{
				WidgetValueSet( widget, options, items[0].label, saveCache );
				if( OnChangeCallback ) { OnChangeCallback(); }
			}
		} );

		// Close After Selection
		quickPick.onDidAccept( () => { quickPick.dispose(); } );
	}
}


async function CreateNewProject( projectName )
{
	// Make sure the name isn't empty
	if( !projectName ) { return; }
	var newProjectPath = path.join( PATH_PROJECTS, projectName );

	try {
		// Make new project directory
		try { await fs.promises.mkdir( PATH_PROJECTS ) } catch( error ) { };
		try { await fs.promises.mkdir( newProjectPath ) } catch( error ) { };

		// Copy template project contents to new project
		PATH_TEMPLATE_PROJECT = path.join( PATH_ENGINE, ".manta", "template" );
		await CopyDirectory( PATH_TEMPLATE_PROJECT, newProjectPath );

		// Update currently selected game
		CollectProjects();
		WidgetValueSet( WIDGET_PROJECT, WIDGET_PROJECT_OPTIONS, projectName, false );
		WorkspaceUpdate( true );

		// Success
		vscode.window.showInformationMessage( `Project "${projectName}" created successfully.` );
	} catch ( error ) { vscode.window.showErrorMessage( `Error creating project: ${error.message}` ); }
}


function IsDirectory( path )
{
	try {
	  const stats = fs.statSync( path );
	  return stats.isDirectory();
	} catch ( error ) { throw new Error( `ERROR: Failed isDirectory() test on ${path}` ); }
}


function CopyDirectory( src, dst )
{
	// Copy directory contents
	if( !fs.existsSync( dst ) ) { fs.mkdirSync( dst ); }
	const files = fs.readdirSync( src );
	for( const file of files )
	{
		const srcPath = path.join( src, file );
		const dstPath = path.join( dst, file );

		if( IsDirectory( srcPath ) )
		{
			// Recurse into directories
			CopyDirectory( srcPath, dstPath );
		}
		else
		{
			// Copy files
			fs.copyFileSync( srcPath, dstPath );
		}
	}
}


async function GlobFiles( path, pattern )
{
	var filePattern = new vscode.RelativePattern( path, pattern );
	try {
		var fileUris = await vscode.workspace.findFiles( filePattern );
		return fileUris.map( uri => uri.fsPath );
	} catch ( error ) { }
}


function WriteVSCodeSettingsJSON()
{
	var searchExcludes = { "**/.manta": true };
	if( !WORKSPACE_IS_PROJECT )
	{
		WIDGET_PROJECT_OPTIONS.forEach( project =>
		{
			if( project.label !== WIDGET_PROJECT.name ) { searchExcludes[`**/projects/${project.label}`] = true; }
		} );
	}

	const json =
	{
		"files.associations": { "*.object": "cpp" },
		"search.exclude": searchExcludes,
		"search.useIgnoreFiles": false
	};

	// Write .vscode/settings.json
	const jsonPath = path.join( PATH_DOT_VSCODE, "settings.json" )
	try { fs.writeFileSync( jsonPath, JSON.stringify( json, null, 4 ) ); }
	catch( error ) { }
}


function WriteVSCodeCppPropertiesJSON()
{
	var name;
	if( PlatformIsWindows() )  { name = "Win32"; } else
	if( PlatformIsLinux() )  { name = "Linux"; } else
	if( PlatformIsMacOS() ) { name = "Mac"; }

	const json =
	{
		"configurations":
		[
			{
				"name": name,
				"cppStandard": "c++20",
				"includePath":
				[
					PATH_ENGINE_SOURCES,
					PATH_PROJECT_BUILD_SOURCES,
					PATH_PROJECT_RUNTIME_SOURCES,
					PATH_PROJECT_OUTPUT_GENERATED,
				]
			}
		]
	};

	// Write .vscode/c_cpp_properties.json
	const jsonPath = path.join( PATH_DOT_VSCODE, "c_cpp_properties.json" )
	try { fs.writeFileSync( jsonPath, JSON.stringify( json, null, 4 ) ); }
	catch( error ) { }
}


function WriteVSCodeLaunchJSON()
{
	const json =
	{
		"version": "0.2.0",
		"configurations":
		[
			{
				"name": "debug-runtime",
				"type": "cppvsdbg",
				"request": "launch",
				"program": PATH_PROJECT_EXECUTABLE,
				"cwd": PATH_PROJECT_OUTPUT_RUNTIME,
				"logging": {
					"moduleLoad": false
				}
			}
		]
	};

	// Write .vscode/launch.json
	const jsonPath = path.join( PATH_DOT_VSCODE, "launch.json" );
	try { fs.writeFileSync( jsonPath, JSON.stringify( json, null, 4 ) ); }
	catch( error ) { }
}


function SaveVSCodeSettings()
{
	// Ensure .vscode folder exists
	try { fs.mkdirSync( PATH_DOT_VSCODE ); } catch( error ) { }

	// Write JSONs
	WriteVSCodeSettingsJSON();
	WriteVSCodeCppPropertiesJSON();
	WriteVSCodeLaunchJSON();
}


function GetBootCommand()
{
	// Boot Args
	var bootArgs= [
		`-project=${WIDGET_PROJECT.name}`,
		`-config=${WIDGET_CONFIG.name}`,
        `-os=${WIDGET_OS.name}`,
        `-architecture=${WIDGET_ARCHITECTURE.name}`,
        `-toolchain=${WIDGET_TOOLCHAIN.name}`,
        `-gfx=${WIDGET_GRAPHICS.name}`,
    ];
	bootArgs = bootArgs.join( " " );

	// Boot Command
	var bootCommand = "";
	if( PlatformIsWindows() )
	{
		bootCommand += "cls & ";
		bootCommand += PATH_BOOT_SCRIPT;
	}
	else if ( PlatformIsLinux() || PlatformIsMacOS() )
	{
		bootCommand += "clear; chmod +x ";
		//bootCommand += WORKSPACE_IS_PROJECT ? "../../boot.sh; ../../boot.sh" : "./boot.sh; ./boot.sh";
		bootCommand += PATH_BOOT_SCRIPT;
		bootCommand += "; ";
		bootCommand += PATH_BOOT_SCRIPT;
	}
	bootCommand += " ";
	bootCommand += bootArgs;

	return bootCommand;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Commands
function CommandAbout()
{
	vscode.window.showInformationMessage( "vscmanta plugin" );
}


function CommandSelectProject()
{
	SaveProjectCache();
	CollectProjects();
	WidgetValueChoosePrompt( WIDGET_PROJECT, WIDGET_PROJECT_OPTIONS, false, WorkspaceUpdate );
}


function CommandSetWorkspace()
{
	SaveProjectCache();
	const projectWorkspace = vscode.Uri.file( WORKSPACE_IS_PROJECT ? PATH_ENGINE : PATH_PROJECT );
	vscode.workspace.updateWorkspaceFolders( 0, 1, { uri: projectWorkspace } );
	WorkspaceUpdate( false );
}


function CommandSelectOS()
{
	WidgetValueChoosePrompt( WIDGET_OS, WIDGET_OS_OPTIONS, true );
}


function CommandSelectArchitecture()
{
	WidgetValueChoosePrompt( WIDGET_ARCHITECTURE, WIDGET_ARCHITECTURE_OPTIONS, true );
}


function CommandSelectToolchain()
{
	WidgetValueChoosePrompt( WIDGET_TOOLCHAIN, WIDGET_TOOLCHAIN_OPTIONS, true );
}


function CommandSelectConfig()
{
	WidgetValueChoosePrompt( WIDGET_CONFIG, WIDGET_CONFIG_OPTIONS, true );
}


function CommandSelectRender()
{
	WidgetValueChoosePrompt( WIDGET_GRAPHICS, WIDGET_GRAPHICS_OPTIONS, true );
}


function CommandBuildAndRun()
{
	var cmd = GetBootCommand() + " -run=1";
	TerminalRunCommand( { saveAll: true, command: cmd } );
}


function CommandBuild()
{
	var cmd = GetBootCommand() + " -run=0";
	TerminalRunCommand( { saveAll: true, command: cmd } );
}


async function CommandDebugRuntime()
{
	// Build the project & wait for it to finish
	var cmd = GetBootCommand() + " -run=0";
	await CommandRunWait( { saveAll: true, command: cmd } );
	CommandStartDebugging( { config: "debug-runtime" } );
}


async function CommandRenderDoc()
{
	// Build the project & wait for it to finish
	var cmd = GetBootCommand() + " -run=0";
	await CommandRunWait( { saveAll: true, command: cmd } );

	try {
		// Run project w/ RenderDoc
		await child_process.execSync( `renderdoccmd capture -w -c capture ${PATH_PROJECT_EXECUTABLE}` );

		// Replace '*.rdc' with your desired file extension or pattern
		var captures = await GlobFiles( PATH_PROJECT_OUTPUT_RUNTIME, '*.rdc' );
		if( captures && captures[0] )
		{
			// Run RenderDoc & delete captures
			await child_process.execSync( `qrenderdoc ${captures[0]}` );
			for( const capture of captures ) { await fs.rm( capture ); }
		}
	}
	catch( error )
	{
		if( PlatformIsWindows() )
		{
			ShowMessage( `RenderDoc failed! Is it configured in the PATH system variable? (${error})` );
		}
		else
		{
			ShowMessage( `RenderDoc failed! Is it installed on the system? (${error})` );
		}
	}
}


function CommandClean()
{
	var outputPath = path.join( PATH_ENGINE, "projects", WIDGET_PROJECT.name, "output" );
	var cleanCommand;

	if( PlatformIsWindows() )
	{
		cleanCommand = [
			"cls",
			`if exist ${outputPath} rmdir ${outputPath} /s /q`,
		];
		cleanCommand = cleanCommand.join( " & " );
	}
	else
	{
		cleanCommand = [
			"clear",
			`rm -rf ${outputPath}`,
		];
		cleanCommand = cleanCommand.join( " ; " );
	}

	TerminalRunCommand( { saveAll: true, command: cleanCommand } );
}


async function CommandNewProject()
{
	// Prompt user for new project name
	const projectName = await vscode.window.showInputBox( { prompt: "Enter project name", placeHolder: "" } );

	// Create Project
	await CreateNewProject( projectName );
}


// Terminal used by runTerminalCommand()
let g_Terminal;
let g_TerminalPID;

async function TerminalRunCommand( args )
{
	if( !args )
	{
		vscode.window.showErrorMessage( "termExec: no arguments specified to execute." );
		return;
	}

	if( !args.command )
	{
		vscode.window.showErrorMessage( "termExec: no command specified to execute." );
		return;
	}

	if( !g_Terminal )
	{
		// Create Terminal
		g_Terminal = vscode.window.createTerminal( "terminal" );
		g_TerminalPID = await g_Terminal.processId;

		// Make sure to delete our reference if the user closes the terminal.
		vscode.window.onDidCloseTerminal( async (event) =>
		{
			if( await event.processId === g_TerminalPID )
			{
				g_Terminal = undefined;
				g_TerminalPID = undefined;
			}
		} );
	}

	if( args.saveAll )
	{
		await vscode.workspace.saveAll( false );
	}

	g_Terminal.sendText( args.command, true );
	g_Terminal.show( true );
}


async function CommandRunWait( args )
{
	if( !args )
	{
		vscode.window.showErrorMessage( "termExec: no arguments specified to execute." );
		return;
	}

	if( !args.command )
	{
		vscode.window.showErrorMessage( "termExec: no command specified to execute." );
		return;
	}

	if( !g_Terminal )
	{
		// Create Terminal
		g_Terminal = vscode.window.createTerminal( "terminal" );
		g_TerminalPID = await g_Terminal.processId;

		// Make sure to delete our reference if the user closes the terminal.
		vscode.window.onDidCloseTerminal( async (event) =>
		{
			if( await event.processId === g_TerminalPID )
			{
				g_Terminal = undefined;
				g_TerminalPID = undefined;
			}
		});
	}

	if( args.saveAll ) { await vscode.workspace.saveAll( false ); }

	const lockPath = path.join( PATH_DOT_VSCODE, "termExec.lock" );
	const writeLock = PlatformIsWindows() ? " & echo lock> " : "; touch ";
	const command = args.command + writeLock + lockPath;
	g_Terminal.sendText( command, true );
	g_Terminal.show( true );

	// Wait for the lock file to be written.
	const lockTimeout = 30000;
	await fsExistsWithTimeout( lockPath, lockTimeout );

	// Delete the lock file.
	fs.rmSync( lockPath );
}

async function CommandStartDebugging( args )
{
	if( !args.config )
	{
		vscode.window.showErrorMessage( "termExec: no configuration specified to debug." );
		return;
	}

	vscode.debug.startDebugging( vscode.workspace.workspaceFolders[0], args.config );
}

function CommandInsertCommentBreak()
{
	// Get the active text editor
	const editor = vscode.window.activeTextEditor;

	if( editor )
	{
		// Insert Comment Break
		const position = editor.selection.active;
		editor.edit( editBuilder =>
		{
			editBuilder.insert( position, "////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////" );
		} );
	}
}

async function OpenFile( path )
{
	let relative = vscode.Uri.joinPath( vscode.workspace.workspaceFolders[0].uri, path );
	let document = await vscode.workspace.openTextDocument( relative );

	return await vscode.window.showTextDocument( document, { preview: false } );
}

function fsExistsWithTimeout( filePath, timeout )
{
	return new Promise( (resolve, reject) =>
	{
		var timer = setTimeout( () =>
		{
			watcher.close();
			reject( new Error( "termExec: lock file was not created during the timeout." ) );
		}, timeout );

		fs.access( filePath, fs.constants.R_OK, ( err ) =>
		{
			if( !err )
			{
				clearTimeout( timer );
				watcher.close();
				resolve();
			}
		});

		var dir = path.dirname( filePath );
		var basename = path.basename( filePath );
		var watcher = fs.watch( dir, ( eventType, filename ) =>
		{
			if( eventType === 'rename' && filename === basename )
			{
				clearTimeout( timer );
				watcher.close();
				resolve();
			}
		} );
	});
}


function ShowMessage( message )
{
	vscode.window.showInformationMessage( message );
}


function CloseTerminals()
{
    const terminals = vscode.window.terminals;
    terminals.forEach( terminal => { terminal.dispose(); } );
}