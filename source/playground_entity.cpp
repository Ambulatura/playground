internal Animation* AddAnimation(Entity* entity, AnimationType animation_type, f32 duration)
{
	Animation* animation = entity->animations + entity->animation_count++;
	
	animation->type = animation_type;
	animation->duration = duration;

	return animation;
}

internal void AddAnimationFrame(Animation* animation, LoadedBmp* sprite)
{
	animation->frames[animation->frame_count++].sprite = sprite;
}

inline Entity* GetEntity(World* world, u32 entity_index);
internal void UpdateEntityTileMapAndTilePosition(World* world, Entity* entity, u32 entity_index, TilePosition* new_tile_position);

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

inline void MakeEntityNonspatialAndDeleteFromTileMap(World* world, Entity* entity, u32 entity_index)
{
	TilePosition invalid_tile_position = InvalidTilePosition();
	MakeEntityNonspatial(entity);
	UpdateEntityTileMapAndTilePosition(world, entity, entity_index, &invalid_tile_position);
	entity->tile_position = invalid_tile_position;
}

inline void MakeEntitySpatial(Entity* entity, v2 direction, v2 position, v2 velocity)
{
	ClearFlags(entity, EntityFlag::NONSPATIAL_FLAG);
	// entity->direction = direction;
	entity->position = position;
	entity->velocity = velocity;
}

inline void MakeEntitySpatialAndAddToTileMap(World* world, Entity* entity, u32 entity_index, v2 direction, v2 position, v2 velocity)
{
	MakeEntitySpatial(entity, direction, position, velocity);

	TilePosition new_tile_position = MapIntoTilePosition(world->camera, entity->position, world->tile_side_in_meters);
	UpdateEntityTileMapAndTilePosition(world, entity, entity_index, &new_tile_position);

}

internal void SetCameraLocationAndUpdateEntities(World* world, TilePosition new_camera, f32 delta_time)
{
	world->active_entity_count = 0;

	f32 max_entity_dimension = (f32)world->tile_count_x + 
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
											  v2(max_bound, 0.0f));
		

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
						if (IsInRectangle2(camera_bounds, new_position)) {
							entity->position = new_position;
							world->active_entity_indices[world->active_entity_count++] = entity_index;
							entity->updatable = IsInRectangle2(updatable_bounds, new_position);
						}
					}
				}
			}
		}
	}
}

internal void UpdateEntityTileMapAndTilePosition(World* world, Entity* entity, u32 entity_index, TilePosition* new_tile_position)
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
						first_entity_block = PushStruct(world->world_arena, EntityBlock);
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

