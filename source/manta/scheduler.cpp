#include <manta/scheduler.hpp>

#include <core/debug.hpp>

#include <manta/draw.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SchedulerJob::execute( const Delta delta )
{
	Timer timer;
	work( delta ); // Perform work
	rollingCost += timer.ms();
	rollingCount++;
}


void SchedulerJob::rebalance()
{
	PrintLn( "Executions per second: %d", rollingCount );
	if( rollingCount <= 0 ) { return; }
	cost = static_cast<float>( rollingCost / rollingCount );
	rollingCost = 0.0;
	rollingCountPrevious = rollingCount;
	rollingCount = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scheduler::init( const float budget )
{
	this->budget = budget;
	rollingCost = 0.0;
	timeSecond = 0.0;
	jobs.init();
	priority.init();

	// TODO: Debug only
	for( int i = 0; i < 128; i++ ) { frames[i].init(); }
}


void Scheduler::free()
{
	jobs.free();
	priority.free();

	// TODO: Debug only
	for( int i = 0; i < 128; i++ ) { frames[i].free(); }
}


SchedulerJob &Scheduler::register_job( const char *name, const Color color, float targetFrequency, float minimumFrequency,
	void ( *func )( const Delta ) )
{
	SchedulerJob &job = jobs.add( SchedulerJob { } );
	job.name = name;
	job.color = color;
	job.frequencyTarget = targetFrequency;
	job.frequencyMinimum = minimumFrequency;
	job.work = func;
	return job;
}


void Scheduler::update( const Delta delta )
{
	frame = ( frame + 1 ) % 128;
	frames[frame].clear();

	// Recalculate job costs every second
	timeSecond += delta;
	if( timeSecond >= 1.0 )
	{
		PrintLn( "\n" );
		PrintLn( "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" );
		for( SchedulerJob &job : jobs ) { job.rebalance(); PrintLn( "%.3f", job.cost ); }
		PrintLn( "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" );
		timeSecond = 0.0;

		cost = static_cast<float>( rollingCost / rollingCount );
		rollingCost = 0.0;
		rollingCount = 0;
	}

	// Advance SchedulerJob Times
	for( SchedulerJob &job : jobs ) { job.time += delta; }

	// Prioritize Jobs (TODO: more efficient sort)
	priority.clear();
	for( usize i = 0; i < jobs.count(); i++ )
	for( usize j = 0; j <= priority.count(); j++ )
	{
		if( j == priority.count() )
		{
			priority.add( i );
			break;
		}
		else if( jobs[i].time >= jobs[priority[j]].time )
		{
			priority.insert( j, i );
			break;
		}
	}

	float frameCost = 0.0f;

	// Target Frequency Jobs
	const float threshold = min( cost, budget );
	while( true )
	{
		bool working = false;

		for( int index : priority )
		{
			SchedulerJob &job = jobs[index];
			const int steps = static_cast<int>( job.time / job.frequencyTarget );
			if( steps == 0 ) { continue; }

			// Target Frequency
			if( job.time < job.frequencyMinimum )
			{
				if( frameCost > 0.0 && frameCost > threshold ) { continue; }
				job.steps++;
				job.time -= job.frequencyTarget;
				frameCost += job.cost;
				working = true;
			}
			else
			// Minimum Frequency
			{
				job.steps++;
				frameCost += job.cost;
				if( frameCost > budget ) { job.time = 0.0; } else { job.time -= job.frequencyTarget; }
				working = true;
			}
		}

		if( !working ) { break; }
	}

	// Process Jobs
	rollingTimer.start();
	{
		for( SchedulerJob &job : jobs )
		{
			while( job.steps > 0 )
			{
				// TODO: VISUALIZER DEBUG (REMOVE)
				FrameEvent event;
				event.color = job.color;
				Timer eventTimer;

				job.execute( delta );
				job.steps--;

				// TODO: VISUALIZER DEBUG (REMOVE)
				event.time = eventTimer.ms();
				frames[frame].add( event );
			}
		}
	}
	rollingCost += rollingTimer.ms();
	rollingCount++;
};


void Scheduler::draw( const int x, const int y, const Delta delta )
{
	const int xScale = 8;
	const int yScale = 16;
	const int x1 = x;
	const int y1 = y;
	const int x2 = x + ARRAY_LENGTH( frames ) * xScale;
	const int y2 = y + ( 32.0f ) * yScale;

	for( usize i = 0; i < ARRAY_LENGTH( frames ); i++ )
	{
		float dx = x1 + i * xScale;
		float dy = y2;
		for( FrameEvent &event : frames[i] )
		{
			draw_rectangle( dx, dy - event.time * yScale, dx + xScale, dy, event.color );
			draw_rectangle( dx, dy - event.time * yScale, dx + xScale, dy, c_black, true );
			dy -= event.time * yScale;
		}
	}

	draw_line(
		x1 + ( frame + 1 ) * xScale, y1,
		x1 + ( frame + 1 ) * xScale, y2, c_dkgray );

	draw_line(
		x1 - 16, y2 - budget * yScale,
		x2 + 16, y2 - budget * yScale, c_white );

	draw_line(
		x1 - 16, y2 - cost * yScale,
		x2 + 16, y2 - cost * yScale, c_yellow );

	// Draw Jobs
	for( usize i = 0; i < jobs.count(); i++ )
	{
		const SchedulerJob &job = jobs[i];
		draw_text_f( fnt_iosevka, 16, x1, y1 + i * 20,
			job.color, "%s: %.4f ms | %d steps/sec", job.name, job.cost, job.rollingCountPrevious );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////