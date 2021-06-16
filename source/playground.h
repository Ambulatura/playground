#ifndef PLAYGROUND_H

#include "playground_platform.h"

#include "playground_intrinsics.h"
#include "playground_math.h"
#include "playground_draw.h"
#include "playground_memory.h"
#include "playground_world.h"

struct PlaygroundState
{
	PlaygroundMemoryArena arena;

	World world;
	v2 screen_center;

	// NOTE(SSJSR): Player sprites.
	// TODO(SSJSR): Probably we need sprite group struct or something different?
	Bitmap player_idle_00;
	Bitmap player_idle_01;
	Bitmap player_idle_02;
	Bitmap player_idle_03;

	Bitmap player_run_00;
	Bitmap player_run_01;
	Bitmap player_run_02;
	Bitmap player_run_03;
	Bitmap player_run_04;
	Bitmap player_run_05;

	Bitmap player_jump_02;
	Bitmap player_jump_03;

	Bitmap player_jump_2_00;
	Bitmap player_jump_2_01;
	Bitmap player_jump_2_02;
	Bitmap player_jump_2_03;

	Bitmap player_fall_00;
	Bitmap player_fall_01;

	Bitmap player_wall_slide_00;
	Bitmap player_wall_slide_01;

	AnimationGroup* player_idle_animations;
	AnimationGroup* player_run_animations;
	AnimationGroup* player_jump_animations;
	AnimationGroup* player_jump_2_animations;
	AnimationGroup* player_fall_animations;
	AnimationGroup* player_wall_slide_animations;

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
	Bitmap familiar_idle_00;
	Bitmap familiar_idle_01;
	Bitmap familiar_idle_02;
	Bitmap familiar_idle_03;

	Bitmap familiar_run_00;
	Bitmap familiar_run_01;
	Bitmap familiar_run_02;
	Bitmap familiar_run_03;
	Bitmap familiar_run_04;
	Bitmap familiar_run_05;

	Bitmap familiar_run_dust_00;
	Bitmap familiar_run_dust_01;
	Bitmap familiar_run_dust_02;
	Bitmap familiar_run_dust_03;
	Bitmap familiar_run_dust_04;
	Bitmap familiar_run_dust_05;

	AnimationGroup* familiar_idle_animations;
	AnimationGroup* familiar_run_animations;
	
	Bitmap background;
};

#define PLAYGROUND_H
#endif
