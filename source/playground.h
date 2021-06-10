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

	// NOTE(SSJSR): Player sprites.
	// TODO(SSJSR): Probably we need sprite group struct or something different?
	LoadedBmp player_idle_00;
	LoadedBmp player_idle_01;
	LoadedBmp player_idle_02;
	LoadedBmp player_idle_03;

	LoadedBmp player_run_00;
	LoadedBmp player_run_01;
	LoadedBmp player_run_02;
	LoadedBmp player_run_03;
	LoadedBmp player_run_04;
	LoadedBmp player_run_05;

	LoadedBmp player_jump_02;
	LoadedBmp player_jump_03;

	LoadedBmp player_jump_2_00;
	LoadedBmp player_jump_2_01;
	LoadedBmp player_jump_2_02;
	LoadedBmp player_jump_2_03;

	LoadedBmp player_fall_00;
	LoadedBmp player_fall_01;

	LoadedBmp player_wall_slide_00;
	LoadedBmp player_wall_slide_01;

	
	// LoadedBmp player_cast_00;
	// LoadedBmp player_cast_01;
	// LoadedBmp player_cast_02;
	// LoadedBmp player_cast_03;

	// LoadedBmp fireball_00;
	// LoadedBmp fireball_01;
	// LoadedBmp fireball_02;
	// LoadedBmp fireball_03;
	// LoadedBmp fireball_04;

	// NOTE(SSJSR): Familiar sprites.
	LoadedBmp familiar_idle_00;
	LoadedBmp familiar_idle_01;
	LoadedBmp familiar_idle_02;
	LoadedBmp familiar_idle_03;

	LoadedBmp familiar_run_00;
	LoadedBmp familiar_run_01;
	LoadedBmp familiar_run_02;
	LoadedBmp familiar_run_03;
	LoadedBmp familiar_run_04;
	LoadedBmp familiar_run_05;
	
	LoadedBmp background;
};

#define PLAYGROUND_H
#endif
