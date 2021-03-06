internal AnimationGroup* MakeAnimationGroup(PlaygroundState* playground_state,
											 AnimationType group_type,
											 u32 total_animation_count)
{
	AnimationGroup* animation_group = PushStruct(&playground_state->arena, AnimationGroup);
	animation_group->animation_count = 0;
	animation_group->total_animation_count = total_animation_count;
	animation_group->animations = PushArray(&playground_state->arena, animation_group->total_animation_count, Animation);
	animation_group->group_type = group_type;

	return animation_group;
}

internal void AddAnimationToAnimationGroup(AnimationGroup* animation_group, Bitmap** sprites, f32 duration, u32 frame_count)
{
	ASSERT(animation_group->animation_count <
		   animation_group->total_animation_count);

	Animation* animation = animation_group->animations +
		animation_group->animation_count++;
	animation->duration = duration;
	animation->frame_count = frame_count;
	animation->total_elapsed_time = 0.0f;

	for (u32 frame_index = 0; frame_index < animation->frame_count; ++frame_index) {
		AnimationFrame* frame = animation->frames + frame_index;
		frame->sprite = sprites[frame_index];
	}
}

internal void AddAnimationGroup(Entity* entity, AnimationGroup* animation_group)
{
	u32 index = animation_group->group_type;
	ASSERT(index < ARRAY_COUNT(entity->animation_groups));
	ASSERT(entity->animation_group_count < ARRAY_COUNT(entity->animation_groups) - 1);
	entity->animation_groups[index] = animation_group;
	entity->animation_group_count++;
}

internal CollisionVolumeGroup* MakeCollisionVolumeGroup(PlaygroundMemoryArena* arena, u32 total_volume_count=1)
{
	CollisionVolumeGroup* volume_group = PushStruct(arena, CollisionVolumeGroup);
	volume_group->volume_count = 0;
	volume_group->total_volume_count = total_volume_count;
	volume_group->volumes = PushArray(arena, volume_group->total_volume_count, CollisionVolume);
	// volume_group->total_volume.offset_position = v2(0.5f * dimension.x,
	// 												0.5f * dimension.y);
	// volume_group->total_volume.dimension = dimension;
	// volume_group->volumes[0] = volume_group->total_volume;

	return volume_group;
}

internal void AddCollisionVolumeToCollisionVolumeGroup(CollisionVolumeGroup* collision_volume_group, v2 dimension, v2 offset=v2())
{
	ASSERT(collision_volume_group->volume_count < collision_volume_group->total_volume_count);

	CollisionVolume* collision_volume = collision_volume_group->volumes + collision_volume_group->volume_count++;
	collision_volume->offset_position = offset + dimension * 0.5f;
	// v2(0.5f * dimension.x,
	//    0.5f * dimension.y);
	collision_volume->dimension = dimension;
	collision_volume_group->total_volume.offset_position += collision_volume->offset_position;
	collision_volume_group->total_volume.dimension += dimension;
	// volume_group->volumes[0] = volume_group->total_volume;
}

inline Entity* GetEntity(World* world, u32 entity_index);
internal void UpdateEntityTileMapAndTilePosition(PlaygroundState* playground_state, World* world, Entity* entity, u32 entity_index, TilePosition* new_tile_position);

inline void AddFlags(Entity* entity, u32 flags)
{
	entity->flags |= flags;
}

inline void ClearFlags(Entity* entity, u32 flags)
{
	entity->flags &= ~flags;
}

inline b32 IsFlagSet(Entity* entity, u32 flags)
{
	b32 result = entity->flags & flags;

	return result;
}

inline void MakeEntityNonspatial(Entity* entity)
{
	AddFlags(entity, EntityFlag::NONSPATIAL_FLAG);
	entity->position = InvalidPosition();
}

inline void MakeEntityNonspatialAndDeleteFromTileMap(PlaygroundState* playground_state, World* world, Entity* entity, u32 entity_index)
{
	TilePosition invalid_tile_position = InvalidTilePosition();
	MakeEntityNonspatial(entity);
	UpdateEntityTileMapAndTilePosition(playground_state, world, entity, entity_index, &invalid_tile_position);
	entity->tile_position = invalid_tile_position;
}

inline void MakeEntitySpatial(Entity* entity, v2 direction, v2 position, v2 velocity)
{
	ClearFlags(entity, EntityFlag::NONSPATIAL_FLAG);
	// entity->direction = direction;
	entity->position = position;
	entity->velocity = velocity;
}

inline void MakeEntitySpatialAndAddToTileMap(PlaygroundState* playground_state, World* world, Entity* entity, u32 entity_index, v2 direction, v2 position, v2 velocity)
{
	MakeEntitySpatial(entity, direction, position, velocity);

	TilePosition new_tile_position = MapIntoTilePosition(world->camera, entity->position, world->tile_side_in_meters);
	UpdateEntityTileMapAndTilePosition(playground_state, world, entity, entity_index, &new_tile_position);

}

inline v2 GetEntityAlignedPosition(Entity* entity)
{
	v2 result = entity->position +
		entity->collision_volume_group->volumes[0].offset_position;

		// entity->collision_volume_group->total_volume.offset_position;

	return result;
}

inline v2 GetEntityDimension(Entity* entity)
{
	v2 result = entity->collision_volume_group->total_volume.dimension;

	return result;
}

inline v2 GetEntityOffsetPosition(Entity* entity)
{
	v2 result = entity->collision_volume_group->total_volume.offset_position;

	return result;
}

inline b32 IsEntityNewPositionInRectangle2(Entity* entity, Rectangle2 rect, v2 position)
{
	v2 entity_dimension = GetEntityDimension(entity);
	Rectangle2 new_rect = AddDimensionTo(rect, 0.5f * entity_dimension);
	v2 entity_offset_position = GetEntityOffsetPosition(entity);
	b32 result = IsInRectangle2(new_rect, position + entity_offset_position);

	return result;

}

