inline Entity* GetEntity(World* world, u32 entity_index);
internal void UpdateEntityPositions(World* world, Entity* entity, u32 entity_index,
									TilePosition* old_tile_position, TilePosition* new_tile_position);


inline void AddFlag(Entity* entity, u32 flag)
{
	entity->flags |= flag;
}

inline void ClearFlag(Entity* entity, u32 flag)
{
	entity->flags &= ~flag;
}

inline b32 IsFlagSet(Entity* entity, u32 flag)
{
	b32 result = entity->flags & flag;

	return result;
}

inline void MakeEntityNonspatial(Entity* entity)
{
	entity->flags |= EntityFlag::NONSPATIAL_FLAG;
	entity->position = InvalidPosition();
}

inline void MakeEntityNonspatialAndDeleteFromTileMap(World* world, Entity* entity, u32 entity_index)
{
	MakeEntityNonspatial(entity);
	UpdateEntityPositions(world, entity, entity_index, &entity->tile_position, 0);
}

inline void MakeEntitySpatial(Entity* entity, v2 direction, v2 position, v2 velocity)
{
	ClearFlag(entity, EntityFlag::NONSPATIAL_FLAG);
	entity->direction = direction;
	entity->position = position;
	entity->velocity = velocity;
}

internal void SetCameraLocationAndUpdateEntities(World* world, TilePosition new_camera, b32 first_time=false)
{
	// v2 offset_to_new_camera = TilePositionDifference(world->camera, new_camera, world->tile_side_in_meters);
	world->camera = new_camera;

	// if (first_time || (Length(offset_to_new_camera) != 0.0f)) {

	i32 camera_span_x = world->tile_count_x * 3;
	i32 camera_span_y = world->tile_count_y;
	Rectangle2 camera_bounds = Rectangle2CenterDimension(v2(0.0f, 0.0f),
														 world->tile_side_in_meters *
														 v2((f32)camera_span_x, (f32)camera_span_y));
	i32 lower_limit = (world->camera.tile_x - camera_span_x / 2) < 0 ? 0 : (i32)(world->camera.tile_x - (f32)camera_span_x / 2.0f);
	i32 upper_limit = (i32)(world->camera.tile_x + (f32)camera_span_x / 2.0f);
	i32 min_tile_map_index = GetTileMapIndex(world, lower_limit);
	i32 max_tile_map_index = GetTileMapIndex(world, upper_limit);

	for (i32 tile_map_index = min_tile_map_index; tile_map_index <= max_tile_map_index; ++tile_map_index)  {
		TileMap* tile_map = world->tile_maps + tile_map_index;
		if (tile_map) {
			for (u32 entity_index_index = 0; entity_index_index < tile_map->entity_index_count; ++entity_index_index) {
				u32 entity_index = tile_map->entity_indices[entity_index_index];
				Entity* entity = GetEntity(world, entity_index);

				if (!IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG)) {
					v2 new_position = TilePositionDifference(entity->tile_position, world->camera, world->tile_side_in_meters);
					if (IsInRectangle2(camera_bounds, new_position)) {
						entity->position = new_position;
					}
				}
			}
		}
	}

	// world->camera = new_camera;
	// }
}

