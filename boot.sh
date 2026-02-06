#!/bin/bash

# usage: boot.sh -project=<project> -config=<config> -gfx=<api> -toolchain=<toolchain> -architecture=<architecture> -os=<os> -clean=<0/1> -run=<0/1>
	# -project=        project from projects/
	# -config=         release, debug, ... (from projects/<project>/configs.json)
	# -gfx=            opengl, d3d11
	# -os=             windows, linux, macos
	# -toolchain=      llvm, gnu
	# -architecture=   x64, arm64
	# -os=             windows, linux, macos
	# -clean=          0 - cached build, 1 - clears build cache
	# -run=            0 - build only, 1 - runs executable

args="$@"
toolchain="llvm"
project=""

print_usage()
{
	echo "Usage: boot.sh -project=<project> -config=<config> -gfx=<api> -toolchain=<toolchain> -architecture=<architecture> -os=<os> -clean=<0/1> -run=<0/1>"
	echo ""
	echo "Required Arguments:"
	echo "  -project=        project from projects/"
	echo ""
	echo "Optional Arguments:"
	echo "  -config=         release, debug, ... (from projects/<project>/configs.json)"
	echo "  -gfx=            opengl, d3d11"
	echo "  -os=             windows, linux, macos"
	echo "  -toolchain=      msvc, llvm, gnu"
	echo "  -architecture=   x64, arm64"
	echo "  -os=             windows, linux, macos"
	echo "  -clean=          0 - cached build, 1 - clears build cache"
	echo "  -run=            0 - build only, 1 - runs executable"
	exit 1
}

parse_args()
{
	# Parse command-line arguments
	while [[ $# -gt 0 ]]; do
		case "$1" in
			-project=*)
				project="${1#*=}"
				;;
			-toolchain=*)
				toolchain="${1#*=}"
				;;
			-help)
				print_usage
				;;
			*)
				# Skip unneeded args
				;;
		esac
		shift
	done

	# Goto build
	build
}

build()
{
	# Clear Terminal
	clear
	echo -e "\033c"

	# Require a project
	if [ -z "$project" ]; then
		echo "Bootstrap failed: No -project=<project> specified"
		echo ""
		print_usage
		return
	fi

	# Ensure project exists
	if [ ! -d "projects/$project" ]; then
		echo "Bootstrap failed: project '$project' does not exist!"
		return
	fi

	# Skip this step if boot executable already exists
	if [ -f "projects/$project/output/boot/boot" ]; then
		strap
		return
	fi

	# Make sure the projects/$project/output/boot/objects directory exists
	mkdir -p "projects/$project/output/boot/objects"

	# MSVC isn't supported on macOS & linux
	if [ "$toolchain" == "msvc" ]; then
		echo "Bootstrap failed: msvc toolchain not supported on this platform"
		return
	fi

	# Call the compiler to make boot executable
	echo "Boot: $project, $toolchain [$args]"
	if [ "$toolchain" == "llvm" ]; then
		clang "source/boot/boot.cpp" -std=c++20 -fno-exceptions -I"source" -I"source/boot" -o "projects/$project/output/boot/boot"
	elif [ "$toolchain" == "gnu" ]; then
		g++ "source/boot/boot.cpp" -std=c++20 -fno-exceptions -I"source" -I"source/boot" -o "projects/$project/output/boot/boot"
	fi

	# Verify that the executable was created successfully
	if [ $? -eq 0 ]; then
		echo "Boot: compile successful"
		chmod +x "projects/$project/output/boot/boot"  # Make the executable executable
		strap
	else
		echo "Boot: compile failed"
		return
	fi
}

strap()
{
	"./projects/$project/output/boot/boot" "-package=0 -verbose=0" $args
}

# Ensure the working directory matches this script's location
if ! cd "$(dirname "$0")"; then
	echo "Boot error: failed to reset working directory" >&2
	exit 1
fi

# Parse Args (entry point)
parse_args "$@"