#ifndef PLAYGROUND_H

#include "playground_platform.h"

#include "playground_intrinsics.h"
#include "playground_math.h"
#include "playground_memory.h"

struct TilePosition
{
	i32 tile_x;
	i32 tile_y;
	
	v2 xy;
};

struct TileMapPosition
{
	u32 tile_map_x;
	u32 tile_map_y;

	i32 tile_map_relative_tile_x;
	i32 tile_map_relative_tile_y;
};

struct TileMap
{
	u32 tile_map_index;
	u32 entity_index_count;
	u32 entity_indices[100];
	u32* tiles;
};

enum EntityType
{
	NULL_TYPE,
	
	PLAYER_TYPE,
	WALL_TYPE,
	MONSTER_TYPE,
};

struct Entity
{
	EntityType type;

	v2 direction;
	v2 position;
	v2 velocity;
	u32 facing_direction;
	
	TilePosition tile_position;
	f32 width;
	f32 height;
	b32 collides;

	f32 distance_remaining;
};

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

	MAX_STATE_TYPE,
};

struct PlayerBitmapState
{
	u32 tick_counter;

	PlayerStateType current_state;
	PlayerStateType last_state;
	
	u32 state_count;
	PlayerStateType state_types[4];
	u32 index_offsets[4];

	u32 bitmap_index_without_offset;
	u32 bitmap_count;
	LoadedBmp bitmaps[14];
};

struct PlaygroundState
{
	PlaygroundMemoryArena arena;

	TilePosition camera;
	TilePosition desired_camera;

	b32 is_camera_moving;
	u32 camera_movement_duration;
	u32 camera_movement_duration_remaining;

	u32 player_entity_index;

	f32 tile_side_in_pixels;
	f32 tile_side_in_meters;
	f32 meters_to_pixels;
	i32 tile_count_y;
	i32 tile_count_x;
	u32 tile_map_count_x;
	u32 tile_map_count_y;
	TileMap* tile_maps;

	u32 entity_count;
	Entity entities[1024];
	
	LoadedBmp background;
	PlayerBitmapState player_bitmap_state;
};

#define PLAYGROUND_H
#endif
