#pragma once

#include <pipeline.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Process
{
#if PIPELINE_OS_WINDOWS
	void *handle;
#elif PIPELINE_OS_MACOS || PIPELINE_OS_LINUX
	int pid;
#endif
	bool running;
	int exitCode;
};

static bool process_launch( Process &process, const char *exe, char **argv, int instanceID = 0 );
static bool process_poll( Process &process );
static void process_wait( Process &process );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if PIPELINE_OS_WINDOWS

#include <vendor/windows.hpp>
#include <vendor/string.hpp>
#include <vendor/stdio.hpp>

static bool process_launch( Process &process, const char *exe, char **argv, int instanceID )
{
	STARTUPINFOA si { };
	PROCESS_INFORMATION pi { };
	si.cb = sizeof( si );

	char cmd[1024];
	snprintf( cmd, sizeof( cmd ), "\"%s\"", exe );
	for( int i = 1; argv[i]; i++ )
	{
		strncat( cmd, " ", sizeof( cmd ) - strlen( cmd ) - 1 );
		strncat( cmd, argv[i], sizeof( cmd ) - strlen( cmd ) - 1 );

	}

	char instance[32];
	snprintf( instance, sizeof( instance ), " -instance=%d", instanceID );
	strncat( cmd, instance, sizeof( cmd ) - strlen( cmd ) - 1 );

	if( !CreateProcessA( nullptr, cmd, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi ) ) { return false; }

	CloseHandle( pi.hThread );

	process.handle = pi.hProcess;
	process.running = true;
	process.exitCode = -1;

	return true;
}


static bool process_poll( Process &process )
{
	if( !process.running ) { return false; }

	DWORD code;
	if( GetExitCodeProcess( static_cast<HANDLE>( process.handle ), &code ) )
	{
		if( code != STILL_ACTIVE )
		{
			process.exitCode = static_cast<int>( code );
			process.running = false;
			CloseHandle( static_cast<HANDLE>( process.handle ) );
			return false;
		}
	}
	return true;
}


static void process_wait( Process &process )
{
	WaitForSingleObject( static_cast<HANDLE>( process.handle ), INFINITE );
	DWORD code;
	GetExitCodeProcess( static_cast<HANDLE>( process.handle ), &code );
	process.exitCode = static_cast<int>( code );
	process.running = false;
	CloseHandle( static_cast<HANDLE>( process.handle ) );
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if PIPELINE_OS_MACOS || PIPELINE_OS_LINUX

#include <vendor/posix.hpp>

static bool process_launch( Process &process, const char *exe, char **argv, int instanceID )
{
	// TODO: Ideally each process should spawn its own terminal, like Windows

	pid_t pid = fork();
	if( pid == 0 )
	{
		argv[0] = const_cast<char *>( exe );
		execvp( exe, argv );
		_exit( 127 );
	}
	if( pid < 0 ) { return false; }

	process.pid = pid;
	process.running = true;
	process.exitCode = -1;
	return true;
}


static bool process_poll( Process &process )
{
	if( !process.running ) { return false; }

	int status;
	pid_t r = waitpid( process.pid, &status, WNOHANG );
	if( r == 0 ) { return true; }

	if( r == process.pid )
	{
		if( WIFEXITED( status ) )
		{
			process.exitCode = WEXITSTATUS( status );
		}
		else
		{
			process.exitCode = -1;
		}
		process.running = false;
		return false;
	}
	return true;
}


static void process_wait( Process &process )
{
	int status;
	waitpid( process.pid, &status, 0 );
	if( WIFEXITED( status ) )
	{
		process.exitCode = WEXITSTATUS( status );
	}
	else
	{
		process.exitCode = -1;
	}
	process.running = false;
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////