#ifndef PLAYGROUND_H

#include "playground_platform.h"

#include "playground_intrinsics.h"
#include "playground_math.h"
#include "playground_memory.h"
#include "playground_world.h"

struct PlaygroundState
{
	PlaygroundMemoryArena arena;

	World world;
	v2 screen_center;

	LoadedBmp player_cast_00;
	LoadedBmp player_cast_01;
	LoadedBmp player_cast_02;
	LoadedBmp player_cast_03;

	LoadedBmp fireball_00;
	LoadedBmp fireball_01;
	LoadedBmp fireball_02;
	LoadedBmp fireball_03;
	LoadedBmp fireball_04;
	
	LoadedBmp background;
};

#define PLAYGROUND_H
#endif