internal void SetCameraLocationAndUpdateEntities(World* world, TilePosition new_camera, f32 delta_time)
{
	world->active_entity_count = 0;

	f32 max_entity_dimension = // (f32)world->tile_count_x +
		10.0f;
	f32 max_entity_velocity = 30.0f;
	f32 max_bound = max_entity_dimension + max_entity_velocity * delta_time;
	u32 camera_span_x = world->tile_count_x * 3;
	u32 camera_span_y = world->tile_count_y;

	Rectangle2 updatable_bounds =
		Rectangle2CenterDimension(v2(0.0f, 0.0f),
								  world->tile_side_in_meters *
								  v2((f32)camera_span_x, (f32)camera_span_y));
	Rectangle2 camera_bounds = AddDimensionTo(updatable_bounds,
											  0.5f * v2(max_bound, 0.0f));


	i32 lower_limit = (i32)(world->camera.tile_x - ((f32)camera_span_x * 0.5f));
	i32 upper_limit = (i32)(world->camera.tile_x + ((f32)camera_span_x * 0.5f));
	i32 min_tile_map_index = GetTileMapIndex(world, lower_limit);
	i32 max_tile_map_index = GetTileMapIndex(world, upper_limit);

	for (i32 tile_map_index = min_tile_map_index; tile_map_index <= max_tile_map_index; ++tile_map_index)  {
		TileMap* tile_map = GetTileMap(world, tile_map_index);
		if (tile_map) {
			for (EntityBlock* block = tile_map->first_entity_block; block; block = block->next_entity_block) {
				for (u32 entity_index_index = 0; entity_index_index < block->entity_index_count; ++entity_index_index) {
					u32 entity_index = block->entity_indices[entity_index_index];
					Entity* entity = GetEntity(world, entity_index);

					if (!IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG)) {
						v2 new_position = TilePositionDifference(entity->tile_position, world->camera, world->tile_side_in_meters);
						if (IsEntityNewPositionInRectangle2(entity, camera_bounds, new_position)) {
							ASSERT(world->active_entity_count < ARRAY_COUNT(world->active_entity_indices) - 1);

							entity->position = new_position;
							world->active_entity_indices[world->active_entity_count++] = entity_index;
							entity->updatable = IsInRectangle2(updatable_bounds, entity->position);
						}
					}
				}
			}
		}
	}
}

internal void UpdateEntityTileMapAndTilePosition(PlaygroundState* playground_state, World* world, Entity* entity, u32 entity_index, TilePosition* new_tile_position)
{
	TilePosition* old_tile_position = &entity->tile_position;

	if (old_tile_position &&
		new_tile_position &&
		AreOnSameTileMap(world, old_tile_position, new_tile_position)) {
		// NOTE(SSJSR): Do not have to move entity between tile maps.

		*old_tile_position = *new_tile_position;
	}
	else {
		if (old_tile_position && !IsTilePositionInvalid(old_tile_position)) {
			i32 old_tile_map_index = GetTileMapIndex(world, old_tile_position->tile_x);
			TileMap* old_tile_map = GetTileMap(world, old_tile_map_index);
			if (old_tile_map) {
				b32 entity_found = false;
				EntityBlock* first_entity_block = old_tile_map->first_entity_block;
				for (EntityBlock* block = first_entity_block;
					 block && !entity_found;
					 block = block->next_entity_block) {
					for (u32 block_entity_index_index = 0;
						 (block_entity_index_index < block->entity_index_count) && (!entity_found);
						 ++block_entity_index_index) {
						u32 block_entity_index = block->entity_indices[block_entity_index_index];
						if (entity_index == block_entity_index) {

							u32* last_entity_index = first_entity_block->entity_indices + (--first_entity_block->entity_index_count);
							block->entity_indices[block_entity_index_index] = *last_entity_index;
							*last_entity_index = 0;

							if (first_entity_block->entity_index_count == 0) {
								EntityBlock* removed_block = first_entity_block;
								old_tile_map->first_entity_block = first_entity_block->next_entity_block;

								removed_block->next_entity_block = world->first_free_entity_block;
								world->first_free_entity_block = removed_block;
							}

							entity_found = true;
						}
					}
				}
			}
		}

		if (new_tile_position && !IsTilePositionInvalid(new_tile_position)) {
			i32 new_tile_map_index = GetTileMapIndex(world, new_tile_position->tile_x);
			TileMap* new_tile_map = GetTileMap(world, new_tile_map_index);
			if (new_tile_map) {
				EntityBlock* first_entity_block = new_tile_map->first_entity_block;
				if (!first_entity_block ||
					first_entity_block->entity_index_count >= ARRAY_COUNT(first_entity_block->entity_indices)) {

					// TODO(SSJSR): We should test this if block.
					if (world->first_free_entity_block) {
						first_entity_block = world->first_free_entity_block;
						first_entity_block->next_entity_block = 0;
						world->first_free_entity_block = world->first_free_entity_block->next_entity_block;
					}
					else {
						first_entity_block = PushStruct(&playground_state->arena, EntityBlock);
					}

					first_entity_block->next_entity_block = new_tile_map->first_entity_block;
					new_tile_map->first_entity_block = first_entity_block;
				}

				*old_tile_position = *new_tile_position;
				first_entity_block->entity_indices[first_entity_block->entity_index_count++] = entity_index;
			}
		}
	}
}

inline b32 TestWall(f32 wall_x,
					f32 tile_relative_position_x, f32 tile_relative_position_y,
					f32 wall_min_corner_y, f32 wall_max_corner_y,
					f32 player_position_delta_x,
					f32 player_position_delta_y,
					f32* time_minimum)
{
	f32 epsilon = 0.001f;
	b32 hit = false;
	if (player_position_delta_x != 0.0f) {
		f32 time_solved = (wall_x - tile_relative_position_x) / player_position_delta_x;
		f32 y = tile_relative_position_y + time_solved * player_position_delta_y;
		if (time_solved >= 0.0f && *time_minimum > time_solved) {
			if (y >= wall_min_corner_y && y <= wall_max_corner_y) {
				*time_minimum = MAXIMUM(0.0f, time_solved - epsilon);
				hit = true;
			}
		}
	}

	return hit;
}

// internal void AddCollisionPair(PlaygroundMemoryArena* arena, World* world,
// 							   u32 first_entity_index, u32 second_entity_index,
// 							   b32 can_collide)
// {
// 	if (first_entity_index != second_entity_index) {
// 		if (first_entity_index > second_entity_index) {
// 			SWAP(first_entity_index, second_entity_index, u32);
// 		}

// 		u32 hash_index = first_entity_index & (ARRAY_COUNT(world->collision_pair_hash) - 1);
// 		CollisionPair* found = 0;
// 		for (CollisionPair* collision_pair = world->collision_pair_hash[hash_index];
// 			 collision_pair;
// 			 collision_pair = collision_pair->next_collision_pair) {
// 			if (collision_pair->first_entity_index == first_entity_index &&
// 				collision_pair->second_entity_index == second_entity_index) {
// 				found = collision_pair;
// 				break;
// 			}
// 		}

// 		if (!found) {
// 			found = world->first_free_collision_pair;
// 			if (!found) {
// 				found = PushStruct(arena, CollisionPair);
// 			}
// 			else {
// 				world->first_free_collision_pair = found->next_collision_pair;
// 				found->next_collision_pair = 0;
// 			}

// 			found->next_collision_pair = world->collision_pair_hash[hash_index];
// 			world->collision_pair_hash[hash_index] = found;
// 		}

// 		if (found) {
// 			found->first_entity_index = first_entity_index;
// 			found->second_entity_index = second_entity_index;
// 			found->can_collide = can_collide;
// 		}
// 	}
// }

internal void AddCollisionPair(PlaygroundMemoryArena* arena, World* world,
								u32 first_entity_index, u32 second_entity_index,
								b32 can_collide)
{
	if (first_entity_index != second_entity_index) {
		// if (first_entity_index > second_entity_index) {
		// 	SWAP(first_entity_index, second_entity_index, u32);
		// }

		u32 hash_indices[2] = {
			first_entity_index & (ARRAY_COUNT(world->collision_pair_hash) - 1),
			second_entity_index & (ARRAY_COUNT(world->collision_pair_hash) - 1)
		};
		CollisionPair* pairs[2] = {};
		for (u32 pair_index = 0; pair_index < 2; ++pair_index) {
			u32 hash_index = hash_indices[pair_index];
			for (CollisionPair* collision_pair = world->collision_pair_hash[hash_index];
				 collision_pair;
				 collision_pair = collision_pair->next_collision_pair) {
				if (collision_pair->first_entity_index == first_entity_index &&
					collision_pair->second_entity_index == second_entity_index) {
					pairs[pair_index] = collision_pair;
					break;
				}
				else if (collision_pair->first_entity_index == second_entity_index &&
						 collision_pair->second_entity_index == first_entity_index) {
					pairs[pair_index] = collision_pair;
					break;
				}
			}
		}

		if (!pairs[0] && !pairs[1]) {
			ASSERT(!pairs[0] && !pairs[1]);
			for (u32 pair_index = 0; pair_index < 2; ++pair_index) {
				u32 hash_index = hash_indices[pair_index];
				
				pairs[pair_index] = world->first_free_collision_pair;
				if (!pairs[pair_index]) {
					pairs[pair_index] = PushStruct(arena, CollisionPair);
				}
				else {
					world->first_free_collision_pair = pairs[pair_index]->next_collision_pair;
				}

				pairs[pair_index]->next_collision_pair = world->collision_pair_hash[hash_index];
				world->collision_pair_hash[hash_index] = pairs[pair_index];

				ASSERT(pairs[pair_index]);
				if (pair_index == 0) {
					pairs[pair_index]->first_entity_index = first_entity_index;
					pairs[pair_index]->second_entity_index = second_entity_index;
				}
				else if (pair_index == 1) {
					pairs[pair_index]->first_entity_index = second_entity_index;
					pairs[pair_index]->second_entity_index = first_entity_index;
				}
				
				pairs[pair_index]->can_collide = can_collide;
			}
		}

		ASSERT(pairs[0] && pairs[1]);
		if (pairs[0] && pairs[1]) {
			pairs[0]->first_entity_index = first_entity_index;
			pairs[0]->second_entity_index = second_entity_index;
			pairs[0]->can_collide = can_collide;

			pairs[1]->first_entity_index = second_entity_index;
			pairs[1]->second_entity_index = first_entity_index;
			pairs[1]->can_collide = can_collide;
		}
	}
}

// internal void ClearCollisionPairs(World* world,
// 								  u32 entity_index)
// {
// 	u32 hash_index = entity_index & (ARRAY_COUNT(world->collision_pair_hash) - 1);
// 	for (CollisionPair** collision_pair = world->collision_pair_hash + hash_index;
// 		 *collision_pair;
// 		 ) {
// 		if ((*collision_pair)->first_entity_index == entity_index ||
// 			(*collision_pair)->second_entity_index == entity_index) {

// 			CollisionPair* deleted_collision_pair = *collision_pair;
// 			*collision_pair = (*collision_pair)->next_collision_pair;