internal void MoveEntity(World* world, u32 entity_index, Entity* entity, f32 delta_time_for_frame, MoveFeature* move_feature)
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

		f32 entity_position_delta_length = Length(entity_position_delta);
		if (entity_position_delta_length > 0.0f) {
			if (entity_position_delta_length > distance_remaining) {
				time_minimum = distance_remaining / entity_position_delta_length;
			}

			b32 hit = false;
			v2 wall_normal = v2(0.0f, 0.0f);

			v2 target_position = entity->position + entity_position_delta;

			// for (u32 test_entity_index = 1;
			// 	 test_entity_index < world->entity_count;
			// 	 ++test_entity_index) {
			for (u32 test_entity_index_index = 0;
				 test_entity_index_index < world->active_entity_count;
				 ++test_entity_index_index) {

				u32 test_entity_index = world->active_entity_indices[test_entity_index_index];

				if (IsFlagSet(entity, EntityFlag::COLLIDES_FLAG) && !IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG)) {
					if (test_entity_index != entity_index) {
						Entity* test_entity = GetEntity(world, test_entity_index);

						if (IsFlagSet(test_entity, EntityFlag::COLLIDES_FLAG) && !IsFlagSet(test_entity, EntityFlag::NONSPATIAL_FLAG)) {
							f32 diameter_width = test_entity->width + entity->width;
							f32 diameter_height = test_entity->height + entity->height;
							v2 wall_min_corner = -0.5f * v2(diameter_width, diameter_height);
							v2 wall_max_corner = 0.5f * v2(diameter_width,  diameter_height);
							
							v2 tile_relative_position = entity->position - test_entity->position;

							if (TestWall(wall_min_corner.x,
										 tile_relative_position.x, tile_relative_position.y,
										 wall_min_corner.y, wall_max_corner.y,
										 entity_position_delta.x,
										 entity_position_delta.y,
										 &time_minimum)) {
								wall_normal = v2(-1.0f, 0.0f);
								entity->velocity.y *= 0.7f;
								hit = true;
								AddFlags(entity, EntityFlag::ON_WALL_FLAG);
							}

							if (TestWall(wall_max_corner.x,
										 tile_relative_position.x, tile_relative_position.y,
										 wall_min_corner.y, wall_max_corner.y,
										 entity_position_delta.x,
										 entity_position_delta.y,
										 &time_minimum)) {
								wall_normal = v2(1.0f, 0.0f);
								entity->velocity.y *= 0.7f;
								hit = true;
								AddFlags(entity, EntityFlag::ON_WALL_FLAG);
							}

							if (TestWall(wall_min_corner.y,
										 tile_relative_position.y, tile_relative_position.x,
										 wall_min_corner.x, wall_max_corner.x,
										 entity_position_delta.y,
										 entity_position_delta.x,
										 &time_minimum)) {
								wall_normal = v2(0.0f, -1.0f);
								hit = true;
							}

							if (TestWall(wall_max_corner.y,
										 tile_relative_position.y, tile_relative_position.x,
										 wall_min_corner.x, wall_max_corner.x,
										 entity_position_delta.y,
										 entity_position_delta.x,
										 &time_minimum)) {
								wall_normal = v2(0.0f, 1.0f);
								hit = true;
								AddFlags(entity, EntityFlag::ON_GROUND_FLAG);
							}
						}
					}
				}
			}

			entity->position = (entity_position_delta * time_minimum) + entity->position;
			distance_remaining -= entity_position_delta_length * time_minimum;
			if (hit) {
				entity->velocity = entity->velocity - 1.0f * Dot(entity->velocity, wall_normal) * wall_normal;
				entity_position_delta = target_position - entity->position;
				entity_position_delta = entity_position_delta - 1.0f * Dot(entity_position_delta, wall_normal) * wall_normal;

				hitted = true;

			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	if (entity->distance_limit != 0.0f) {
		entity->distance_limit = distance_remaining;
	}

	if (hitted && entity->type == EntityType::BALL_TYPE) {
		MakeEntityNonspatialAndDeleteFromTileMap(world, entity, entity_index);
	}

	// if (entity->velocity.x < 0.0f) {
	// 	entity->facing_direction = 1; // Left
	// }
	// else if (entity->velocity.x > 0.0f) {
	// 	entity->facing_direction = 2; // Right
	// }

	if (entity->direction.x < 0.0f) {
		entity->facing_direction = 1; // Left
	}
	else if (entity->direction.x > 0.0f) {
		entity->facing_direction = 2; // Right
	}

	if (entity->direction.y < 0 && entity->velocity.y < 0.0f) {
		AddFlags(entity, EntityFlag::FALL_FLAG);
	}
}

internal u32 AddEntity(World* world, EntityType type, TilePosition* tile_position)
{
	ASSERT(world->entity_count < ARRAY_COUNT(world->entities));

	u32 entity_index = world->entity_count++;

	Entity* entity = world->entities + entity_index;
	*entity = {};
	entity->type = type;
	entity->tile_position = InvalidTilePosition();

	if (tile_position) {
		UpdateEntityTileMapAndTilePosition(world, entity, entity_index, tile_position);
	}
	else {
		MakeEntityNonspatial(entity);
	}
	return entity_index;
}

