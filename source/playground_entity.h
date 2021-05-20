#ifndef PLAYGROUND_ENTITY_H

enum EntityType
{
	NULL_TYPE,
	
	PLAYER_TYPE,
	WALL_TYPE,
	MONSTER_TYPE,
	BALL_TYPE,
};

enum EntityFlag
{
	COLLIDES_FLAG = (1 << 1),
	NONSPATIAL_FLAG = (1 << 2),
};

struct Entity
{
	EntityType type;
	u32 flags;

	v2 direction;
	v2 position;
	v2 velocity;
	u32 facing_direction;
	
	TilePosition tile_position;
	f32 width;
	f32 height;

	u32 ball_index;
	f32 distance_limit;

	u32 tick;
};

#define PLAYGROUND_ENTITY_H
#endif
