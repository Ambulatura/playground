#ifndef PLAYGROUND_ENTITY_H

enum EntityType
{
	NULL_TYPE,
	
	PLAYER_TYPE,
	FAMILIAR_TYPE,
	WALL_TYPE,
	MONSTER_TYPE,
	BALL_TYPE,
};

enum EntityFlag
{
	COLLIDES_FLAG = (1 << 1),
	NONSPATIAL_FLAG = (1 << 2),
	MOVEABLE_FLAG = (1 << 3),

	RUN_FLAG = (1 << 4),
	JUMP_FLAG = (1 << 5),
	DOUBLE_JUMP_FLAG = (1 << 6),
	FALL_FLAG = (1 << 7),
	ON_GROUND_FLAG = (1 << 8),
	ON_WALL_FLAG = (1 << 9),
};

struct LoadedBmp
{
	u32* pixels;
	i32 width;
	i32 height;

	i32 align_x;
	i32 align_y;
};

enum AnimationType
{
	NULL_ANIMATION_TYPE,

	IDLE_ANIMATION_TYPE,
	RUN_ANIMATION_TYPE,
	JUMP_ANIMATION_TYPE,
	JUMP_2_ANIMATION_TYPE,
	FALL_ANIMATION_TYPE,
	WALL_SLIDE_ANIMATION_TYPE,
	CAST_ANIMATION_TYPE,

	MAX_ANIMATION_TYPE,
};

struct AnimationFrame
{
	LoadedBmp* sprite;

	// TODO(SSJSR): Can we use entities for hitboxes?
	u32 hitboxes[4];
};

struct Animation
{
	// NOTE(SSJSR):
	// duration: How many seconds does whole animation take?
	f32 duration;
	f32 total_elapsed_time;
	
	AnimationType type;
	u32 frame_index;
	u32 frame_count;
	AnimationFrame frames[16];
};

struct Entity
{
	EntityType type;
	u32 flags;

	b32 updatable;
	
	v2 direction;
	v2 acceleration;
	v2 position;
	v2 velocity;
	u32 facing_direction;
	
	TilePosition tile_position;
	f32 width;
	f32 height;

	u32 ball_index;
	f32 distance_limit;

	// u32 ticks;

	u32 animation_count;
	Animation animations[16];
};

struct MoveFeature
{
	v2 direction;
	v2 acceleration;

	f32 friction_coefficient;
	b32 max_unit_vector_length;
};

#define PLAYGROUND_ENTITY_H
#endif
