#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <pthread.h>
	#include <vendor/conflicts.hpp>
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// wordsize.h

	#if defined __x86_64__ && !defined __ILP32__
		# define __WORDSIZE	64
	#else
		# define __WORDSIZE	32
		#define __WORDSIZE32_SIZE_ULONG 0
		#define __WORDSIZE32_PTRDIFF_LONG 0
	#endif

	#define __WORDSIZE_TIME64_COMPAT32 1

	#ifdef __x86_64__
		# define __SYSCALL_WORDSIZE 64
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pthreadtypes-arch.h

	#ifdef __x86_64__
		#if __WORDSIZE == 64
			#define __SIZEOF_PTHREAD_MUTEX_T 40
			#define __SIZEOF_PTHREAD_ATTR_T 56
			#define __SIZEOF_PTHREAD_RWLOCK_T 56
			#define __SIZEOF_PTHREAD_BARRIER_T 32
		#else
			#define __SIZEOF_PTHREAD_MUTEX_T 32
			#define __SIZEOF_PTHREAD_ATTR_T 32
			#define __SIZEOF_PTHREAD_RWLOCK_T 44
			#define __SIZEOF_PTHREAD_BARRIER_T 20
		#endif
	#else
		#define __SIZEOF_PTHREAD_MUTEX_T 24
		#define __SIZEOF_PTHREAD_ATTR_T 36
		#define __SIZEOF_PTHREAD_RWLOCK_T 32
		#define __SIZEOF_PTHREAD_BARRIER_T 20
	#endif

	#define __SIZEOF_PTHREAD_MUTEXATTR_T 4
	#define __SIZEOF_PTHREAD_COND_T 48
	#define __SIZEOF_PTHREAD_CONDATTR_T 4
	#define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
	#define __SIZEOF_PTHREAD_BARRIERATTR_T 4

	#define __LOCK_ALIGNMENT
	#define __ONCE_ALIGNMENT

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// atomic_wide_counter.h

	union __atomic_wide_counter
	{
		__extension__ unsigned long long int __value64;
		struct
		{
			unsigned int __low;
			unsigned int __high;
		} __value32;
	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// thread-shared-types.h

	struct __pthread_internal_list
	{
		struct __pthread_internal_list *__prev;
		struct __pthread_internal_list *__next;
	};
	using __pthread_list_t = __pthread_internal_list;

	struct __pthread_cond_s
	{
		__atomic_wide_counter __wseq;
		__atomic_wide_counter __g1_start;
		unsigned int __g_size[2] __LOCK_ALIGNMENT;
		unsigned int __g1_orig_size;
		unsigned int __wrefs;
		unsigned int __g_signals[2];
		unsigned int __unused_initialized_1;
		unsigned int __unused_initialized_2;
	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// struct_mutex.h

	struct __pthread_mutex_s
	{
		int __lock;
		unsigned int __count;
		int __owner;
		#ifdef __x86_64__
			unsigned int __nusers;
		#endif
		int __kind;
		#ifdef __x86_64__
			short __spins;
			short __elision;
			__pthread_list_t __list;
			#define __PTHREAD_MUTEX_HAVE_PREV 1
		#else
			unsigned int __nusers;
			__extension__ union
			{
				struct
				{
					short __espins;
					short __eelision;
				#define __spins __elision_data.__espins
				#define __elision __elision_data.__eelision
				} __elision_data;
				__pthread_slist_t __list;
			};
			#define __PTHREAD_MUTEX_HAVE_PREV 0
		#endif
	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pthreadtypes.h

	using pthread_t = unsigned long int;
	using pthread_attr_t = void *; // not really, but we don't use it...
	using pthread_mutexattr_t = void *; // not really, but we don't use it...
	using pthread_condattr_t = void *; // not really, but we don't use it...

	enum
	{
		PTHREAD_MUTEX_TIMED_NP,
		PTHREAD_MUTEX_RECURSIVE_NP,
		PTHREAD_MUTEX_ERRORCHECK_NP,
		PTHREAD_MUTEX_ADAPTIVE_NP,
		PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
		PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
		PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
		PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
		#ifdef __USE_GNU
		, PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_TIMED_NP
		#endif
	};

	union pthread_mutex_t
	{
		struct __pthread_mutex_s __data;
		char __size[__SIZEOF_PTHREAD_MUTEX_T];
		long int __align;
	};

	union pthread_cond_t
	{
		struct __pthread_cond_s __data;
		char __size[__SIZEOF_PTHREAD_COND_T];
		__extension__ long long int __align;
	};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pthread.h

	extern "C" int pthread_create( pthread_t *, const pthread_attr_t *, void *(*)(void *), void * );
	extern "C" pthread_t pthread_self( void );

	extern "C" int pthread_mutexattr_init( pthread_mutexattr_t * );
	extern "C" int pthread_mutexattr_settype( pthread_mutexattr_t *, int );
	extern "C" int pthread_mutexattr_destroy( pthread_mutexattr_t * );

	extern "C" int pthread_mutex_init( pthread_mutex_t *, const pthread_mutexattr_t * );
	extern "C" int pthread_mutex_destroy( pthread_mutex_t * );
	extern "C" int pthread_mutex_lock( pthread_mutex_t * );
	extern "C" int pthread_mutex_unlock( pthread_mutex_t * );

	extern "C" int pthread_cond_init( pthread_cond_t *, const pthread_condattr_t * );
	extern "C" int pthread_cond_destroy( pthread_cond_t * );
	extern "C" int pthread_cond_wait( pthread_cond_t *, pthread_mutex_t * );
	extern "C" int pthread_cond_signal( pthread_cond_t * );
	extern "C" int pthread_cond_broadcast( pthread_cond_t * );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif