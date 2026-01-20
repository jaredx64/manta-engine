#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <windows.h>
		#include <windowsx.h>
		#include <Winuser.h>
		#include <WinBase.h>
		#include <ShlObj.h>
	#include <vendor/conflicts.hpp>

	#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#else
	#include <vendor/vendor.hpp>

	#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004

	#define STILL_ACTIVE ((DWORD)0x00000103L)

	#define T3XT(x) L##x
	#define TEXT(x) T3XT(x)
	#define CALLBACK STD_CALL

	#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
	#define FAILED(hr) (((HRESULT)(hr)) < 0)
	#define TIMERR_NOERROR 0

	#define LOWORD(l) ((WORD)(((UINT_PTR)(l)) & 0xFFFF))
	#define HIWORD(l) ((WORD)((((UINT_PTR)(l)) >> 16) & 0xFFFF))

	#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
	#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
	#define GET_WHEEL_DELTA_WPARAM(wp) ((short)HIWORD(wp))

	#define IS_HIGH_SURROGATE(x) (((x) >= HIGH_SURROGATE_START) && ((x) <= HIGH_SURROGATE_END))
	#define IS_LOW_SURROGATE(x) (((x) >= LOW_SURROGATE_START) && ((x) <= LOW_SURROGATE_END))
	#define IS_SURROGATE_PAIR(hs, ls) (IS_HIGH_SURROGATE(hs) && IS_LOW_SURROGATE(ls))

	#define MAKEINTRESOURCEW(i) ((LPWSTR)((UINT_PTR)((WORD)(i))))

	#define IDC_ARROW MAKEINTRESOURCEW(32512)
	#define IDC_IBEAM MAKEINTRESOURCEW(32513)
	#define IDC_HAND MAKEINTRESOURCEW(32649)

	#define HIGH_SURROGATE_START 0xD800
	#define HIGH_SURROGATE_END 0xDBFF
	#define LOW_SURROGATE_START 0xDC00
	#define LOW_SURROGATE_END 0xDFFF

	#define UNICODE_NOCHAR 0xFFFF

	#define MEM_COMMIT 0x1000
	#define MEM_RESERVE 0x2000
	#define MEM_DECOMMIT 0x4000
	#define MEM_RELEASE 0x8000

	#define HEAP_NO_SERIALIZE 0x1
	#define HEAP_ZERO_MEMORY 0x8

	#define PAGE_NOACCESS 0x01
	#define PAGE_READONLY 0x02
	#define PAGE_READWRITE 0x04
	#define PAGE_EXECUTE 0x10

	#define CSIDL_LOCAL_APPDATA 0x1C

	#define CREATE_NEW 1
	#define CREATE_ALWAYS 2
	#define OPEN_EXISTING 3
	#define OPEN_ALWAYS 4
	#define TRUNCATE_EXISTING 5

	#define FILE_BEGIN 0
	#define FILE_CURRENT 1
	#define FILE_END 2

	#define FILE_SHARE_READ 1
	#define FILE_SHARE_WRITE 2

	#define FILE_ATTRIBUTE_READONLY 0x01
	#define FILE_ATTRIBUTE_HIDDEN 0x02
	#define FILE_ATTRIBUTE_SYSTEM 0x04
	#define FILE_ATTRIBUTE_DIRECTORY 0x10
	#define FILE_ATTRIBUTE_NORMAL 0x80

	#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF)
	#define INVALID_SET_FILE_POINTER ((DWORD) - 1)
	#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)

	#define FILE_MAP_WRITE 2
	#define FILE_MAP_READ 4

	#define GENERIC_READ 0x80000000
	#define GENERIC_WRITE 0x40000000

	#define STD_OUTPUT_HANDLE ((DWORD) - 11)
	#define STD_ERROR_HANDLE ((DWORD) - 12)

	#define WS_MAXIMIZEBOX 0x010000
	#define WS_THICKFRAME 0x040000
	#define WS_CAPTION 0xC00000
	#define WS_OVERLAPPEDWINDOW 0xCF0000
	#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L

	#define WM_MOVE 0x003
	#define WM_SIZE 0x005
	#define WM_CLOSE 0x010
	#define WM_ACTIVATEAPP 0x01C
	#define WM_SETCURSOR 0x020
	#define WM_GETMINMAXINFO 0x024
	#define WM_KEYDOWN 0x100
	#define WM_KEYUP 0x101
	#define WM_CHAR 0x102
	#define WM_SYSKEYDOWN 0x104
	#define WM_SYSKEYUP 0x105
	#define WM_UNICHAR 0x109
	#define WM_MOUSEMOVE 0x200
	#define WM_LBUTTONDOWN 0x201
	#define WM_LBUTTONUP 0x202
	#define WM_RBUTTONDOWN 0x204
	#define WM_RBUTTONUP 0x205
	#define WM_MBUTTONDOWN 0x207
	#define WM_MBUTTONUP 0x208
	#define WM_MOUSEWHEEL 0x20A
	#define WM_MOUSEHWHEEL 0x020E
	#define WM_DEVICECHANGE 0x219

	#define CS_OWNDC 0x20

	#define CW_USEDEFAULT ((int)0x80000000)

	#define MK_LBUTTON 0x01
	#define MK_RBUTTON 0x02
	#define MK_MBUTTON 0x10

	#define MB_SYSTEMMODAL 0x1000
	#define MB_OK 0x00000000L
	#define MB_OKCANCEL 0x00000001L
	#define MB_ABORTRETRYIGNORE 0x00000002L
	#define MB_YESNOCANCEL 0x00000003L
	#define MB_YESNO 0x00000004L
	#define MB_RETRYCANCEL 0x00000005L
	#define MB_ICONHAND 0x00000010L
	#define MB_ICONQUESTION 0x00000020L
	#define MB_ICONEXCLAMATION 0x00000030L
	#define MB_ICONASTERISK 0x00000040L
	#define MB_ICONERROR 0x0010
	#define MB_ICONINFORMATION MB_ICONASTERISK

	#define SPI_GETWORKAREA             0x0030

	#define KF_ALTDOWN 0x2000

	#define HTNOWHERE 0
	#define HTCLIENT 1
	#define HTCAPTION 2

	#define SM_CXSCREEN 0
	#define SM_CYSCREEN 1

	#define SW_NORMAL 1
	#define SW_MAXIMIZE 3

	#define SWP_NOSIZE 0x0001
	#define SWP_NOMOVE 0x0002
	#define SWP_NOZORDER 0x0004
	#define SWP_NOREDRAW 0x0008
	#define SWP_NOACTIVATE 0x0010
	#define SWP_FRAMECHANGED 0x0020
	#define SWP_SHOWWINDOW 0x0040
	#define SWP_HIDEWINDOW 0x0080
	#define SWP_NOCOPYBITS 0x0100
	#define SWP_NOOWNERZORDER 0x0200
	#define SWP_NOSENDCHANGING 0x0400

	#define MONITOR_DEFAULTTOPRIMARY 1
	#define MONITOR_DEFAULTTONEAREST 2

	#define GWL_STYLE (-16)
	#define CP_UTF8 65001
	#define MAX_PATH 260
	#define PM_REMOVE 1
	#define ERROR_SUCCESS 0

	#define CF_TEXT 1
	#define CF_BITMAP 2

	#define GMEM_FIXED 0x0000
	#define GMEM_MOVEABLE 0x0002
	#define GMEM_NOCOMPACT 0x0010
	#define GMEM_NODISCARD 0x0020
	#define GMEM_ZEROINIT 0x0040
	#define GMEM_MODIFY 0x0080
	#define GMEM_DISCARDABLE 0x0100
	#define GMEM_NOT_BANKED 0x1000
	#define GMEM_SHARE 0x2000
	#define GMEM_DDESHARE 0x2000
	#define GMEM_NOTIFY 0x4000
	#define GMEM_LOWER GMEM_NOT_BANKED
	#define GMEM_VALID_FLAGS 0x7F72
	#define GMEM_INVALID_HANDLE 0x8000

	#define INFINITE 0xFFFFFFFF
	#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR) - 1)

	using HANDLE = void *;
	using HBRUSH = void *;
	using HCURSOR = void *;
	using HICON = void *;
	using HINSTANCE = void *;
	using HMENU = void *;
	using HMONITOR = void *;
	using HWND = void *;
	using HDC = void *;
	using HGLRC = void *;
	using HMODULE = void *;

	#if PIPELINE_ARCHITECTURE_X64 || PIPELINE_ARCHITECTURE_ARM64
		using UINT_PTR = unsigned long long;
		using LONG_PTR = long long;
	#else
		static_assert(false, "Unsupported architecture!");
		using UINT_PTR = unsigned int;
		using LONG_PTR = int;
	#endif

	using VOID = void;
	using PVOID = void *;
	using LPVOID = void *;
	using CHAR = char;
	using PCHAR = CHAR *;
	using BYTE = unsigned char;
	using UINT8 = unsigned char;
	using UINT32 = unsigned int;
	using WORD = unsigned short;
	using ATOM = unsigned short;
	using SHORT = short;
	using BOOL = int;
	using INT = int;
	using UINT = unsigned int;
	using LONG = int;
	using ULONG = unsigned int;
	using ULONG64 = unsigned long long;
	static_assert(sizeof(ULONG64) == 8, "ULONG64 not 8 bytes");
	using DWORD = unsigned int;
	using DWORD64 = unsigned long long;
	static_assert(sizeof(DWORD64) == 8, "DWORD64 not 8 bytes");
	using MMRESULT = unsigned int;
	using FLOAT = float;
	using LONGLONG = long long;
	using ULONGLONG = long long;
	using SIZE_T = UINT_PTR;
	using WCHAR = wchar_t;
	using LPSTR = char *;
	using LPWSTR = WCHAR *;
	using PCSTR = const char *;
	using LPCSTR = const char *;
	using LPCWSTR = const WCHAR *;
	using WPARAM = UINT_PTR;
	using LPARAM = LONG_PTR;
	using LRESULT = LONG_PTR;
	using HRESULT = int;
	using PROC = int(CALLBACK *)();
	using WNDPROC = LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM);
	using LPTHREAD_START_ROUTINE = DWORD(CALLBACK *)(void *);
	using HGLOBAL = HANDLE;

	struct RECT
	{
		LONG left;
		LONG top;
		LONG right;
		LONG bottom;
	};

	struct SMALL_RECT
	{
		SHORT Left;
		SHORT Top;
		SHORT Right;
		SHORT Bottom;
	};

	struct POINT
	{
		LONG x;
		LONG y;
	};

	struct COORD
	{
		SHORT X;
		SHORT Y;
	};

	struct WNDCLASSW
	{
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		HINSTANCE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCWSTR lpszMenuName;
		LPCWSTR lpszClassName;
	};

	struct MSG
	{
		HWND hwnd;
		UINT message;
		WPARAM wParam;
		LPARAM lParam;
		DWORD time;
		POINT pt;
	};

	struct MONITORINFO
	{
		DWORD cbSize;
		RECT rcMonitor;
		RECT rcWork;
		DWORD dwFlags;
	};

	struct FILETIME
	{
		DWORD dwLowDateTime;
		DWORD dwHighDateTime;
	};

	struct SYSTEMTIME
	{
		WORD wYear;
		WORD wMonth;
		WORD wDayOfWeek;
		WORD wDay;
		WORD wHour;
		WORD wMinute;
		WORD wSecond;
		WORD wMilliseconds;
	};

	// TODO: Unicode Support
	struct WIN32_FIND_DATAA
	{
		DWORD dwFileAttributes;
		FILETIME ftCreationTime;
		FILETIME ftLastAccessTime;
		FILETIME ftLastWriteTime;
		DWORD nFileSizeHigh;
		DWORD nFileSizeLow;
		DWORD dwReserved0;
		DWORD dwReserved1;
		CHAR cFileName[MAX_PATH];
		CHAR cAlternateFileName[14];
	};

	struct CONSOLE_SCREEN_BUFFER_INFO
	{
		COORD dwSize;
		COORD dwCursorPosition;
		WORD wAttributes;
		SMALL_RECT srWindow;
		COORD dwMaximumWindowSize;
	};

	struct PROCESS_INFORMATION
	{
		HANDLE hProcess;
		HANDLE hThread;
		DWORD dwProcessId;
		DWORD dwThreadId;
	};

	struct STARTUPINFOA
	{
		DWORD cb;
		LPSTR lpReserved;
		LPSTR lpDesktop;
		LPSTR lpTitle;
		DWORD dwX;
		DWORD dwY;
		DWORD dwXSize;
		DWORD dwYSize;
		DWORD dwXCountChars;
		DWORD dwYCountChars;
		DWORD dwFillAttribute;
		DWORD dwFlags;
		WORD wShowWindow;
		WORD cbReserved2;
		BYTE *lpReserved2;
		HANDLE hStdInput;
		HANDLE hStdOutput;
		HANDLE hStdError;
	};

	struct MINMAXINFO
	{
		POINT ptReserved;
		POINT ptMaxSize;
		POINT ptMaxPosition;
		POINT ptMinTrackSize;
		POINT ptMaxTrackSize;
	};

	struct LIST_ENTRY
	{
		struct LIST_ENTRY *Flink;
		struct LIST_ENTRY *Blink;
	};

	struct CRITICAL_SECTION_DEBUG
	{
		WORD Type;
		WORD CreatorBackTraceIndex;
		struct CRITICAL_SECTION *CriticalSection;
		LIST_ENTRY ProcessLocksList;
		DWORD EntryCount;
		DWORD ContentionCount;
		DWORD Flags;
		WORD CreatorBackTraceIndexHigh;
		WORD SpareWORD;
	};

	struct CRITICAL_SECTION
	{
		CRITICAL_SECTION_DEBUG DebugInfo;
		LONG LockCount;
		LONG RecursionCount;
		HANDLE OwningThread;
		HANDLE LockSemaphore;
		ULONG *SpinCount;
	};

	struct CONDITION_VARIABLE
	{
		void *Ptr;
	};

	union LARGE_INTEGER
	{
		struct
		{
			DWORD LowPart;
			LONG HighPart;
		};
		LONGLONG QuadPart;
	};

	union ULARGE_INTEGER
	{
		struct
		{
			DWORD LowPart;
			LONG HighPart;
		};
		ULONGLONG QuadPart;
	};

	// intrin.h
