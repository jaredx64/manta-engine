#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
#include <vendor/conflicts.hpp>
	#include <math.h>
#include <vendor/conflicts.hpp>
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// math.h

	#if PIPELINE_COMPILER_MSVC
		extern "C" double cos(double);
		extern "C" float cosf(float);
		extern "C" double acos(double);
		extern "C" float acosf(float);
		extern "C" double sin(double);
		extern "C" float sinf(float);
		extern "C" double asin(double);
		extern "C" float asinf(float);
		extern "C" double tan(double);
		extern "C" float tanf(float);
		extern "C" double atan(double);
		extern "C" float atanf(float);
		extern "C" double atan2(double, double);
		extern "C" float atan2f(float, float);
		extern "C" double fmin(double, double);
		extern "C" double fmax(double, double);
		extern "C" float fminf(float, float);
		extern "C" float fmaxf(float, float);
		extern "C" float ceilf(float);
		extern "C" float coshf(float);
		extern "C" float expf(float);
		extern "C" double fabs(double);
		extern "C" double ceil(double);
		extern "C" double floor(double);
		extern "C" double fmod(double, double);
		extern "C" float floorf(float);
		extern "C" double round(double);
		extern "C" double sqrt(double);
		extern "C" float sqrtf(float);
		extern "C" float powf(float, float);
		extern "C" double pow(double, double);
		extern "C" int abs(int);
		extern "C" double frexp(double, int *);
		extern "C" double ldexp(double, int);
	#else
		inline double cos(double x) { return __builtin_cos(x); }
		inline float cosf(float x) { return __builtin_cosf(x); }
		inline double acos(double x) { return __builtin_acos(x); }
		inline float acosf(float x) { return __builtin_acosf(x); }
		inline double sin(double x) { return __builtin_sin(x); }
		inline float sinf(float x) { return __builtin_sinf(x); }
		inline float asinf(float x) { return __builtin_asinf(x); }
		inline float tan(double x) { return __builtin_tan(x); }
		inline float tanf(float x) { return __builtin_tanf(x); }
		inline float atan(double x) { return __builtin_atan(x); }
		inline float atanf(float x) { return __builtin_atanf(x); }
		inline float atan2(double x, double y) { return __builtin_atan2(x, y); }
		inline float atan2f(float x, float y) { return __builtin_atan2f(x, y); }
		inline double fmin(double a, double b) { return __builtin_fmin(a, b); }
		inline double fmax(double a, double b) { return __builtin_fmax(a, b); }
		inline float fminf(float a, float b) { return __builtin_fminf(a, b); }
		inline float fmaxf(float a, float b) { return __builtin_fmaxf(a, b); }
		inline float ceilf(float x) { return __builtin_ceilf(x); }
		inline float coshf(float x) { return __builtin_coshf(x); }
		inline float expf(float x) { return __builtin_expf(x); }
		inline double fabs(double x) { return __builtin_fabs(x); }
		inline double ceil(double x) { return __builtin_ceil(x); }
		inline double floor(double x) { return __builtin_floor(x); }
		inline double fmod(double x, double y) { return __builtin_fmod(x, y); }
		inline float floorf(float x) { return __builtin_floorf(x); }
		inline double round(double x) { return __builtin_round(x); }
		inline double sqrt(double x) { return __builtin_sqrt(x); }
		inline float sqrtf(float x) { return __builtin_sqrtf(x); }
		inline float powf(float x, float y) { return __builtin_powf(x, y); }
		inline double pow(double x, double y) { return __builtin_pow(x, y); }
		inline double abs(double x) { return __builtin_fabs(x); }
		inline float abs(float x) { return __builtin_fabsf(x); }
		inline double frexp(double x, int *y) { return __builtin_frexp(x, y); }
		inline double ldexp(double x, int y) { return __builtin_ldexp(x, y); }

		// TODO: Why are the compilers so mad about this single function?
		#if defined(__clang__)
			extern "C" int abs(int);
		#elif defined(__GNUC__)
			extern "C" int abs(int) noexcept;
		#else
			extern "C" int abs(int);
		#endif
	#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif