#ifndef PLAYGROUND_H

#include "playground_platform.h"

#include "playground_intrinsics.h"
#include "playground_math.h"
#include "playground_render.h"
#include "playground_memory.h"
#include "playground_world.h"
#include "playground_state.h"

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
	Bitmap player_attack_1[5];
	Bitmap player_attack_2[6];
	
	AnimationGroup* player_idle_animations;
	AnimationGroup* player_run_animations;
	AnimationGroup* player_jump_animations;
	AnimationGroup* player_jump_2_animations;
	AnimationGroup* player_fall_animations;
	AnimationGroup* player_wall_slide_animations;
	AnimationGroup* player_attack_1_animations;
	AnimationGroup* player_attack_2_animations;

	// NOTE(SSJSR): Familiar sprites.
	Bitmap familiar_idle[4];
	Bitmap familiar_run[6];
	Bitmap familiar_run_dust[6];

	AnimationGroup* familiar_idle_animations;
	AnimationGroup* familiar_run_animations;
	
	Bitmap background;

	CollisionVolumeGroup* attack_1_collision_volume_group;
	CollisionVolumeGroup* attack_2_collision_volume_group;
};

struct TransientState
{
	PlaygroundMemoryArena transient_arena;

	b32 is_initialized;
};

#define PLAYGROUND_H
#endif
