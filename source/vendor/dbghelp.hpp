#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <dbghelp.h>
	#include <vendor/conflicts.hpp>
#else
	#include <vendor/windows.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dbghelp.h

	struct SYMBOL_INFO
	{
		ULONG SizeOfStruct;
		ULONG TypeIndex;
		ULONG64 Reserved[2];
		ULONG Index;
		ULONG Size;
		ULONG64 ModBase;
		ULONG Flags;
		ULONG64 Value;
		ULONG64 Address;
		ULONG Register;
		ULONG Scope;
		ULONG Tag;
		ULONG NameLen;
		ULONG MaxNameLen;
		CHAR Name[1];
	};

	struct SYMBOL_INFOW
	{
		ULONG SizeOfStruct;
		ULONG TypeIndex;
		ULONG64 Reserved[2];
		ULONG Index;
		ULONG Size;
		ULONG64 ModBase;
		ULONG Flags;
		ULONG64 Value;
		ULONG64 Address;
		ULONG Register;
		ULONG Scope;
		ULONG Tag;
		ULONG NameLen;
		ULONG MaxNameLen;
		WCHAR Name[1];
	};

	struct IMAGEHLP_LINE
	{
		DWORD SizeOfStruct;
		PVOID Key;
		DWORD LineNumber;
		PCHAR FileName;
		DWORD Address;
	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

// dbghelp.dll (NOTE: Link to the DLL only when necessary (debug builds)
typedef BOOL ( *SymInitializeFunc )( HANDLE, PCSTR, BOOL );
typedef WORD ( *RtlCaptureStackBackTraceFunc )( DWORD, DWORD, PVOID *, DWORD * );
typedef BOOL ( *SymFromAddrFunc )( HANDLE, DWORD64, DWORD64 *, SYMBOL_INFO * );
typedef BOOL ( *SymGetLineFromAddrFunc )( HANDLE, DWORD64, DWORD *, IMAGEHLP_LINE * );
typedef BOOL ( *SymCleanupFunc )( HANDLE );