// 			deleted_collision_pair->next_collision_pair = world->first_free_collision_pair;
// 			world->first_free_collision_pair = deleted_collision_pair;
// 		}
// 		else {
// 			collision_pair = &(*collision_pair)->next_collision_pair;
// 		}
// 	}
// }

internal void ClearCollisionPairs(World* world,
								  u32 entity_index)
{
	u32 hash_index = entity_index & (ARRAY_COUNT(world->collision_pair_hash) - 1);
	CollisionPair* saved_first_free_collision_pair = world->first_free_collision_pair;
	for (CollisionPair** collision_pair = world->collision_pair_hash + hash_index;
		 *collision_pair;
		 ) {
		if ((*collision_pair)->first_entity_index == entity_index) {

			CollisionPair* deleted_collision_pair = *collision_pair;
			*collision_pair = (*collision_pair)->next_collision_pair;

			deleted_collision_pair->next_collision_pair = world->first_free_collision_pair;
			world->first_free_collision_pair = deleted_collision_pair;
		}
		else {
			collision_pair = &(*collision_pair)->next_collision_pair;
		}
	}

	for (CollisionPair* freed_collision_pair = world->first_free_collision_pair;
		 freed_collision_pair != saved_first_free_collision_pair;
		 freed_collision_pair = freed_collision_pair->next_collision_pair) {
		
		hash_index = freed_collision_pair->second_entity_index & (ARRAY_COUNT(world->collision_pair_hash) - 1);
		for (CollisionPair** collision_pair = world->collision_pair_hash + hash_index;
			 *collision_pair;
			 ) {
			if ((*collision_pair)->second_entity_index == entity_index) {

				CollisionPair* deleted_collision_pair = *collision_pair;
				*collision_pair = (*collision_pair)->next_collision_pair;

				deleted_collision_pair->next_collision_pair = world->first_free_collision_pair;
				world->first_free_collision_pair = deleted_collision_pair;
			}
			else {
				collision_pair = &(*collision_pair)->next_collision_pair;
			}
		}
	}
}


internal b32 CanCollide(World* world,
						Entity* first_entity, u32 first_entity_index,
						Entity* second_entity, u32 second_entity_index)
{
	b32 result = false;

	if (first_entity_index != second_entity_index) {

		if (first_entity_index > second_entity_index) {
			SWAP(first_entity_index, second_entity_index, u32);
		}

		if (!IsFlagSet(first_entity, EntityFlag::NONSPATIAL_FLAG) &&
			!IsFlagSet(second_entity, EntityFlag::NONSPATIAL_FLAG)) {
			result = true;
		}

		if (first_entity->type == EntityType::PLAYER_TYPE &&
			second_entity->type == EntityType::SWORD_TYPE) {
			result = false;
		}

		u32 hash_index = first_entity_index & (ARRAY_COUNT(world->collision_pair_hash) - 1);
		ASSERT(hash_index < ARRAY_COUNT(world->collision_pair_hash));
		for (CollisionPair* collision_pair = world->collision_pair_hash[hash_index];
			 collision_pair;
			 collision_pair = collision_pair->next_collision_pair) {
			if (collision_pair->first_entity_index == first_entity_index &&
				collision_pair->second_entity_index == second_entity_index) {
				result = collision_pair->can_collide;
				break;
			}
		}
	}

	return result;
}

internal b32 StopsOnCollision(PlaygroundMemoryArena* arena, World* world,
							  Entity* first_entity, u32 first_entity_index,
							  Entity* second_entity, u32 second_entity_index) {

	b32 result = true;

	if (first_entity_index != second_entity_index) {

		if (first_entity->type > second_entity->type) {
			SWAP(first_entity_index, second_entity_index, u32);
			SWAP(first_entity, second_entity, Entity*);
		}

		if (first_entity->type == EntityType::FAMILIAR_TYPE ||
			second_entity->type == EntityType::FAMILIAR_TYPE) {
			result = false;
			AddCollisionPair(arena, world, first_entity_index, second_entity_index, result);
		}

		if (first_entity->type == EntityType::BALL_TYPE ||
			second_entity->type == EntityType::BALL_TYPE) {
			result = false;
			AddCollisionPair(arena, world, first_entity_index, second_entity_index, result);
		}

		if (first_entity->type == EntityType::PLAYER_TYPE &&
			second_entity->type == EntityType::MONSTER_TYPE) {
			result = false;
			AddCollisionPair(arena, world, first_entity_index, second_entity_index, result);
		}

		if (first_entity->type == EntityType::MONSTER_TYPE &&
			second_entity->type == EntityType::SWORD_TYPE) {
			result = false;
			AddCollisionPair(arena, world, first_entity_index, second_entity_index, result);
			first_entity->velocity.y = 10.0f;
			if (second_entity->position.x > first_entity->position.x) {
				first_entity->velocity.x = -5.0f;
			}
			else {
				first_entity->velocity.x = 5.0f;
			}
		}
	}

	return result;
}

