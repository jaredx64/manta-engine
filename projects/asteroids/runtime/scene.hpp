#pragma once

#include <manta/objects.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Scene
{
	extern ObjectContext objects;
	extern int score;
	extern bool dead;
}

extern void scene_init();
extern void scene_free();

extern void scene_update( const Delta delta );
extern void scene_draw( const Delta delta );