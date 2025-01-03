#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
        #include <pthread.h>
    #include <vendor/conflicts.hpp>
#else
    using pthread_t = unsigned long int;
    using pthread_attr_t = void *; // not really, but we don't use it...
    using pthread_mutexattr_t = void *; // not really, but we don't use it...
    using pthread_condattr_t = void *; // not really, but we don't use it...

    struct pthread_mutex_t
    {
        int value;
    };

    struct pthread_cond_t
    {
        int value;
    };

    extern "C" int pthread_create( pthread_t *, const pthread_attr_t *, void *(*)(void *), void * );
    extern "C" pthread_t pthread_self( void );

    extern "C" int pthread_mutex_init( pthread_mutex_t *, const pthread_mutexattr_t * );
    extern "C" int pthread_mutex_destroy( pthread_mutex_t * );
    extern "C" int pthread_mutex_lock( pthread_mutex_t * );
    extern "C" int pthread_mutex_unlock( pthread_mutex_t * );

    extern "C" int pthread_cond_init( pthread_cond_t *, const pthread_condattr_t * );
    extern "C" int pthread_cond_destroy( pthread_cond_t * );
    extern "C" int pthread_cond_wait( pthread_cond_t *, pthread_mutex_t * );
    extern "C" int pthread_cond_signal( pthread_cond_t * );
    extern "C" int pthread_cond_broadcast( pthread_cond_t * );
#endif