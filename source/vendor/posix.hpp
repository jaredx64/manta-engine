#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <unistd.h>
		#include <fcntl.h>
		#include <dirent.h>
		#include <limits.h>
		#include <time.h>
		#include <sys/stat.h>
		#include <sys/mman.h>
	#include <vendor/conflicts.hpp>
#else
	#include <core/types.hpp>

	#define O_RDONLY 0x0000
	#define O_WRONLY 0x0001
	#define O_CREAT 0x0040
	#define O_TRUNC 0x0200

	#define S_IRUSR 00400
	#define S_IWUSR 00200
	#define S_IXUSR 00100

	#define S_IRGRP 00040
	#define S_IWGRP 00020
	#define S_IXGRP 00010

	#define S_IROTH 00004
	#define S_IWOTH 00002
	#define S_IXOTH 00001

	#define PROT_NONE 0
	#define PROT_READ 1
	#define PROT_WRITE 2

	#define MAP_PRIVATE 0x02
	#define MAP_FAILED reinterpret_cast<void *>(-1)

	#define CLOCK_MONOTONIC 1

	#define DT_UNKNOWN 0
	#define DT_FIFO 1
	#define DT_CHR 2
	#define DT_DIR 4
	#define DT_BLK 6
	#define DT_REG 8
	#define DT_LNK 10
	#define DT_SOCK 12
	#define DT_WHT 14

	struct timespec
	{
		long long tv_sec;
		long long tv_nsec;
	};

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

	struct dirent
	{
		unsigned long int d_ino;
		long int d_off;
		unsigned short int d_reclen;
		unsigned char d_type;
		char d_name[256];
	};

	using DIR = void *;

	// TODO: Fix
	#define S_ISDIR(mode) ((((mode)) & 0170000) == (0040000))

	extern "C" int closedir(DIR *);
	extern "C" DIR *opendir(const char *);
	extern "C" struct dirent *readdir(DIR *);
	extern "C" int readdir_r(DIR *, struct dirent *, struct dirent **);
	extern "C" void rewinddir(DIR *);
	extern "C" void seekdir(DIR *, long int);
	extern "C" long int telldir(DIR *);

	extern "C" int lstat(const char *, struct stat *);
	extern "C" int clock_gettime(int, timespec *);
	extern "C" int open(const char *, int, ...);
	extern "C" void *mmap(void *, unsigned long, int, int, int, long);
	extern "C" int mprotect(void *, unsigned long, int);
	extern "C" int munmap(void *, unsigned long);
	extern "C" int fstat(int, stat *);
	extern "C" int close(int);
	extern "C" long lseek(int, long, int);
	extern "C" long read(int, void *, unsigned long);
	extern "C" long write(int, const void *, unsigned long);
	extern "C" int usleep(unsigned int);
	extern "C" int unlink(const char *);
	extern "C" int mkdir(const char *, unsigned int);
	extern "C" int rmdir(const char *);
#endif