internal void MoveEntity(PlaygroundState* playground_state, World* world, u32 entity_index, Entity* entity, f32 delta_time_for_frame, MoveFeature* move_feature)
{
	v2 direction = move_feature->direction;

	if (move_feature->max_unit_vector_length) {
		f32 direction_length = Length(direction);
		if (direction_length > 1.0f) {
			f32 ratio = 1.0f / direction_length;
			direction *= ratio;
		}
	}

	v2 acceleration = HadamardProduct(move_feature->acceleration, direction);

	// v2 player_acceleration = v2(50.0f, 50.0f);

	// player_acceleration.x *= player_direction.x;
	// player_acceleration.y *= player_direction.y;

	// acceleration += move_feature->friction_coefficient * -entity->velocity;
	acceleration.x += move_feature->friction_coefficient * -entity->velocity.x;

	v2 entity_position_delta = (0.5f * acceleration * delta_time_for_frame * delta_time_for_frame) +
		(entity->velocity * delta_time_for_frame);

	entity->velocity =
		(acceleration * delta_time_for_frame) +
		(entity->velocity);

	// NOTE(SSJSR): If distance_remaining is 0, it means there is no limit.
	f32 distance_remaining = entity->distance_limit;
	if (distance_remaining == 0.0f) {
		distance_remaining = 10000.0f;
	}

	b32 hitted = false;
	for (u32 iteration = 0; iteration < 4; ++iteration) {
		f32 time_minimum = 1.0f;
		b32 hitted_side_wall = false;

		f32 entity_position_delta_length = Length(entity_position_delta);
		if (entity_position_delta_length > 0.0f) {
			if (entity_position_delta_length > distance_remaining) {
				time_minimum = distance_remaining / entity_position_delta_length;
			}

			Entity* hit_entity = 0;
			u32 hit_entity_index = 0;
			v2 wall_normal = v2(0.0f, 0.0f);

			v2 target_position = entity->position + entity_position_delta;

			for (u32 test_entity_index_index = 0;
				 test_entity_index_index < world->active_entity_count;
				 ++test_entity_index_index) {

				u32 test_entity_index = world->active_entity_indices[test_entity_index_index];

				// if (IsFlagSet(entity, EntityFlag::COLLIDES_FLAG) && !IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG)) {
				// 	if (test_entity_index != entity_index) {
				Entity* test_entity = GetEntity(world, test_entity_index);
				if (CanCollide(world, entity, entity_index, test_entity, test_entity_index)) {

					// if (IsFlagSet(test_entity, EntityFlag::COLLIDES_FLAG) && !IsFlagSet(test_entity, EntityFlag::NONSPATIAL_FLAG)) {

					for (u32 volume_group_index = 0;
						 volume_group_index < entity->collision_volume_group->volume_count;
						 ++volume_group_index) {
						CollisionVolume* collision_volume = entity->collision_volume_group->volumes + volume_group_index;

						for (u32 test_volume_group_index = 0;
							 test_volume_group_index < test_entity->collision_volume_group->volume_count;
							 ++test_volume_group_index) {
							CollisionVolume* test_collision_volume = test_entity->collision_volume_group->volumes + test_volume_group_index;

							v2 diameter = collision_volume->dimension + test_collision_volume->dimension;

							v2 wall_min_corner = -0.5f * diameter;
							v2 wall_max_corner = 0.5f * diameter;

							v2 tile_relative_position = ((entity->position + collision_volume->offset_position) -
														 (test_entity->position + test_collision_volume->offset_position));

							if (TestWall(wall_min_corner.x,
										 tile_relative_position.x, tile_relative_position.y,
										 wall_min_corner.y, wall_max_corner.y,
										 entity_position_delta.x,
										 entity_position_delta.y,
										 &time_minimum)) {
								wall_normal = v2(-1.0f, 0.0f);
								hit_entity = test_entity;
								hit_entity_index = test_entity_index;
								hitted_side_wall = true;
							}

							if (TestWall(wall_max_corner.x,
										 tile_relative_position.x, tile_relative_position.y,
										 wall_min_corner.y, wall_max_corner.y,
										 entity_position_delta.x,
										 entity_position_delta.y,
										 &time_minimum)) {
								wall_normal = v2(1.0f, 0.0f);
								hit_entity = test_entity;
								hit_entity_index = test_entity_index;
								hitted_side_wall = true;
							}

							if (TestWall(wall_min_corner.y,
										 tile_relative_position.y, tile_relative_position.x,
										 wall_min_corner.x, wall_max_corner.x,
										 entity_position_delta.y,
										 entity_position_delta.x,
										 &time_minimum)) {
								wall_normal = v2(0.0f, -1.0f);
								hit_entity = test_entity;
								hit_entity_index = test_entity_index;
							}

							if (TestWall(wall_max_corner.y,
										 tile_relative_position.y, tile_relative_position.x,
										 wall_min_corner.x, wall_max_corner.x,
										 entity_position_delta.y,
										 entity_position_delta.x,
										 &time_minimum)) {
								wall_normal = v2(0.0f, 1.0f);
								hit_entity = test_entity;
								hit_entity_index = test_entity_index;
							}
						}
					}
				}
			}

			entity->position = (entity_position_delta * time_minimum) + entity->position;
			distance_remaining -= entity_position_delta_length * time_minimum;
			if (hit_entity) {
				entity_position_delta = target_position - entity->position;

				b32 stops_on_collision = StopsOnCollision(&playground_state->arena, world, entity, entity_index, hit_entity, hit_entity_index);
				if (stops_on_collision) {
					entity->velocity = entity->velocity - 1.0f * Dot(entity->velocity, wall_normal) * wall_normal;
					entity_position_delta = entity_position_delta - 1.0f * Dot(entity_position_delta, wall_normal) * wall_normal;
				}

				if (hitted_side_wall) {
					hitted = true;
				}
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	if (hitted) {
		entity->current_action = WALL_SLIDE_ACTION;
		entity->jump_count = 1;
	}
	else if (entity->velocity.y < 0.0f) {
		entity->current_action = FALL_ACTION;
	}

	if (entity->distance_limit != 0.0f) {
		entity->distance_limit = distance_remaining;
	}

	// if (hitted && entity->type == EntityType::BALL_TYPE) {
	// 	MakeEntityNonspatialAndDeleteFromTileMap(playground_state, world, entity, entity_index);
	// }

	if (entity->direction.x < 0.0f) {
		entity->facing_direction = 1; // Left
	}
	else if (entity->direction.x > 0.0f) {
		entity->facing_direction = 2; // Right
	}
}

internal u32 AddEntity(PlaygroundState* playground_state, World* world, EntityType type, TilePosition* tile_position)
{
	ASSERT(world->entity_count < ARRAY_COUNT(world->entities));

	u32 entity_index = world->entity_count++;

	Entity* entity = world->entities + entity_index;
	*entity = {};
	entity->type = type;
	entity->tile_position = InvalidTilePosition();

	if (tile_position) {
		UpdateEntityTileMapAndTilePosition(playground_state, world, entity, entity_index, tile_position);
	}
	else {
		MakeEntityNonspatial(entity);
	}
	return entity_index;
}

inline Entity* GetEntity(World* world, u32 entity_index)
{
	ASSERT(entity_index < world->entity_count);

	Entity* entity = 0;

	if (entity_index < world->entity_count) {
		entity = world->entities + entity_index;
	}

	return entity;
}

internal u32 AddBall(PlaygroundState* playground_state)
{
	World* world = &playground_state->world;
	v2 dimension = v2(0.5f, 0.5f);

	u32 entity_index = AddEntity(playground_state, world, EntityType::BALL_TYPE, 0);
	Entity* entity = GetEntity(world, entity_index);

	entity->collision_volume_group =
		MakeCollisionVolumeGroup(&playground_state->arena);
	AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
											 dimension);
	AddFlags(entity, EntityFlag::COLLIDES_FLAG | EntityFlag::MOVEABLE_FLAG);

	return entity_index;
}

internal u32 AddSword(PlaygroundState* playground_state)
{
	World* world = &playground_state->world;
	v2 dimension = v2(0.25f, 0.25f);

	u32 entity_index = AddEntity(playground_state, world, EntityType::SWORD_TYPE, 0);
	Entity* entity = GetEntity(world, entity_index);

	// entity->collision_volume_group =
	// 	MakeCollisionVolumeGroup(&playground_state->arena, 5);
	// AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
	// 										 v2(0.125f, 1.1f));
	// AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
	// 										 v2(0.125f, 1.0f), v2(0.125f, 0.05f));
	// AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
	// 										 v2(0.125f, 0.9f), v2(0.250f, 0.10f));
	// AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
	// 										 v2(0.125f, 0.75f), v2(0.375f, 0.15f));
	// AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
	// 										 v2(0.125f*0.5f, 0.60f), v2(0.500f, 0.20f));

	AddFlags(entity, EntityFlag::COLLIDES_FLAG | EntityFlag::MOVEABLE_FLAG);

	return entity_index;
}

internal u32 AddPlayer(PlaygroundState* playground_state)
{
	World* world = &playground_state->world;

	TilePosition tile_position = {};
	tile_position.tile_x = 6;
	tile_position.tile_y = 5;
	v2 dimension = v2(0.6f, 0.9f);

	u32 entity_index = AddEntity(playground_state, world, EntityType::PLAYER_TYPE, &tile_position);
	Entity* entity = GetEntity(world, entity_index);

	entity->collision_volume_group =
		MakeCollisionVolumeGroup(&playground_state->arena);
	AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
											 dimension);

	AddFlags(entity, EntityFlag::COLLIDES_FLAG | EntityFlag::MOVEABLE_FLAG);

	entity->ball_index = AddBall(playground_state);
	entity->sword_index = AddSword(playground_state);

	AddAnimationGroup(entity, playground_state->player_idle_animations);
	AddAnimationGroup(entity, playground_state->player_run_animations);
	AddAnimationGroup(entity, playground_state->player_jump_animations);
	AddAnimationGroup(entity, playground_state->player_jump_2_animations);
	AddAnimationGroup(entity, playground_state->player_fall_animations);
	AddAnimationGroup(entity, playground_state->player_wall_slide_animations);
	AddAnimationGroup(entity, playground_state->player_attack_1_animations);
	AddAnimationGroup(entity, playground_state->player_attack_2_animations);

	entity->current_action = FALL_ACTION;

	return entity_index;
}

internal u32 AddFamiliar(PlaygroundState* playground_state)
{
	World* world = &playground_state->world;
	Entity* player_entity = GetEntity(world, world->player_entity_index);
	TilePosition tile_position = player_entity->tile_position;
	v2 dimension = v2(0.5f, 0.6f);

	u32 entity_index = AddEntity(playground_state, world, EntityType::FAMILIAR_TYPE, &tile_position);
	Entity* entity = GetEntity(world, entity_index);

	entity->collision_volume_group =
		MakeCollisionVolumeGroup(&playground_state->arena);
	AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
											 dimension);

	AddFlags(entity, EntityFlag::MOVEABLE_FLAG);

	AddAnimationGroup(entity, playground_state->familiar_idle_animations);
	AddAnimationGroup(entity, playground_state->familiar_run_animations);

	return entity_index;
}

