#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/color.hpp>

#include <manta/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SchedulerJob
{
public:
	void execute( Delta delta );
	void rebalance();

public:
	float frequencyTarget = 1.0f / 100.0f;
	float frequencyMinimum = 1.0f / 100.0f;
	float cost = 0.0f; // Average cost of a step of work (ms)
	int steps = 0; // Amount of work to do this frame
	double time = 0.0; // Time (ms) since last execution
	void ( *work )( Delta delta ) = nullptr;

	const char *name = "";
	Color color = c_white;

	double rollingCost = 0.0;
	int rollingCount = 0;
	int rollingCountPrevious = 0;
};

class Scheduler
{
public:
	void init( float budget );
	void free();
	SchedulerJob &register_job( const char *name, Color color, float targetFrequency,
		float minimumFrequency, void ( *func )( Delta ) );
	void update( Delta delta );
	void draw( int x, int y, Delta delta );

public:
	List<SchedulerJob> jobs;
	List<int> priority;
	float budget = 0.0f; // Maximum time of target work (ms)
	float cost = 0.0f; // Average cost of a frame of scheduled work (ms)

private:
	double timeSecond = 0.0;

	Timer rollingTimer;
	double rollingCost = 0.0;
	int rollingCount = 0;

	// TODO: Debug build only
	struct FrameEvent
	{
		Color color;
		float time;
	};

	List<FrameEvent> frames[128];
	int frame = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////