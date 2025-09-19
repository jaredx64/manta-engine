@echo off
:: usage: boot.bat -project=<project> -os=<os> -architecture=<architecture> -toolchain=<toolchain> -config=<config> -gfx=<api> -clean=<0/1> -run=<0/1>
	:: -project=        project from projects/
	:: -config=         release, debug (from projects/^<project^>/configs.json)
	:: -gfx=            opengl, d3d11
	:: -os=             windows, linux, macos
	:: -toolchain=      msvc, llvm, gnu
	:: -architecture=   x64, arm, arm64
	:: -clean=          0 - cached build, 1 - clears build cache
	:: -run=            0 - build only, 1 - runs executable

:: The first thing we should do is rebase the working directory so the script
:: can be called from anywhere while actually working at the root of the repo.
pushd "%~dp0\"

setlocal enabledelayedexpansion

set toolchain=msvc
set project=

:parse_args
	:: Check if we've finished parsing arguments
	if "%~1" == "" goto build

	:: Parse Arguments
	if /i "%~1" == "-help" (
		goto print_usage
	)

	if /i "%~1" == "-project" (
		set project=%~2
	) else (
		shift
	)

	if /i "%~1" == "-toolchain" (
		set toolchain=%~2
	) else (
		shift
	)

	goto parse_args

:build
	:: Clear Terminal
	cls

	:: Require a project
	if "%project%" == "" (
		echo Bootstrap failed: No -project=<project> specified
		goto print_usage
	)

	:: Ensure project exists
	if not exist projects\%project% (
		echo Bootstrap failed: project '%project%' does not exist!
		goto final
	)

	:: Load vcvars64 (required for MSVC)
	if %toolchain% == msvc (
		set "vcvars64_file=boot-vcvars64.txt"
		set "default_vcvars64=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

		:: Check if boot-vcvars64.txt exists
		if not exist "!vcvars64_file!" (
			echo !default_vcvars64! > "!vcvars64_file!"
			if %errorlevel% neq 0 (
				echo Error: Failed to write to !vcvars64_file!
				echo.
				goto final
			)
		)

		:: Read vcvars64.bat path from the file
		set "vcvars64_path="
		for /f "usebackq delims=" %%i in ("!vcvars64_file!") do set "vcvars64_path=%%i"

		:: Validate read path
		if "!vcvars64_path!" == "" (
			echo Error: boot-vcvars64.txt is empty or invalid.
			goto final
		)

		:: Run vcvars64.bat
		if "%VSINSTALLDIR%" == "" (
			:: Ensure vcvars64.bat exists
			if not exist "!vcvars64_path!" (
				goto print_vcvars_error
			)

			:: Call vcvars64.bat
			call "!vcvars64_path!"
			if errorlevel 1 (
				goto print_vcvars_error
			)
		)
	)

	:: Skip this step if boot.exe already exists
	if exist projects\%project%\output\boot\boot.exe (
		goto strap
	)

	:: Make sure the projects\%project%\output\boot\objects directory exists
	if not exist projects\%project%\output\boot\objects (
		mkdir projects\%project%\output\boot\objects
	)

	:: Call the compiler to make boot.exe
	echo Boot: %project%, %toolchain% [%*]
	if %toolchain% == msvc (
		cl source\boot\boot.cpp -std:c++20 -EHsc -Isource -Isource\boot -nologo -MD -Fe:projects\%project%\output\boot\boot.exe -Fo:projects\%project%\output\boot\objects -DCOMPILE_DEBUG=1 -DEBUG
	)
	if %toolchain% == llvm (
		clang source\boot\boot.cpp -std=c++20 -fno-exceptions -Isource -Isource\boot -o projects\%project%\output\boot\boot.exe -DCOMPILE_DEBUG=1 -g -ldbghelp
	)
	if %toolchain% == gnu (
		gcc source\boot\boot.cpp -std=c++20 -fno-exceptions -Isource -Isource\boot -o projects\%project%\output\boot\boot.exe -DCOMPILE_DEBUG=1 -g -ldbghelp
	)

:strap
	call projects\%project%\output\boot\boot.exe -package=0 -verbose=0 %*
	goto final

:print_usage
	echo.
	echo Usage: boot.bat -project=^<project^> -config=^<config^> -gfx=^<api^> -toolchain=^<toolchain^> -architecture=^<architecture^> -os=^<os^> -clean=^<0/1^> -run=^<0/1^>
	echo.
	echo Required Arguments:
	echo   -project=        project from projects/
	echo.
	echo Optional Arguments:
	echo   -config=         release, debug, ... (from projects/^<project^>/configs.json)
	echo   -gfx=            opengl, d3d11
	echo   -os=             windows, linux, macos
	echo   -toolchain=      msvc, llvm, gnu
	echo   -architecture=   x64, arm32, arm64
	echo   -os=             windows, linux, macos
	echo   -clean=          0 - cached build, 1 - clears build cache
	echo   -run=            0 - build only, 1 - runs executable
	echo.
	goto final

:print_vcvars_error
	echo Error: Could not load vcvars64.bat for MSVC
	echo        !vcvars64_path!
	echo.
	echo To fix:
	echo 1.^) Ensure Microsoft Visual Studio is installed on your machine
	echo 2.^) Locate its vcvars64.bat
	echo 3.^) Update boot-vcvars64.txt (located next to this script) with the appropriate path for your machine
	echo.
	echo Alternatively, try -toolchain=llvm or -toolchain=gnu if you have LLVM or GNU installed
	echo.
	goto final

:final
	popd