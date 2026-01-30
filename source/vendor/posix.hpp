#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <unistd.h>
		#include <fcntl.h>
		#include <dirent.h>
		#include <limits.h>
		#include <time.h>
		#include <libgen.h>
		#include <sched.h>
		#include <sys/stat.h>
		#include <sys/mman.h>
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <sys/wait.h>
		#include <netinet/in.h>
		#include <netinet/tcp.h>
		#include <arpa/inet.h>
		#include <netdb.h>
		#include <errno.h>
	#include <vendor/conflicts.hpp>
#else
	#include <core/types.hpp>

	// bits/types.h
	typedef signed char __int8_t;
	typedef unsigned char __uint8_t;
	typedef signed short int __int16_t;
	typedef unsigned short int __uint16_t;
	typedef signed int __int32_t;
	typedef unsigned int __uint32_t;
	typedef signed long int __int64_t;
	typedef unsigned long int __uint64_t;
	typedef __uint8_t uint8_t;
	typedef __uint16_t uint16_t;
	typedef __uint32_t uint32_t;
	//typedef __uint64_t uint64_t;
	typedef int __pid_t;
	typedef __pid_t pid_t;

	// errno-base.h
	#define	EPERM 1
	#define	ENOENT 2
	#define	ESRCH 3
	#define	EINTR 4
	#define	EIO	 5
	#define	ENXIO 6
	#define	E2BIG 7
	#define	ENOEXEC 8
	#define	EBADF 9
	#define	ECHILD 10
	#define	EAGAIN 11
	#define	ENOMEM 12
	#define	EACCES 13
	#define	EFAULT 14
	#define	ENOTBLK 15
	#define	EBUSY 16
	#define	EEXIST 17
	#define	EXDEV 18
	#define	ENODEV 19
	#define	ENOTDIR 20
	#define	EISDIR 21
	#define	EINVAL 22
	#define	ENFILE 23
	#define	EMFILE 24
	#define	ENOTTY 25
	#define	ETXTBSY 26
	#define	EFBIG 27
	#define	ENOSPC 28
	#define	ESPIPE 29
	#define	EROFS 30
	#define	EMLINK 31
	#define	EPIPE 32
	#define	EDOM 33
	#define	ERANGE 34

	// errno.h
	#define	EWOULDBLOCK	EAGAIN

	extern "C" int *__errno_location();
	#define errno (*__errno_location ())

	// fcntl.h
	extern "C" int fcntl( int , int, ... );
	extern "C" int open( const char *, int, ... );

	// fcntl-linux.h
	#define F_DUPFD 0
	#define F_GETFD 1
	#define F_SETFD 2
	#define F_GETFL 3
	#define F_SETFL 4

	#define O_RDONLY 0x0000
	#define O_WRONLY 0x0001
	#define O_CREAT 0x0040
	#define O_TRUNC 0x0200
	#define O_NONBLOCK 04000

	// sys/mman.h
	extern "C" void *mmap( void *, unsigned long, int, int, int, long );
	extern "C" int mprotect( void *, unsigned long, int );
	extern "C" int munmap( void *, unsigned long );

	// mman-linux.h
	#define PROT_NONE 0
	#define PROT_READ 1
	#define PROT_WRITE 2

	#define MAP_PRIVATE 0x02
	#define MAP_FAILED reinterpret_cast<void *>(-1)

	// struct_timespec.h
	struct timespec
	{
		long long tv_sec;
		long long tv_nsec;
	};

	// struct_tm.h
	struct tm
	{
		int tm_sec;
		int tm_min;
		int tm_hour;
		int tm_mday;
		int tm_mon;
		int tm_year;
		int tm_wday;
		int tm_yday;
		int tm_isdst;
	# ifdef	__USE_MISC
		long int tm_gmtoff;
		const char *tm_zone;
	# else
		long int __tm_gmtoff;
		const char *__tm_zone;
	# endif
	};

	// time.h
	#define CLOCK_MONOTONIC 1
	#define time_t long long int

	extern "C" int clock_gettime( int, timespec * );
	extern "C" struct tm *localtime_r( const time_t *, struct tm * );

	// struct_stat.h
	struct stat
	{
		unsigned long int st_dev;
		unsigned long int st_ino;
		unsigned long int st_nlink;
		unsigned int st_mode;
		unsigned int st_uid;
		unsigned int st_gid;
		int __pad0;
		unsigned long int st_rdev;
		long int st_size;
		long int st_blksize;
		long int st_blocks;
		struct timespec st_atim;
		struct timespec st_mtim;
		struct timespec st_ctim;
		long int __glibc_reserved[3];
	};

	#define st_atime st_atim.tv_sec
	#define st_mtime st_mtim.tv_sec
	#define st_ctime st_ctim.tv_sec

	// sys/stat.h
	#define S_IRUSR 00400
	#define S_IWUSR 00200
	#define S_IXUSR 00100
	#define S_IRGRP 00040
	#define S_IWGRP 00020
	#define S_IXGRP 00010
	#define S_IROTH 00004
	#define S_IWOTH 00002
	#define S_IXOTH 00001

	#define S_ISDIR(mode) ((((mode)) & 0170000) == (0040000))
	#define	S_ISREG(mode) ((((mode)) & 0170000) == (0100000))

	extern "C" int lstat( const char *, struct stat * );
	extern "C" int fstat( int, stat * );

	// dirent.h
	#define DT_UNKNOWN 0
	#define DT_FIFO 1
	#define DT_CHR 2
	#define DT_DIR 4
	#define DT_BLK 6
	#define DT_REG 8
	#define DT_LNK 10
	#define DT_SOCK 12
	#define DT_WHT 14

	struct dirent
	{
		unsigned long int d_ino;
		long int d_off;
		unsigned short int d_reclen;
		unsigned char d_type;
		char d_name[256];
	};

	using DIR = void *;

	extern "C" int closedir( DIR * );
	extern "C" DIR *opendir( const char * );
	extern "C" struct dirent *readdir( DIR * );
	extern "C" int readdir_r( DIR *, struct dirent *, struct dirent ** );
	extern "C" void rewinddir( DIR * );
	extern "C" void seekdir( DIR *, long int );
	extern "C" long int telldir( DIR * );

	// unistd.h
	extern "C" int close( int );
	extern "C" long lseek( int, long, int );
	extern "C" long read( int, void *, unsigned long );
	extern "C" long write( int, const void *, unsigned long );
	extern "C" int usleep( unsigned int );
	extern "C" int unlink( const char * );
	extern "C" int mkdir( const char *, unsigned int );
	extern "C" int rmdir( const char * );
	extern "C" void _exit( int );
	extern "C" __pid_t fork();
	extern "C" int execvp( const char *, char *const * );

	// wait.h
	#define	WNOHANG 1
	#define	WUNTRACED 2
	#define WEXITSTATUS(status)	(((status) & 0xff00) >> 8)
	#define WIFEXITED(status) (((status) & 0x7f) == 0)

	extern "C" __pid_t waitpid( __pid_t, int *, int );

	// libgen.h
	extern "C" char *dirname(char *);

	// sockaddr.h
	typedef unsigned short int sa_family_t;
	typedef uint16_t in_port_t;
	#define	__SOCKADDR_COMMON(sa_prefix) sa_family_t sa_prefix##family
	#define __SOCKADDR_COMMON_SIZE (sizeof(unsigned short int))

	// socket_type.h
	enum __socket_type
	{
		SOCK_STREAM = 1,
	#define SOCK_STREAM SOCK_STREAM
		SOCK_DGRAM = 2,
	#define SOCK_DGRAM SOCK_DGRAM
		SOCK_RAW = 3,
	#define SOCK_RAW SOCK_RAW
		SOCK_RDM = 4,
	#define SOCK_RDM SOCK_RDM
		SOCK_SEQPACKET = 5,
	#define SOCK_SEQPACKET SOCK_SEQPACKET
		SOCK_DCCP = 6,
	#define SOCK_DCCP SOCK_DCCP
		SOCK_PACKET = 10,
	#define SOCK_PACKET SOCK_PACKET
		SOCK_CLOEXEC = 02000000,
	#define SOCK_CLOEXEC SOCK_CLOEXEC
		SOCK_NONBLOCK = 00004000
	#define SOCK_NONBLOCK SOCK_NONBLOCK
	};

	// sys/socket.h
	#define PF_UNSPEC 0
	#define AF_UNSPEC PF_UNSPEC
	#define PF_INET	2
	#define AF_INET	PF_INET

	#define SOL_SOCKET 1
	#define SO_REUSEADDR 2

	#define SOMAXCONN 4096

	enum
	{
		SHUT_RD = 0,
	#define SHUT_RD SHUT_RD
		SHUT_WR,
	#define SHUT_WR SHUT_WR
		SHUT_RDWR
	#define SHUT_RDWR SHUT_RDWR
	};

	typedef unsigned int socklen_t;

	struct sockaddr
	{
		__SOCKADDR_COMMON (sa_);
		char sa_data[14];
	};

	extern "C" int setsockopt( int, int, int, const void *, socklen_t );
	extern "C" int bind( int, const struct sockaddr *, socklen_t );
	extern "C" int listen( int, int );
	extern "C" int socket( int, int, int );
	extern "C" int connect( int, const struct sockaddr *, socklen_t );
	extern "C" int shutdown( int, int );
	extern "C" int accept( int, struct sockaddr *, socklen_t * );
	extern "C" long int send( int, const void *, size_t, int );
	extern "C" long int sendto( int, const void *, size_t, int, const struct sockaddr *,socklen_t );
	extern "C" long int recv( int, void *, size_t, int );
	extern "C" long int recvfrom( int, void *, size_t, int, struct sockaddr *, socklen_t * );

	// netinet/tcp.h
	#define	TCP_NODELAY 1

	// netinet/in.h
	typedef uint32_t in_addr_t;
	struct in_addr
	{
		in_addr_t s_addr;
	};

	#define	INADDR_ANY ((in_addr_t) 0x00000000)

	enum
	{
		IPPROTO_TCP = 6,
	#define IPPROTO_TCP IPPROTO_TCP
		IPPROTO_UDP = 17,
	#define IPPROTO_UDP IPPROTO_UDP
	};

	struct sockaddr_in
	{
		__SOCKADDR_COMMON (sin_);
		in_port_t sin_port;
		struct in_addr sin_addr;

		unsigned char sin_zero[sizeof(struct sockaddr)
			- __SOCKADDR_COMMON_SIZE
			- sizeof(in_port_t)
			- sizeof(struct in_addr)];
	};

	extern "C" uint32_t ntohl( uint32_t );
	extern "C" uint16_t ntohs( uint16_t );
	extern "C" uint32_t htonl( uint32_t );
	extern "C" uint16_t htons( uint16_t );

	// arpa/inet.h
	extern "C" int inet_pton( int, const char *, void * );
	extern "C" const char *inet_ntop( int, const void *, char *, socklen_t );

	// netdb.h
	struct addrinfo
	{
		int ai_flags;
		int ai_family;
		int ai_socktype;
		int ai_protocol;
		socklen_t ai_addrlen;
		struct sockaddr *ai_addr;
		char *ai_canonname;
		struct addrinfo *ai_next;
	};

	extern "C" int getaddrinfo( const char *, const char *, const struct addrinfo *, struct addrinfo ** );
	extern "C" void freeaddrinfo( struct addrinfo * );

	// sched.h
	extern "C" int sched_yield();

#endif