internal u32 AddWall(PlaygroundState* playground_state, i32 tile_x, i32 tile_y, f32 width, f32 height)
{
	World* world = &playground_state->world;
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;
	v2 dimension = v2(width, height);

	u32 entity_index = AddEntity(playground_state, world, EntityType::WALL_TYPE, &tile_position);
	Entity* entity = GetEntity(world, entity_index);

	entity->collision_volume_group =
		MakeCollisionVolumeGroup(&playground_state->arena);
	AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
											 dimension);

	AddFlags(entity, EntityFlag::COLLIDES_FLAG);

	return entity_index;
}

internal u32 AddMonster(PlaygroundState* playground_state, i32 tile_x, i32 tile_y)
{
	World* world = &playground_state->world;
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;
	v2 dimension = v2(0.6f, 0.9f);

	u32 entity_index = AddEntity(playground_state, world, EntityType::MONSTER_TYPE, &tile_position);
	Entity* entity = GetEntity(world, entity_index);

	entity->collision_volume_group =
		MakeCollisionVolumeGroup(&playground_state->arena);
	AddCollisionVolumeToCollisionVolumeGroup(entity->collision_volume_group,
											 dimension);

	AddFlags(entity, EntityFlag::COLLIDES_FLAG);

	return entity_index;
}

