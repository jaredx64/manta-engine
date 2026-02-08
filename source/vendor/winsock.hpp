#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <winsock2.h>
		#include <ws2tcpip.h>
	#include <vendor/conflicts.hpp>
#else

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// winnt.h

	using VOID = void;
	using PVOID = void *;
	using LPVOID = void *;
	using BYTE = unsigned char;
	using WORD = unsigned short;
	using DWORD = unsigned int;
	using BOOL = int;
	using INT = int;
	using LONG = int;
	using ULONG = unsigned int;
	using PULONG = unsigned int *;
	using USHORT = unsigned short;
	using PUSHORT = unsigned short *;
	using ADDRESS_FAMILY = unsigned short *;
	using CHAR = char;
	using PCHAR = char *;
	using PSTR = char *;
	using UCHAR = unsigned char;
	using PUCHAR = unsigned char *;
	using PCSTR = const char *;
	using PSZ = char *;
	using HANDLE = void *;
#if defined( _WIN64 ) || defined( __x86_64__ ) || defined( __aarch64__ ) || defined( _M_X64 ) || defined( _M_ARM64 )
	using INT_PTR = long long;
	using UINT_PTR = unsigned long long;
#else
	using INT_PTR = long;
	using UINT_PTR = unsigned long;
#endif
    using PINT_PTR = INT_PTR *;
    using PUINT_PTR = UINT_PTR *;
	using LONG_PTR = INT_PTR;
    using PLONG_PTR = INT_PTR *;
    using ULONG_PTR = UINT_PTR;
	using PULONG_PTR = UINT_PTR *;
	using DWORD_PTR = UINT_PTR;
	using PDWORD_PTR = UINT_PTR *;
	using SOCKET = UINT_PTR;

	using u_char = unsigned char;
	using u_short = unsigned short;
	using u_int = unsigned int;
	using u_long = unsigned long;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// inaddr.h

	struct in_addr
	{
		union
		{
			struct { UCHAR s_b1, s_b2, s_b3, s_b4; } S_un_b;
			struct { USHORT s_w1, s_w2; } S_un_w;
			ULONG S_addr;
		} S_un;
		#define s_addr  S_un.S_addr
		#define s_host  S_un.S_un_b.s_b2
		#define s_net   S_un.S_un_b.s_b1
		#define s_imp   S_un.S_un_w.s_w2
		#define s_impno S_un.S_un_b.s_b4
		#define s_lh    S_un.S_un_b.s_b3
	};
	typedef in_addr IN_ADDR;
	typedef in_addr *PIN_ADDR;
	typedef in_addr *LPIN_ADDR;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// winerror.h

	#define WSAEWOULDBLOCK 10035L

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ws2def.h

	#define AF_UNSPEC 0
	#define AF_UNIX 1
	#define AF_INET 2
	#define AF_IMPLINK 3
	#define AF_PUP 4
	#define AF_CHAOS 5
	#define AF_NS 6
	#define AF_IPX AF_NS
	#define AF_ISO 7
	#define AF_OSI AF_ISO
	#define AF_ECMA 8
	#define AF_DATAKIT 9
	#define AF_CCITT 10
	#define AF_SNA 11
	#define AF_DECnet 12
	#define AF_DLI 13
	#define AF_LAT 14
	#define AF_HYLINK 15
	#define AF_APPLETALK 16
	#define AF_NETBIOS 17
	#define AF_VOICEVIEW 18
	#define AF_FIREFOX 19
	#define AF_UNKNOWN1 20
	#define AF_BAN 21
	#define AF_ATM 22
	#define AF_INET6 23
	#define AF_CLUSTER 24
	#define AF_12844 25
	#define AF_IRDA 26
	#define AF_NETDES 28

	#define INADDR_ANY (ULONG)0x00000000
	#define INADDR_LOOPBACK 0x7f000001
	#define INADDR_BROADCAST (ULONG)0xffffffff
	#define INADDR_NONE 0xffffffff

	#define TCP_NODELAY 0x0001

	enum
	{
		IPPROTO_TCP = 6,
		IPPROTO_UDP = 17,
	};

	struct sockaddr_in
	{
		short sin_family;
		USHORT sin_port;
		IN_ADDR sin_addr;
		CHAR sin_zero[8];
	};

	struct sockaddr
	{
		u_short sa_family;
		CHAR sa_data[14];
	};
	typedef sockaddr SOCKADDR;

	struct addrinfo
	{
		int ai_flags;
		int ai_family;
		int ai_socktype;
		int ai_protocol;
		size_t ai_addrlen;
		char *ai_canonname;
		struct sockaddr *ai_addr;
		struct addrinfo *ai_next;
	};
	typedef addrinfo ADDRINFOA;
	typedef addrinfo *PADDRINFOA;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// winsock2.h

	#define SOCK_STREAM 1
	#define SOCK_DGRAM 2
	#define SOCK_RAW 3
	#define SOCK_RDM 4
	#define SOCK_SEQPACKET 5

	#define SOMAXCONN 0x7fffffff

	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR (-1)

	#define WSA_INVALID_EVENT (WSAEVENT)(0)
	#define WSA_WAIT_FAILED (DWORD)(0xFFFFFFFF)
	#define WSA_WAIT_TIMEOUT (258L)

	#define SD_RECEIVE 0x00
	#define SD_SEND 0x01
	#define SD_BOTH 0x02

	#define FD_READ_BIT 0
	#define FD_READ (1 << FD_READ_BIT)
	#define FD_WRITE_BIT 1
	#define FD_WRITE (1 << FD_WRITE_BIT)
	#define FD_OOB_BIT 2
	#define FD_OOB (1 << FD_OOB_BIT)
	#define FD_ACCEPT_BIT 3
	#define FD_ACCEPT (1 << FD_ACCEPT_BIT)
	#define FD_CONNECT_BIT 4
	#define FD_CONNECT (1 << FD_CONNECT_BIT)
	#define FD_CLOSE_BIT 5
	#define FD_CLOSE (1 << FD_CLOSE_BIT)
	#define FD_QOS_BIT 6
	#define FD_QOS (1 << FD_QOS_BIT)
	#define FD_GROUP_QOS_BIT 7
	#define FD_GROUP_QOS (1 << FD_GROUP_QOS_BIT)
	#define FD_ROUTING_INTERFACE_CHANGE_BIT 8
	#define FD_ROUTING_INTERFACE_CHANGE (1 << FD_ROUTING_INTERFACE_CHANGE_BIT)
	#define FD_ADDRESS_LIST_CHANGE_BIT 9
	#define FD_ADDRESS_LIST_CHANGE (1 << FD_ADDRESS_LIST_CHANGE_BIT)
	#define FD_MAX_EVENTS 10
	#define FD_ALL_EVENTS ((1 << FD_MAX_EVENTS) - 1)

	#define MAKEWORD(a, b) ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
	#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
	#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
	#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
	#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
	#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

	#define IOCPARM_MASK 0x7f
	#define IOC_VOID 0x20000000
	#define IOC_OUT 0x40000000
	#define IOC_IN 0x80000000
	#define IOC_INOUT (IOC_IN | IOC_OUT)

	#define _IO(x,y) (IOC_VOID|((x)<<8)|(y))
	#define _IOR(x,y,t) (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
	#define _IOW(x,y,t) (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

	#define FIONREAD _IOR('f', 127, u_long)
	#define FIONBIO _IOW('f', 126, u_long)
	#define FIOASYNC _IOW('f', 125, u_long)

	#define SIOCSHIWAT _IOW('s', 0, u_long)
	#define SIOCGHIWAT _IOR('s', 1, u_long)
	#define SIOCSLOWAT _IOW('s', 2, u_long)
	#define SIOCGLOWAT _IOR('s', 3, u_long)
	#define SIOCATMARK _IOR('s', 7, u_long)

	#define WSADESCRIPTION_LEN 256
	#define WSASYS_STATUS_LEN 128

	#define WSAEVENT HANDLE

	struct WSAData
	{
		WORD wVersion;
		WORD wHighVersion;
		unsigned short iMaxSockets;
		unsigned short iMaxUdpDg;
		char *lpVendorInfo;
		char szDescription[WSADESCRIPTION_LEN + 1];
		char szSystemStatus[WSASYS_STATUS_LEN + 1];
	};
	typedef WSAData WSADATA;
	typedef WSAData *PWSADATA;
	typedef WSAData *LPWSADATA;

	struct WSANETWORKEVENTS
	{
		long lNetworkEvents;
		int iErrorCode[FD_MAX_EVENTS];
	};
	typedef WSANETWORKEVENTS *LPWSANETWORKEVENTS;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ws2tcpip.h

	extern "C" DLL_IMPORT INT STD_CALL getaddrinfo( PCSTR, PCSTR, const ADDRINFOA *, PADDRINFOA * );
	extern "C" DLL_IMPORT VOID STD_CALL freeaddrinfo( PADDRINFOA );
	extern "C" DLL_IMPORT INT STD_CALL inet_pton( INT, PCSTR, PVOID pAddrBuf );
	extern "C" DLL_IMPORT PCSTR STD_CALL inet_ntop( INT, const VOID *, PSTR, size_t );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// winsock2.h

	extern "C" DLL_IMPORT SOCKET STD_CALL socket( int, int, int );
	extern "C" DLL_IMPORT SOCKET STD_CALL accept( SOCKET, struct sockaddr *, int * );
	extern "C" DLL_IMPORT int STD_CALL closesocket( SOCKET );
	extern "C" DLL_IMPORT int STD_CALL ioctlsocket( SOCKET, long, u_long * );
	extern "C" DLL_IMPORT int STD_CALL setsockopt( SOCKET, int, int, const char *, int );
	extern "C" DLL_IMPORT int STD_CALL connect( SOCKET, const struct sockaddr *, int );
	extern "C" DLL_IMPORT int STD_CALL listen( SOCKET, int );
	extern "C" DLL_IMPORT int STD_CALL send( SOCKET, const char *, int, int );
	extern "C" DLL_IMPORT int STD_CALL sendto( SOCKET, const char *, int, int, const struct sockaddr *, int );
	extern "C" DLL_IMPORT int STD_CALL recv( SOCKET, char *, int, int );
	extern "C" DLL_IMPORT int STD_CALL recvfrom( SOCKET, char *, int, int, struct sockaddr *, int * );
	extern "C" DLL_IMPORT int STD_CALL bind( SOCKET, const struct sockaddr *, int );
	extern "C" DLL_IMPORT int STD_CALL shutdown( SOCKET, int );
	extern "C" DLL_IMPORT u_long STD_CALL htonl( u_long );
	extern "C" DLL_IMPORT u_short STD_CALL htons( u_short );
	extern "C" DLL_IMPORT u_short STD_CALL ntohs( u_short );

	extern "C" DLL_IMPORT int STD_CALL WSAStartup( WORD wVersionRequested, LPWSADATA lpWSAData );
	extern "C" DLL_IMPORT int STD_CALL WSACleanup();
	extern "C" DLL_IMPORT WSAEVENT STD_CALL WSACreateEvent();
	extern "C" DLL_IMPORT int STD_CALL WSAEventSelect( SOCKET, WSAEVENT, long );
	extern "C" DLL_IMPORT DWORD STD_CALL WSAWaitForMultipleEvents( DWORD, const WSAEVENT *, BOOL, DWORD, BOOL );
	extern "C" DLL_IMPORT int STD_CALL WSAEnumNetworkEvents( SOCKET, WSAEVENT, LPWSANETWORKEVENTS );
	extern "C" DLL_IMPORT BOOL STD_CALL WSACloseEvent( WSAEVENT );
	extern "C" DLL_IMPORT int STD_CALL WSAGetLastError();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif