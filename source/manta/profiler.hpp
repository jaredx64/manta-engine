#pragma once

#include <config.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if COMPILE_PROFILING

#include <core/color.hpp>
#include <manta/time.hpp>

namespace CoreProfiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ProfilerSnapshot
{
	char nameSnapshot[128] = "";
	char nameFunction[128] = "";
	Color color = Color { 0, 0, 0, 0 };
	double timeStartUS = 0.0;
	double timeEndUS = 0.0;
	int depth = 0;
};


class ProfilerManager
{
public:
	void init();
	void free();
	void clear();
	void start();
	void stop();
	void frame_start();
	void frame_end();

	void draw( Delta delta );

public:
	static constexpr int MAX_SNAPSHOTS = 512;
	static constexpr int MAX_FRAMES = 144;

	ProfilerSnapshot snapshots[MAX_FRAMES][MAX_SNAPSHOTS] = { };
	int snapshotsCount[MAX_FRAMES] = { 0 };
	double frameTimeUS[MAX_FRAMES] = { 0.0 };
	double frameTimeUSHighest = 0.0;
	double frameTimeUSLowest = 0.0;
	double frameTimeUSAverage = 0.0;
	double frameTimeStart = 0.0;
	int snapshotCurrent = 0;
	int depthCurrent = 0;
	int frameCurrent = 0;


	bool capturingFrame = false;
	bool capturing = true;
	Timer timer;
};


extern ProfilerManager PROFILER;


class ProfilerScope
{
public:
	ProfilerScope( const char *nameSnapshot, const char *nameFunction, Color color = c_white );
	~ProfilerScope();

	ProfilerSnapshot *snapshot = nullptr;
};

};
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if COMPILE_PROFILING
	#define PROFILING( code ) code

	#define PROFILE_SCOPE( name ) \
		CoreProfiler::ProfilerScope _PROFILE_SCOPE_ { name, __FUNCTION__ };

	#define PROFILE_SCOPE_COLOR( name, color ) \
		CoreProfiler::ProfilerScope _PROFILE_SCOPE_ { name, __FUNCTION__, color };
#else
	#define PROFILING( code )
	#define PROFILE_SCOPE( name )
	#define PROFILE_SCOPE_COLOR( name, color )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////