internal AnimationGroup* UpdateAnimationGroup(Entity* entity, u32 animation_group_index, f32 delta_time_for_frame)
{
	ASSERT(animation_group_index > AnimationType::NULL_ANIMATION_TYPE);
	ASSERT(animation_group_index < AnimationType::MAX_ANIMATION_TYPE);

	AnimationGroup* entity_animation_group = entity->animation_groups[animation_group_index];
	for (u32 animation_index = 0;
		 animation_index < entity_animation_group->animation_count;
		 ++animation_index) {
		Animation* entity_animation = entity_animation_group->animations + animation_index;
		f32 seconds_per_frame = entity_animation->duration / (f32)entity_animation->frame_count;
		// entity_animation->total_elapsed_time += delta_time_for_frame;
		// entity_animation->frame_index = (u32)(entity_animation->total_elapsed_time / seconds_per_frame);

		entity->state_time += delta_time_for_frame;
		entity->animation_frame_index = (u32)(entity->state_time / seconds_per_frame);

		// if (entity_animation->total_elapsed_time >= entity_animation->duration) {
		if (entity->state_time >= entity_animation->duration) {

			// entity_animation->total_elapsed_time = 0.0;
			entity->state_time = 0.0f;
			entity->animation_frame_index = 0;
		}
	}

	return entity_animation_group;
}

