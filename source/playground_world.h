#ifndef PLAYGROUND_WORLD_H
#define PLAYGROUND_WORLD_H

#define INVALID_TILE_POSITION 1000000
#define INVALID_POSITION v2(1000000.0f, 1000000.0f)

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

#include "playground_entity.h"

struct World
{
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

	u32 active_entity_count;
	u32 active_entity_indices[1024];

	u32 entity_count;
	Entity entities[4096];
};

#endif

