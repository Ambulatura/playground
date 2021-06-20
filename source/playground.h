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
	Bitmap player_idle[4];
	Bitmap player_run[6];
	Bitmap player_jump[2];
	Bitmap player_jump_2[4];
	Bitmap player_fall[2];
	Bitmap player_wall_slide[2];
	
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
	Bitmap familiar_idle[4];
	Bitmap familiar_run[6];
	Bitmap familiar_run_dust[6];

	AnimationGroup* familiar_idle_animations;
	AnimationGroup* familiar_run_animations;
	
	Bitmap background;
};

// struct TransientState
// {
// 	PlaygroundMemoryArena transient_arena;

// 	b32 is_initialized;
// };

#define PLAYGROUND_H
#endif