internal void UpdateEntityPositions(World* world, Entity* entity, u32 entity_index,
									TilePosition* old_tile_position, TilePosition* new_tile_position)
{
	// TilePosition* old_tile_position = &entity->tile_position;

	if (old_tile_position &&
		new_tile_position &&
		AreOnSameTileMap(world, old_tile_position, new_tile_position)) {
		// NOTE(SSJSR): Do not have to move entity between tile maps.

		entity->tile_position = *new_tile_position;
	}
	else {
		if (old_tile_position && !IsTilePositionInvalid(old_tile_position)) {
			TileMap* old_tile_map = GetTileMap(world, old_tile_position->tile_x);

			for (u32 test_entity_index_index = 0;
				 test_entity_index_index < old_tile_map->entity_index_count;
				 ++test_entity_index_index) {
				u32 test_entity_index = old_tile_map->entity_indices[test_entity_index_index];
				if (test_entity_index == entity_index) {
					u32 last_entity_index = old_tile_map->entity_indices[old_tile_map->entity_index_count - 1];
					old_tile_map->entity_indices[test_entity_index_index] = last_entity_index;
					old_tile_map->entity_indices[--old_tile_map->entity_index_count] = 0;
					break;
				}
			}
		}

		if (new_tile_position && !IsTilePositionInvalid(new_tile_position)) {
			TileMap* new_tile_map = GetTileMap(world, new_tile_position->tile_x);

			// Entity* entity = playground_state->entities + entity_index;
			entity->tile_position = *new_tile_position;

			new_tile_map->entity_indices[new_tile_map->entity_index_count++] = entity_index;
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

internal void MoveEntity(World* world, u32 entity_index, Entity* entity, v2 player_direction, f32 delta_time_for_frame)
{
	v2 player_acceleration = v2(50.0f, 50.0f);

	player_acceleration.x *= player_direction.x;
	player_acceleration.y *= player_direction.y;

	player_acceleration += 8.0f * -entity->velocity;

	v2 player_position_delta = (0.5f * player_acceleration * delta_time_for_frame * delta_time_for_frame) +
		(entity->velocity * delta_time_for_frame);

	entity->velocity =
		(player_acceleration * delta_time_for_frame) +
		(entity->velocity);

	b32 hitted = false;
	for (u32 iteration = 0; iteration < 4; ++iteration) {
		f32 time_minimum = 1.0f;
		b32 hit = false;
		v2 wall_normal = v2(0.0f, 0.0f);

		v2 target_position = entity->position + player_position_delta;
		
		for (u32 test_entity_index = 1;
			 test_entity_index < world->entity_count;
			 ++test_entity_index) {
			
			if (IsFlagSet(entity, EntityFlag::COLLIDES_FLAG) && !IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG)) {
				if (test_entity_index != entity_index) {
					Entity* test_entity = GetEntity(world, test_entity_index); //playground_state->entities + test_entity_index;
					
					if (IsFlagSet(test_entity, EntityFlag::COLLIDES_FLAG) && !IsFlagSet(test_entity, EntityFlag::NONSPATIAL_FLAG)) {
						f32 diameter_width = test_entity->width + entity->width;
						f32 diameter_height = test_entity->height + entity->height;
						v2 wall_min_corner = -0.5f * v2(diameter_width, diameter_height);
						v2 wall_max_corner = 0.5f * v2(diameter_width,  diameter_height);

						v2 tile_relative_position = entity->position - test_entity->position;

						if (TestWall(wall_min_corner.x,
									 tile_relative_position.x, tile_relative_position.y,
									 wall_min_corner.y, wall_max_corner.y,
									 player_position_delta.x,
									 player_position_delta.y,
									 &time_minimum)) {
							wall_normal = v2(-1.0f, 0.0f);
							entity->velocity.y *= 0.4f;
							hit = true;
						}
						
						if (TestWall(wall_max_corner.x,
									 tile_relative_position.x, tile_relative_position.y,
									 wall_min_corner.y, wall_max_corner.y,
									 player_position_delta.x,
									 player_position_delta.y,
									 &time_minimum)) {
							wall_normal = v2(1.0f, 0.0f);
							entity->velocity.y *= 0.4f;
							hit = true;
						}

						if (TestWall(wall_min_corner.y,
									 tile_relative_position.y, tile_relative_position.x,
									 wall_min_corner.x, wall_max_corner.x,
									 player_position_delta.y,
									 player_position_delta.x,
									 &time_minimum)) {
							wall_normal = v2(0.0f, -1.0f);
							hit = true;
						}
						
						if (TestWall(wall_max_corner.y,
									 tile_relative_position.y, tile_relative_position.x,
									 wall_min_corner.x, wall_max_corner.x,
									 player_position_delta.y,
									 player_position_delta.x,
									 &time_minimum)) {
							wall_normal = v2(0.0f, 1.0f);
							hit = true;
						}


					}
				}
			}
		}

		entity->position = (player_position_delta * time_minimum) + entity->position;

		// NormalizePositions(&playground_state->player.position, playground_state->tile_side_in_meters);
		if (hit) {
			entity->velocity = entity->velocity - 1.0f * Dot(entity->velocity, wall_normal) * wall_normal;
			player_position_delta = target_position - entity->position;
			// player_position_delta = TilePositionDifference(new_player_position, playground_state->player.position, playground_state->tile_side_in_meters);
			player_position_delta = player_position_delta - 1.0f * Dot(player_position_delta, wall_normal) * wall_normal;

			hitted = true;

		}
		else {
			break;
		}
	}

	if (hitted && entity->type == EntityType::BALL_TYPE) {
		MakeEntityNonspatialAndDeleteFromTileMap(world, entity, entity_index);
	}
	else {
		TilePosition new_tile_position = MapIntoTilePosition(world->camera, entity->position, world->tile_side_in_meters);
		UpdateEntityPositions(world, entity, entity_index, &entity->tile_position, &new_tile_position);
	}

	if (entity->velocity.x < 0.0f) {
		entity->facing_direction = 1; // Left
	}
	else if (entity->velocity.x > 0.0f) {
		entity->facing_direction = 2; // Right
	}
}

internal u32 AddEntity(World* world, EntityType type, TilePosition* tile_position)
{
	ASSERT(world->entity_count < ARRAY_COUNT(world->entities));

	u32 entity_index = world->entity_count++;

	Entity* entity = world->entities + entity_index;
	*entity = {};
	entity->type = type;

	if (tile_position) {
		entity->tile_position = *tile_position;
		
		v2 relative_position = TilePositionDifference(entity->tile_position, world->camera, world->tile_side_in_meters);
		entity->position = relative_position;
		
		TileMap* tile_map = GetTileMap(world, tile_position->tile_x);
		tile_map->entity_indices[tile_map->entity_index_count++] = entity_index;
	}
	else {
		entity->tile_position = InvalidTilePosition();
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

internal u32 AddBall(World* world)
{
	u32 entity_index = AddEntity(world, EntityType::BALL_TYPE, 0);

	Entity* entity = GetEntity(world, entity_index);

	entity->height = 0.5f;
	entity->width = 0.5f;
	AddFlag(entity, EntityFlag::COLLIDES_FLAG);

	return entity_index;
}

internal u32 AddPlayer(World* world)
{
	TilePosition tile_position = {};
	tile_position.tile_x = 5;
	tile_position.tile_y = 5;
	u32 entity_index = AddEntity(world, EntityType::PLAYER_TYPE, &tile_position);

	Entity* entity = GetEntity(world, entity_index);

	entity->height = 0.9f;
	entity->width = 0.6f;
	// entity->collides = true;
	AddFlag(entity, EntityFlag::COLLIDES_FLAG);

	entity->ball_index = AddBall(world);

	return entity_index;
}

internal u32 AddWall(World* world, i32 tile_x, i32 tile_y)
{
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;

	u32 entity_index = AddEntity(world, EntityType::WALL_TYPE, &tile_position);

	Entity* entity = GetEntity(world, entity_index);

	entity->height = world->tile_side_in_meters;
	entity->width = world->tile_side_in_meters;
	// entity->collides = true;
	AddFlag(entity, EntityFlag::COLLIDES_FLAG);

	return entity_index;
}

internal u32 AddMonster(World* world, i32 tile_x, i32 tile_y)
{
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;

	u32 entity_index = AddEntity(world, EntityType::MONSTER_TYPE, &tile_position);

	Entity* entity = GetEntity(world, entity_index);

	entity->height = 0.9f;
	entity->width = 0.6f;
	// entity->collides = true;
	AddFlag(entity, EntityFlag::COLLIDES_FLAG);
	entity->direction = v2(0.4f, -1.0f);
	entity->distance_remaining = 20.0f;

	return entity_index;
}

inline void UpdateMonster(World* world, u32 entity_index, f32 delta_time)
{
	Entity* monster_entity = GetEntity(world, entity_index);

	// v2 monster_direction = v2(0.4f, -1.0f);
	
	if (monster_entity->distance_remaining < 0.0f) {
		monster_entity->direction.x *= -1.0f;
		monster_entity->distance_remaining = 20.0f;
	}

	v2 old_monster_position = monster_entity->position;
	
	MoveEntity(world, entity_index, monster_entity, monster_entity->direction, delta_time);

	f32 monster_distance_travelled = Length(monster_entity->position - old_monster_position);
	
	monster_entity->distance_remaining -= monster_distance_travelled;
}

inline void UpdateBall(World* world, u32 entity_index, Entity* ball_entity, f32 delta_time)
{
	v2 old_ball_position = ball_entity->position;
	
	MoveEntity(world, entity_index, ball_entity, ball_entity->direction, delta_time);

	f32 ball_distance_travelled = Length(ball_entity->position - old_ball_position);
	
	ball_entity->distance_remaining -= ball_distance_travelled;
	if (ball_entity->distance_remaining < 0.0f) {
		MakeEntityNonspatialAndDeleteFromTileMap(world, ball_entity, entity_index);
	}
}