internal AnimationGroup* EntityStateControl(PlaygroundState* playground_state,
											Entity* entity,
											PlaygroundInput* input=0)
{
	ASSERT(input);

	AnimationGroup* entity_animation_group = 0;
	u32 animation_group_index = AnimationType::IDLE_ANIMATION_TYPE;

	if (entity->type == EntityType::PLAYER_TYPE) {

		entity->direction = v2(0.0f, -1.0f);
		f32 duration = 0.2f;
		f32 jump_height = 1.5f;
		f32 jump_speed = (2.0f * jump_height) / duration;
		f32 gravity = (2.0f * jump_height) / Square(duration);
		entity->acceleration.y = gravity;

		// f32 duration = 0.2f;
		// f32 jump_height = 1.5f;
		// f32 jump_speed = (2.0f * jump_height) / duration;
		// f32 gravity = (2.0f * jump_height) / Square(duration);
		// entity->acceleration.y = gravity;

		// entity->direction.y = 1.0f;
		// entity->velocity.y = jump_speed;

		u32 event = EVENT_ANY;

		if (input->move_left.is_down) {
			event = EVENT_LEFT_KEY;
			entity->direction.x = -1.0f;
		}

		if (input->move_right.is_down) {
			event = EVENT_RIGHT_KEY;
			entity->direction.x = 1.0f;
		}

		if (input->move_up.is_down) {
			event = EVENT_UP_KEY;
		}

		if (input->attack.is_down) {
			event = EVENT_ATTACK_1;
		}

		if (input->move_right.is_released || input->move_left.is_released) {
			event = EVENT_STOP;
		}

		// if (entity->velocity.y == 0.0f && entity->current_action != ATTACK_1_ACTION) {
		// 	entity->current_action = IDLE_ACTION;
		// }

		if (entity->current_action == IDLE_ACTION) {
			entity->jump_count = 0;

			if (event == EVENT_RIGHT_KEY || event == EVENT_LEFT_KEY) {
				entity->current_action = RUN_ACTION;
			}
			else if (event == EVENT_UP_KEY) {
				entity->current_action = JUMP_ACTION;

				entity->direction.y = 1.0f;
				entity->velocity.y = jump_speed;
				++entity->jump_count;
			}
			else if (event == EVENT_ATTACK_1) {
				entity->current_action = ATTACK_1_ACTION;
			}
		}
		else if (entity->current_action == RUN_ACTION) {
			entity->jump_count = 0;

			if (event == EVENT_RIGHT_KEY || event == EVENT_LEFT_KEY) {
				entity->current_action = RUN_ACTION;
			}
			else if (event == EVENT_STOP) {
				entity->current_action = IDLE_ACTION;
			}
			else if (event == EVENT_UP_KEY) {
				entity->current_action = JUMP_ACTION;

				entity->direction.y = 1.0f;
				entity->velocity.y = jump_speed;
				++entity->jump_count;
			}
			else if (event == EVENT_ATTACK_1) {
				entity->current_action = ATTACK_1_ACTION;
			}
		}
		else if (entity->current_action == FALL_ACTION) {
			if (event == EVENT_UP_KEY && entity->jump_count == 1) {
				entity->current_action = JUMP_2_ACTION;

				entity->direction.y = 1.0f;
				entity->velocity.y = jump_speed;
				++entity->jump_count;
			}
			else if (entity->velocity.y == 0.0f) {
				entity->current_action = IDLE_ACTION;
			}
		}
		else if (entity->current_action == WALL_SLIDE_ACTION) {
			if (entity->velocity.y == 0.0f) {
				entity->current_action = IDLE_ACTION;
			}
			else {
				entity->velocity.y *= 0.8f;
			}
		}
		else if (entity->current_action == ATTACK_1_ACTION) {
			if (entity->state_time == 0.0f) {
				if (event == EVENT_ATTACK_1) {
					entity->current_action = ATTACK_2_ACTION;
				}
				else {
					entity->current_action = IDLE_ACTION;
				}
			}
			else {
				entity->velocity *= 0.5f;
			}

			if (entity->animation_frame_index == 2) {
				Entity* sword_entity = GetEntity(&playground_state->world, entity->sword_index);
				sword_entity->collision_volume_group = playground_state->attack_1_collision_volume_group;
				ASSERT(sword_entity->collision_volume_group);
				v2 sword_position = v2();
				b32 left_facing = entity->facing_direction == 1;
				for (u32 collision_volume_index = 0;
					 collision_volume_index < sword_entity->collision_volume_group->volume_count;
					 ++collision_volume_index) {
					CollisionVolume* collision_volume = sword_entity->collision_volume_group->volumes + collision_volume_index;
					collision_volume->offset_position.x = left_facing ?
						-AbsoluteOf(collision_volume->offset_position.x) :
						AbsoluteOf(collision_volume->offset_position.x);
				}
				if (left_facing) {
					sword_position = entity->position + v2(0.1f, 0.12f);
				}
				else {
					sword_position = entity->position + v2(0.52f, 0.12f);
				}

				MakeEntitySpatialAndAddToTileMap(playground_state,
												 &playground_state->world,
												 sword_entity, entity->sword_index,
												 v2(0.0f, 0.0f), sword_position, v2());
			}
			else if (entity->animation_frame_index == 3) {
				Entity* sword_entity = GetEntity(&playground_state->world, entity->sword_index);
				MakeEntityNonspatialAndDeleteFromTileMap(playground_state, &playground_state->world,
														 sword_entity, entity->sword_index);
				ClearCollisionPairs(&playground_state->world, entity->sword_index);
			}
		}

		else if (entity->current_action == ATTACK_2_ACTION) {
			if (entity->state_time == 0.0f) {
				entity->current_action = IDLE_ACTION;
			}
			else {
				entity->velocity.x *= 0.5f;
			}

			if (entity->animation_frame_index == 3) {
				Entity* sword_entity = GetEntity(&playground_state->world, entity->sword_index);
				sword_entity->collision_volume_group = playground_state->attack_2_collision_volume_group;
				ASSERT(sword_entity->collision_volume_group);
				v2 sword_position = v2();
				b32 left_facing = entity->facing_direction == 1;
				for (u32 collision_volume_index = 0;
					 collision_volume_index < sword_entity->collision_volume_group->volume_count;
					 ++collision_volume_index) {
					CollisionVolume* collision_volume = sword_entity->collision_volume_group->volumes + collision_volume_index;
					collision_volume->offset_position.x = left_facing ?
						-AbsoluteOf(collision_volume->offset_position.x) :
						AbsoluteOf(collision_volume->offset_position.x);
				}
				if (left_facing) {
					sword_position = entity->position + v2(0.1f, 0.0f);
				}
				else {
					sword_position = entity->position + v2(0.5f, 0.0f);
				}

				MakeEntitySpatialAndAddToTileMap(playground_state,
												 &playground_state->world,
												 sword_entity, entity->sword_index,
												 v2(0.0f, 0.0f), sword_position, v2());
			}

			else if (entity->animation_frame_index == 4) {
				Entity* sword_entity = GetEntity(&playground_state->world, entity->sword_index);
				MakeEntityNonspatialAndDeleteFromTileMap(playground_state, &playground_state->world,
														 sword_entity, entity->sword_index);
				ClearCollisionPairs(&playground_state->world, entity->sword_index);
			}
		}

		if (entity->last_action != entity->current_action) {
			entity->last_action = entity->current_action;
			entity->state_time = 0.0f;
		}

		switch (entity->current_action) {
			case IDLE_ACTION: {
				animation_group_index = AnimationType::IDLE_ANIMATION_TYPE;
			} break;

			case RUN_ACTION: {
				animation_group_index = AnimationType::RUN_ANIMATION_TYPE;
			} break;

			case JUMP_ACTION: {
				animation_group_index = AnimationType::JUMP_ANIMATION_TYPE;
			} break;

			case JUMP_2_ACTION: {
				animation_group_index = AnimationType::JUMP_2_ANIMATION_TYPE;
			} break;

			case JUMP_3_ACTION: {
				animation_group_index = AnimationType::JUMP_2_ANIMATION_TYPE;
			} break;

			case FALL_ACTION: {
				animation_group_index = AnimationType::FALL_ANIMATION_TYPE;
			} break;

			case FALL_2_ACTION: {
				animation_group_index = AnimationType::FALL_ANIMATION_TYPE;
			} break;

			case FALL_3_ACTION: {
				animation_group_index = AnimationType::FALL_ANIMATION_TYPE;
			} break;

			case WALL_SLIDE_ACTION: {
				animation_group_index = AnimationType::WALL_SLIDE_ANIMATION_TYPE;
			} break;

			case ATTACK_1_ACTION: {
				animation_group_index = AnimationType::ATTACK_1_ANIMATION_TYPE;
			} break;

			case ATTACK_2_ACTION: {
				animation_group_index = AnimationType::ATTACK_2_ANIMATION_TYPE;
			} break;
		}


	}
	else if (entity->type == EntityType::FAMILIAR_TYPE) {

		Entity* player_entity = GetEntity(&playground_state->world, playground_state->world.player_entity_index);
		if (player_entity->current_action == RUN_ACTION) {
			animation_group_index = AnimationType::RUN_ANIMATION_TYPE;
		}
	}

	entity_animation_group = UpdateAnimationGroup(entity, animation_group_index, input->delta_time_for_frame);

	// entity_animation = entity->animations + animation_index;
	// f32 seconds_per_frame = entity_animation->duration / (f32)entity_animation->frame_count;
	// entity_animation->total_elapsed_time += input->delta_time_for_frame;

	// entity_animation->frame_index = (u32)(entity_animation->total_elapsed_time / seconds_per_frame);

	// if (entity_animation->total_elapsed_time > entity_animation->duration) {
	// 	entity_animation->total_elapsed_time = 0.0;
	// 	entity_animation->frame_index = 0;
	// }

	return entity_animation_group;
}