#if defined( _MSC_VER ) && !defined( _INTRIN_DEFINED )
	extern "C" short __cdecl _InterlockedCompareExchange16( short volatile *, short, short );
	#pragma intrinsic( _InterlockedCompareExchange16 )
	extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );
	#pragma intrinsic( _InterlockedCompareExchange )
	extern "C" long long __cdecl _InterlockedCompareExchange64( long long volatile *, long long, long long );
	#pragma intrinsic( _InterlockedCompareExchange64 )
	extern "C" long __cdecl _InterlockedExchange( long volatile *, long );
	#pragma intrinsic( _InterlockedExchange )
	extern "C" long long __cdecl _InterlockedExchange64( long long volatile *, long long );
	#pragma intrinsic( _InterlockedExchange64 )
#endif

	// kernel32.dll
	extern "C" DLL_IMPORT BOOL STD_CALL CloseHandle( HANDLE );
	extern "C" DLL_IMPORT BOOL STD_CALL CreateDirectoryA( LPCSTR, struct SECURITY_ATTRIBUTES * );
	extern "C" DLL_IMPORT HANDLE STD_CALL CreateEventW( struct SECURITY_ATTRIBUTES *, BOOL, BOOL, LPCWSTR );
	extern "C" DLL_IMPORT HANDLE STD_CALL CreateFileA( LPCSTR, DWORD, DWORD, struct SECURITY_ATTRIBUTES *,
		DWORD, DWORD, HANDLE );
	extern "C" DLL_IMPORT HANDLE STD_CALL CreateFileMappingW(HANDLE, struct SECURITY_ATTRIBUTES *, DWORD,
		DWORD, DWORD, LPCWSTR );
	extern "C" DLL_IMPORT BOOL STD_CALL CreateProcessA( LPCSTR, LPSTR, struct SECURITY_ATTRIBUTES *,
		struct SECURITY_ATTRIBUTES *, BOOL, DWORD, void *, LPCSTR,
		STARTUPINFOA *, PROCESS_INFORMATION * );
	extern "C" DLL_IMPORT HANDLE STD_CALL CreateThread( struct SECURITY_ATTRIBUTES *, SIZE_T,
		LPTHREAD_START_ROUTINE, void *, DWORD, DWORD * );
	extern "C" DLL_IMPORT BOOL STD_CALL MoveFileA( LPCSTR, LPCSTR );
	extern "C" DLL_IMPORT BOOL STD_CALL DeleteFileA( LPCSTR );
	extern "C" DLL_IMPORT NO_RETURN void STD_CALL ExitProcess( UINT );
	extern "C" DLL_IMPORT BOOL STD_CALL FindClose( HANDLE );
	extern "C" DLL_IMPORT HANDLE STD_CALL FindFirstFileA( LPCSTR, WIN32_FIND_DATAA * );
	extern "C" DLL_IMPORT BOOL STD_CALL FindNextFileA( HANDLE, WIN32_FIND_DATAA * );
	extern "C" DLL_IMPORT BOOL STD_CALL FreeLibrary( HMODULE );
	extern "C" DLL_IMPORT LPSTR STD_CALL GetCommandLineA();
	extern "C" DLL_IMPORT BOOL STD_CALL GetConsoleScreenBufferInfo( HANDLE, CONSOLE_SCREEN_BUFFER_INFO * );
	extern "C" DLL_IMPORT DWORD STD_CALL GetFileAttributesA( LPCSTR );
	extern "C" DLL_IMPORT DWORD STD_CALL GetFileSize( HANDLE, DWORD * );
	extern "C" DLL_IMPORT BOOL STD_CALL GetFileSizeEx( HANDLE, LARGE_INTEGER * );
	extern "C" DLL_IMPORT BOOL STD_CALL GetFileTime( HANDLE, FILETIME *, FILETIME *, FILETIME * );
	extern "C" DLL_IMPORT BOOL STD_CALL FileTimeToLocalFileTime( FILETIME *, FILETIME * );
	extern "C" DLL_IMPORT BOOL STD_CALL FileTimeToSystemTime( FILETIME *, SYSTEMTIME * );
	extern "C" DLL_IMPORT DWORD STD_CALL GetFullPathNameA( LPCSTR, DWORD, LPSTR, LPSTR * );
	extern "C" DLL_IMPORT DWORD STD_CALL GetModuleFileNameA( HMODULE, LPSTR, DWORD );
	extern "C" DLL_IMPORT DWORD STD_CALL GetModuleFileNameW( HMODULE, LPWSTR, DWORD );
	extern "C" DLL_IMPORT HMODULE STD_CALL GetModuleHandleW( LPCWSTR );
	extern "C" DLL_IMPORT void *STD_CALL GetProcAddress( HMODULE, LPCSTR );
	extern "C" DLL_IMPORT HANDLE STD_CALL GetProcessHeap();
	extern "C" DLL_IMPORT HANDLE STD_CALL GetStdHandle( DWORD );
	extern "C" DLL_IMPORT MALLOC_LIKE void *STD_CALL HeapAlloc( HANDLE, DWORD, SIZE_T );
	extern "C" DLL_IMPORT BOOL STD_CALL HeapFree( HANDLE, DWORD, void * );
	extern "C" DLL_IMPORT void *STD_CALL HeapReAlloc( HANDLE, DWORD, void *, SIZE_T );
	extern "C" DLL_IMPORT HMODULE STD_CALL LoadLibraryA( LPCSTR );
	extern "C" DLL_IMPORT HMODULE STD_CALL GetModuleHandleA( LPCSTR );
	extern "C" DLL_IMPORT void *STD_CALL MapViewOfFile( HANDLE, DWORD, DWORD, DWORD, SIZE_T );
	extern "C" DLL_IMPORT int STD_CALL MultiByteToWideChar( UINT, DWORD, LPCSTR, int, LPWSTR, int );
	extern "C" DLL_IMPORT BOOL STD_CALL QueryPerformanceCounter( LARGE_INTEGER * );
	extern "C" DLL_IMPORT BOOL STD_CALL QueryPerformanceFrequency( LARGE_INTEGER * );
	extern "C" DLL_IMPORT BOOL STD_CALL ReadFile( HANDLE, void *, DWORD, DWORD *, struct OVERLAPPED * );
	extern "C" DLL_IMPORT BOOL STD_CALL SetConsoleTextAttribute( HANDLE, WORD );
	extern "C" DLL_IMPORT DWORD STD_CALL SetFilePointer( HANDLE, LONG, LONG *, DWORD );
	extern "C" DLL_IMPORT void STD_CALL Sleep( DWORD );
	extern "C" DLL_IMPORT BOOL STD_CALL SwitchToThread();
	extern "C" DLL_IMPORT BOOL STD_CALL RemoveDirectoryA( LPCSTR );
	extern "C" DLL_IMPORT BOOL STD_CALL UnmapViewOfFile( const void * );
	extern "C" DLL_IMPORT void *STD_CALL VirtualAlloc( void *, SIZE_T, DWORD, DWORD );
	extern "C" DLL_IMPORT BOOL STD_CALL VirtualFree( void *, SIZE_T, DWORD );
	extern "C" DLL_IMPORT BOOL STD_CALL VirtualProtect( void *, SIZE_T, DWORD, DWORD * );
	extern "C" DLL_IMPORT DWORD STD_CALL WaitForMultipleObjects( DWORD, const HANDLE *, BOOL, DWORD );
	extern "C" DLL_IMPORT DWORD STD_CALL WaitForSingleObject( HANDLE, DWORD );
	extern "C" DLL_IMPORT int STD_CALL WideCharToMultiByte( UINT, DWORD, LPCWSTR, int, LPSTR, int, LPSTR, BOOL * );
	extern "C" DLL_IMPORT BOOL STD_CALL WriteConsoleA( HANDLE, const void *, DWORD, DWORD *, void * );
	extern "C" DLL_IMPORT BOOL STD_CALL WriteFile( HANDLE, const void *, DWORD, DWORD *, struct OVERLAPPED * );
	extern "C" DLL_IMPORT BOOL STD_CALL WriteFile( HANDLE, const void *, DWORD, DWORD *, struct OVERLAPPED * );
	extern "C" DLL_IMPORT HANDLE STD_CALL CreateMutexA( struct SECURITY_ATTRIBUTES *, BOOL, LPCSTR );
	extern "C" DLL_IMPORT HANDLE STD_CALL CreateMutexW( struct SECURITY_ATTRIBUTES *, BOOL, LPCWSTR );
	extern "C" DLL_IMPORT BOOL STD_CALL ReleaseMutex( HANDLE );
	extern "C" DLL_IMPORT void STD_CALL InitializeCriticalSection( CRITICAL_SECTION * );
	extern "C" DLL_IMPORT void STD_CALL DeleteCriticalSection( CRITICAL_SECTION * );
	extern "C" DLL_IMPORT void STD_CALL EnterCriticalSection( CRITICAL_SECTION * );
	extern "C" DLL_IMPORT void STD_CALL LeaveCriticalSection( CRITICAL_SECTION * );
	extern "C" DLL_IMPORT void STD_CALL InitializeConditionVariable( CONDITION_VARIABLE * );
	extern "C" DLL_IMPORT BOOL STD_CALL SleepConditionVariableCS( CONDITION_VARIABLE *, CRITICAL_SECTION *, DWORD );
	extern "C" DLL_IMPORT void STD_CALL WakeConditionVariable( CONDITION_VARIABLE * );
	extern "C" DLL_IMPORT void STD_CALL WakeAllConditionVariable( CONDITION_VARIABLE * );
	extern "C" DLL_IMPORT DWORD STD_CALL GetCurrentThreadId();
	extern "C" DLL_IMPORT HANDLE STD_CALL GetCurrentProcess();
	extern "C" DLL_IMPORT DWORD STD_CALL GetCurrentProcessId();

	// user32.dll
	extern "C" DLL_IMPORT BOOL STD_CALL AdjustWindowRect( RECT *, DWORD, BOOL );
	extern "C" DLL_IMPORT BOOL STD_CALL ClientToScreen( HWND, POINT * );
	extern "C" DLL_IMPORT BOOL STD_CALL BringWindowToTop( HWND );
	extern "C" DLL_IMPORT HWND STD_CALL CreateWindowExW( DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int,
		HWND, HMENU, HINSTANCE, void * );
	extern "C" DLL_IMPORT LRESULT STD_CALL DefWindowProcW( HWND, UINT, WPARAM, LPARAM );
	extern "C" DLL_IMPORT LRESULT STD_CALL DispatchMessageW( const MSG * );
	extern "C" DLL_IMPORT HDC STD_CALL GetDC( HWND );
	extern "C" DLL_IMPORT BOOL STD_CALL GetMonitorInfoW( HMONITOR, MONITORINFO * );
	extern "C" DLL_IMPORT int STD_CALL GetSystemMetrics( int );
	extern "C" DLL_IMPORT HCURSOR STD_CALL LoadCursorW( HINSTANCE, LPCWSTR );
	extern "C" DLL_IMPORT int STD_CALL MessageBoxA( HWND, LPCSTR, LPCSTR, UINT );
	extern "C" DLL_IMPORT int STD_CALL MessageBoxW( HWND, LPCWSTR, LPCWSTR, UINT );
	extern "C" DLL_IMPORT HMONITOR STD_CALL MonitorFromWindow( HWND, DWORD );
	extern "C" DLL_IMPORT BOOL STD_CALL PeekMessageW( MSG *, HWND, UINT, UINT, UINT );
	extern "C" DLL_IMPORT ATOM STD_CALL RegisterClassW( const WNDCLASSW * );
	extern "C" DLL_IMPORT BOOL STD_CALL ReleaseCapture();
	extern "C" DLL_IMPORT HWND STD_CALL SetCapture( HWND );
	extern "C" DLL_IMPORT HCURSOR STD_CALL SetCursor( HCURSOR );
	extern "C" DLL_IMPORT BOOL STD_CALL SetCursorPos( int, int );
	extern "C" DLL_IMPORT BOOL STD_CALL SetWindowPos( HWND, HWND, int, int, int, int, UINT );
	extern "C" DLL_IMPORT BOOL STD_CALL SetWindowTextW( HWND, LPCWSTR );
	extern "C" DLL_IMPORT BOOL STD_CALL ShowWindow( HWND, int );
	extern "C" DLL_IMPORT BOOL STD_CALL TranslateMessage( const MSG * );

	#if PIPELINE_ARCHITECTURE_X64 || PIPELINE_ARCHITECTURE_ARM64
		extern "C" DLL_IMPORT LONG_PTR STD_CALL SetWindowLongPtrW( HWND, int, LONG_PTR );
	#else
		// extern "C" DLL_IMPORT  LONG STD_CALL SetWindowLongW( HWND, int, LONG ); (32-bit)
		// #define SetWindowLongPtrW SetWindowLongW
	#endif

	// shell32.dll
	extern "C" DLL_IMPORT HICON STD_CALL ExtractIconW( HINSTANCE, LPCWSTR, UINT );
	extern "C" DLL_IMPORT HRESULT STD_CALL SHGetFolderPathA( HWND, int, HANDLE, DWORD, LPSTR );

	// winmm.dll
	extern "C" DLL_IMPORT MMRESULT STD_CALL timeBeginPeriod( UINT );

	// ntdll.dll
	extern "C" DLL_IMPORT WORD STD_CALL RtlCaptureStackBackTrace( DWORD, DWORD, PVOID *, DWORD * );

	// Winuser.h
	extern "C" DLL_IMPORT BOOL STD_CALL OpenClipboard( HWND );
	extern "C" DLL_IMPORT BOOL STD_CALL CloseClipboard( VOID );
	extern "C" DLL_IMPORT BOOL STD_CALL EmptyClipboard( VOID );
	extern "C" DLL_IMPORT BOOL STD_CALL IsClipboardFormatAvailable( UINT );
	extern "C" DLL_IMPORT HANDLE STD_CALL GetClipboardData( UINT );
	extern "C" DLL_IMPORT HANDLE STD_CALL SetClipboardData( UINT, HANDLE );
	extern "C" DLL_IMPORT BOOL STD_CALL SystemParametersInfoA( UINT, UINT, PVOID, UINT );
	extern "C" DLL_IMPORT BOOL STD_CALL SystemParametersInfoW( UINT, UINT, PVOID, UINT );
	extern "C" DLL_IMPORT int STD_CALL ReleaseDC( HWND, HDC );
	extern "C" DLL_IMPORT BOOL STD_CALL DestroyWindow( HWND );
	extern "C" DLL_IMPORT BOOL STD_CALL UnregisterClassW( LPCWSTR, HINSTANCE );
	extern "C" DLL_IMPORT int STD_CALL ShowCursor( BOOL );

	// WinBase.h
	extern "C" int WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
	extern "C" DLL_IMPORT LPVOID STD_CALL GlobalLock( HGLOBAL );
	extern "C" DLL_IMPORT BOOL STD_CALL GlobalUnlock( HGLOBAL );
	extern "C" DLL_IMPORT HGLOBAL STD_CALL GlobalFree( HGLOBAL );
	extern "C" DLL_IMPORT HGLOBAL STD_CALL GlobalAlloc( UINT, SIZE_T );

	// conesoleapi.h
	#define ATTACH_PARENT_PROCESS ((DWORD)-1)

	extern "C" DLL_IMPORT BOOL STD_CALL GetConsoleMode( HANDLE, DWORD * );
	extern "C" DLL_IMPORT BOOL STD_CALL SetConsoleMode( HANDLE, DWORD );
	extern "C" DLL_IMPORT BOOL STD_CALL AllocConsole( VOID );
	extern "C" DLL_IMPORT BOOL STD_CALL FreeConsole( VOID );
	extern "C" DLL_IMPORT BOOL STD_CALL AttachConsole( DWORD );

	// processthreadsapi.h
	extern "C" DLL_IMPORT BOOL STD_CALL GetExitCodeProcess( HANDLE, DWORD * );

	// synchapi.h
	extern "C" DLL_IMPORT DWORD STD_CALL WaitForSingleObject( HANDLE, DWORD );

	// shlobj.h
	extern "C" DLL_IMPORT HRESULT STD_CALL SHGetKnownFolderPath( const struct GUID &, DWORD, HANDLE, WCHAR ** );

	// com.h
	#include <vendor/com.hpp>
#endif