internal u32 AddAlignedEntity(World* world, EntityType type, TilePosition* tile_position, f32 width, f32 height)
{
	u32 entity_index = 0;
	
	if (tile_position) {
		TilePosition new_tile_position =
			MapIntoTilePosition(*tile_position,
								v2(0.5f * width, 0.5f * height),
								world->tile_side_in_meters);
		entity_index = AddEntity(world, type, &new_tile_position);
	}
	else {
		entity_index = AddEntity(world, type, tile_position);
	}

	Entity* entity = GetEntity(world, entity_index);
	entity->width = width;
	entity->height = height;

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

internal u32 AddBall(World* world)
{
	// TODO(SSJSR): Think about starting position of non-spatial entities.
	f32 height = 0.5f;
	f32 width = 0.5f;
	u32 entity_index = AddAlignedEntity(world, EntityType::BALL_TYPE, 0, width, height);

	Entity* entity = GetEntity(world, entity_index);
	AddFlags(entity, EntityFlag::COLLIDES_FLAG | EntityFlag::MOVEABLE_FLAG);

	return entity_index;
}

internal u32 AddPlayer(PlaygroundState* playground_state)
{
	World* world = &playground_state->world;
	
	TilePosition tile_position = {};
	tile_position.tile_x = 6;
	tile_position.tile_y = 5;
	f32 width = 0.6f;
	f32 height = 0.9f;
	u32 entity_index = AddAlignedEntity(world, EntityType::PLAYER_TYPE, &tile_position, width, height);

	Entity* entity = GetEntity(world, entity_index);
	AddFlags(entity, EntityFlag::COLLIDES_FLAG | EntityFlag::MOVEABLE_FLAG);

	entity->ball_index = AddBall(world);

	Animation* player_idle_animation = AddAnimation(entity, AnimationType::IDLE_ANIMATION_TYPE, 0.4f);
	AddAnimationFrame(player_idle_animation, &playground_state->player_idle_00);
	AddAnimationFrame(player_idle_animation, &playground_state->player_idle_01);
	AddAnimationFrame(player_idle_animation, &playground_state->player_idle_02);
	AddAnimationFrame(player_idle_animation, &playground_state->player_idle_03);
	
	Animation* player_run_animation = AddAnimation(entity, AnimationType::RUN_ANIMATION_TYPE, 0.6f);
	AddAnimationFrame(player_run_animation, &playground_state->player_run_00);
	AddAnimationFrame(player_run_animation, &playground_state->player_run_01);
	AddAnimationFrame(player_run_animation, &playground_state->player_run_02);
	AddAnimationFrame(player_run_animation, &playground_state->player_run_03);
	AddAnimationFrame(player_run_animation, &playground_state->player_run_04);
	AddAnimationFrame(player_run_animation, &playground_state->player_run_05);

	Animation* player_jump_animation = AddAnimation(entity, AnimationType::JUMP_ANIMATION_TYPE, 0.2f);
	// AddAnimationFrame(player_jump_animation, &playground_state->player_jump_00);
	// AddAnimationFrame(player_jump_animation, &playground_state->player_jump_01);
	AddAnimationFrame(player_jump_animation, &playground_state->player_jump_02);
	AddAnimationFrame(player_jump_animation, &playground_state->player_jump_03);

	Animation* player_jump_2_animation = AddAnimation(entity, AnimationType::JUMP_2_ANIMATION_TYPE, 0.2f);
	AddAnimationFrame(player_jump_2_animation, &playground_state->player_jump_2_00);
	AddAnimationFrame(player_jump_2_animation, &playground_state->player_jump_2_01);
	AddAnimationFrame(player_jump_2_animation, &playground_state->player_jump_2_02);
	AddAnimationFrame(player_jump_2_animation, &playground_state->player_jump_2_03);

	Animation* player_fall_animation = AddAnimation(entity, AnimationType::FALL_ANIMATION_TYPE, 0.2f);
	AddAnimationFrame(player_fall_animation, &playground_state->player_fall_00);
	AddAnimationFrame(player_fall_animation, &playground_state->player_fall_01);

	Animation* player_wall_slide_animation = AddAnimation(entity, AnimationType::WALL_SLIDE_ANIMATION_TYPE, 0.2f);
	AddAnimationFrame(player_wall_slide_animation, &playground_state->player_wall_slide_00);
	AddAnimationFrame(player_wall_slide_animation, &playground_state->player_wall_slide_01);

	return entity_index;
}

internal u32 AddFamiliar(PlaygroundState* playground_state)
{
	World* world = &playground_state->world;
	Entity* player_entity = GetEntity(world, world->player_entity_index);
	TilePosition tile_position = player_entity->tile_position;
	f32 width = 0.5f;
	f32 height = 0.6f;
	u32 entity_index = AddAlignedEntity(world, EntityType::FAMILIAR_TYPE, &tile_position, width, height);

	Entity* entity = GetEntity(world, entity_index);
	AddFlags(entity, EntityFlag::MOVEABLE_FLAG);

	Animation* familiar_idle_animation = AddAnimation(entity, AnimationType::IDLE_ANIMATION_TYPE, 0.4f);
	AddAnimationFrame(familiar_idle_animation, &playground_state->familiar_idle_00);
	AddAnimationFrame(familiar_idle_animation, &playground_state->familiar_idle_01);
	AddAnimationFrame(familiar_idle_animation, &playground_state->familiar_idle_02);
	AddAnimationFrame(familiar_idle_animation, &playground_state->familiar_idle_03);

		Animation* familiar_run_animation = AddAnimation(entity, AnimationType::RUN_ANIMATION_TYPE, 0.6f);
	AddAnimationFrame(familiar_run_animation, &playground_state->familiar_run_00);
	AddAnimationFrame(familiar_run_animation, &playground_state->familiar_run_01);
	AddAnimationFrame(familiar_run_animation, &playground_state->familiar_run_02);
	AddAnimationFrame(familiar_run_animation, &playground_state->familiar_run_03);
	AddAnimationFrame(familiar_run_animation, &playground_state->familiar_run_02);
	AddAnimationFrame(familiar_run_animation, &playground_state->familiar_run_03);

	return entity_index;
}

internal u32 AddWall(World* world, i32 tile_x, i32 tile_y, f32 width, f32 height)
{
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;

	u32 entity_index = AddAlignedEntity(world, EntityType::WALL_TYPE, &tile_position, width, height);

	Entity* entity = GetEntity(world, entity_index);

	AddFlags(entity, EntityFlag::COLLIDES_FLAG);

	return entity_index;
}

internal u32 AddMonster(World* world, i32 tile_x, i32 tile_y)
{
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;
	f32 width = 0.6f;
	f32 height = 0.9f;
	u32 entity_index = AddAlignedEntity(world, EntityType::MONSTER_TYPE, &tile_position, width, height);

	Entity* entity = GetEntity(world, entity_index);

	AddFlags(entity, EntityFlag::COLLIDES_FLAG);

	return entity_index;
}

internal Animation* EntityStateControl(PlaygroundState* playground_state,
									   Entity* entity,
									   PlaygroundInput* input=0)
{
	ASSERT(input);
	
	Animation* entity_animation = 0;
	u32 animation_index = 0;
	
	if (entity->type == EntityType::PLAYER_TYPE) {
		if (IsFlagSet(entity, EntityFlag::ON_GROUND_FLAG)) {
			ClearFlags(entity,
					   EntityFlag::RUN_FLAG |
					   EntityFlag::JUMP_FLAG |
					   EntityFlag::DOUBLE_JUMP_FLAG |
					   EntityFlag::FALL_FLAG |
					   EntityFlag::ON_WALL_FLAG);

		}
		if (IsFlagSet(entity, EntityFlag::ON_WALL_FLAG)) {
			animation_index = 5;
			ClearFlags(entity,
					   EntityFlag::RUN_FLAG |
					   EntityFlag::JUMP_FLAG |
					   EntityFlag::DOUBLE_JUMP_FLAG |
					   EntityFlag::FALL_FLAG |
					   EntityFlag::ON_GROUND_FLAG);
		}
		if (IsFlagSet(entity, EntityFlag::JUMP_FLAG)) {
			animation_index = 2;
		}
		if (IsFlagSet(entity, EntityFlag::DOUBLE_JUMP_FLAG)) {
			animation_index = 3;
		}

		if (IsFlagSet(entity, EntityFlag::FALL_FLAG)) {
			animation_index = 4;
		}

		entity->direction = v2(0.0f, -1.0f);
		
		f32 duration = 0.2f;// input->delta_time_for_frame == 0.0f ? 0.2f : input->delta_time_for_frame * 12;
		f32 jump_height = 1.5f;
		f32 jump_speed = (2.0f * jump_height) / duration;
		f32 gravity = (2.0f * jump_height) / Square(duration);
		entity->acceleration.y = gravity;
		
		if (input->move_up.is_down) {
			if (IsFlagSet(entity, EntityFlag::ON_GROUND_FLAG)) {
				entity->direction.y = 1.0f;
				entity->velocity.y = jump_speed;
				
				AddFlags(entity, EntityFlag::JUMP_FLAG);
				ClearFlags(entity, EntityFlag::ON_GROUND_FLAG);
				animation_index = 2;
			}
			else if ((IsFlagSet(entity, EntityFlag::ON_WALL_FLAG) ||
					  IsFlagSet(entity, EntityFlag::FALL_FLAG)) &&
					 !IsFlagSet(entity, EntityFlag::DOUBLE_JUMP_FLAG)) {
				entity->direction.y = 1.0f;
				entity->velocity.y = jump_speed;
				
				AddFlags(entity, EntityFlag::DOUBLE_JUMP_FLAG);
				ClearFlags(entity,
						   EntityFlag::JUMP_FLAG | EntityFlag::FALL_FLAG);
				animation_index = 3;
			}
		}
		if (input->move_down.is_down) {
			entity->direction.y = -1.0f;
		}
		if (input->move_left.is_down) {
			entity->direction.x = -1.0f;
			if (IsFlagSet(entity, EntityFlag::ON_GROUND_FLAG)) {
				AddFlags(entity, EntityFlag::RUN_FLAG);
				animation_index = 1;
			}
		}
		if (input->move_right.is_down) {
			entity->direction.x = 1.0f;

			if (IsFlagSet(entity, EntityFlag::ON_GROUND_FLAG)) {
				AddFlags(entity, EntityFlag::RUN_FLAG);
				animation_index = 1;
			}
		}

		ClearFlags(entity,
				   EntityFlag::ON_WALL_FLAG |
				   EntityFlag::ON_GROUND_FLAG |
				   EntityFlag::FALL_FLAG);
	}
	else if (entity->type == EntityType::FAMILIAR_TYPE) {
		animation_index = 0;

		Entity* player_entity = GetEntity(&playground_state->world, playground_state->world.player_entity_index);
		if (IsFlagSet(player_entity, EntityFlag::RUN_FLAG)) {
			animation_index = 1;
		}
	}

	entity_animation = entity->animations + animation_index;
	f32 seconds_per_frame = entity_animation->duration / (f32)entity_animation->frame_count;
	entity_animation->total_elapsed_time += input->delta_time_for_frame;

	entity_animation->frame_index = (u32)(entity_animation->total_elapsed_time / seconds_per_frame);
	
	if (entity_animation->total_elapsed_time > entity_animation->duration) {
		entity_animation->total_elapsed_time = 0.0;
		entity_animation->frame_index = 0;
	}

	return entity_animation;
}
