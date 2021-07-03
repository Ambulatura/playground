#ifndef PLAYGROUND_ENTITY_H

enum EntityType
{
	NULL_TYPE,
	
	PLAYER_TYPE,
	FAMILIAR_TYPE,
	WALL_TYPE,
	MONSTER_TYPE,
	BALL_TYPE,
	SWORD_TYPE,
};

enum EntityFlag
{
	COLLIDES_FLAG = (1 << 1),
	NONSPATIAL_FLAG = (1 << 2),
	MOVEABLE_FLAG = (1 << 3),

	// NOTE(SSJSR): Passive states.
	ON_GROUND_FLAG = (1 << 4),
	ON_WALL_FLAG = (1 << 5),
	ON_AIR_FLAG = (1 << 6),

	// NOTE(SSJSR): Active states. (Actions)
	IDLE_FLAG = (1 << 7),
	RUN_FLAG = (1 << 8),
	JUMP_FLAG = (1 << 9),
	JUMP_2_FLAG = (1 << 10),
	FALL_FLAG = (1 << 11),
	WALL_SLIDE_FLAG = (1 << 12),
	ATTACK_1_FLAG = (1 << 13),

	STATE_ANY = (1 << 13),
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
	ATTACK_1_ANIMATION_TYPE,
	CAST_ANIMATION_TYPE,

	MAX_ANIMATION_TYPE,
};

struct AnimationFrame
{
	Bitmap* sprite;

	// TODO(SSJSR): Can we use entities for hitboxes?
	u32 hitboxes[4];
};

struct Animation
{
	// NOTE(SSJSR):
	// duration: How many seconds does whole animation take?
	f32 duration;
	f32 total_elapsed_time;
	
	u32 frame_count;
	AnimationFrame frames[16];
};

struct AnimationGroup
{
	AnimationType group_type;
	u32 animation_count;
	u32 total_animation_count;
	Animation* animations;
};

struct CollisionVolume
{
	v2 offset_position;
	v2 dimension;
};

struct CollisionVolumeGroup
{
	CollisionVolume total_volume;

	u32 volume_count;
	u32 total_volume_count;
	CollisionVolume* volumes;
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

	u32 ball_index;
	u32 sword_index;
	
	f32 distance_limit;

	u32 animation_group_count;
	AnimationGroup* animation_groups[16];

	CollisionVolumeGroup* collision_volume_group;

	f32 state_time;
	u32 animation_frame_index;

	u32 last_action;
	u32 current_action;

	u32 jump_count;
	u32 attack_count;
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
