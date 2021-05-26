#ifndef PLAYGROUND_H

#include "playground_platform.h"

#include "playground_intrinsics.h"
#include "playground_math.h"
#include "playground_memory.h"
#include "playground_world.h"

struct LoadedBmp
{
	u32* pixels;
	i32 width;
	i32 height;

	u32 align_x;
	u32 align_y;
};

enum PlayerStateType
{
	NULL_STATE_TYPE,

	IDLE_STATE_TYPE,
	RUN_STATE_TYPE,
	JUMP_STATE_TYPE,
	CAST_STATE_TYPE,

	MAX_STATE_TYPE,
};

struct PlayerBitmapState
{
	u32 tick_counter;

	PlayerStateType current_state;
	PlayerStateType last_state;
	
	u32 state_count;
	PlayerStateType state_types[PlayerStateType::MAX_STATE_TYPE];
	u32 index_offsets[PlayerStateType::MAX_STATE_TYPE];

	u32 bitmap_index_without_offset;
	u32 bitmap_count;
	LoadedBmp* bitmaps[18];
};

enum FireballStateType
{
	NULL_FIREBALL_STATE_TYPE,

	CASTING_FIREBALL_STATE_TYPE,

	MAX_FIREBALL_STATE_TYPE,
};

struct FireballBitmapState
{
	u32 tick_counter;

	FireballStateType current_state;
	FireballStateType last_state;
	
	u32 state_count;
	FireballStateType state_types[FireballStateType::MAX_FIREBALL_STATE_TYPE];
	u32 index_offsets[FireballStateType::MAX_FIREBALL_STATE_TYPE];

	u32 bitmap_index_without_offset;
	u32 bitmap_count;
	LoadedBmp* bitmaps[14];
};

struct PlaygroundState
{
	PlaygroundMemoryArena arena;

	World world;
	v2 screen_center;
	v2 mouse_offset;

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

	LoadedBmp player_jump_00;
	LoadedBmp player_jump_01;
	LoadedBmp player_jump_02;
	LoadedBmp player_jump_03;

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
	PlayerBitmapState player_bitmap_state;
	FireballBitmapState fireball_bitmap_state;
};

#define PLAYGROUND_H
#endif
