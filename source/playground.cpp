#include "playground.h"
#include "playground_draw.cpp"

internal void NormalizeCoordinates(i32* tile_position, f32* position, f32 tile_side_in_meters)
{
	i32 offset = RoundF32ToI32(*position / tile_side_in_meters);
	*tile_position += offset;
	f32 normalized_tile_coordinate = offset * tile_side_in_meters;
	*position -= normalized_tile_coordinate;
}

internal void NormalizePositions(TilePosition* position, f32 tile_side_in_meters)
{
	NormalizeCoordinates(&position->tile_x, &position->xy.x, tile_side_in_meters);
	NormalizeCoordinates(&position->tile_y, &position->xy.y, tile_side_in_meters);
}

inline v2 TilePositionDifference(TilePosition a, TilePosition b, f32 tile_side_in_meters)
{
	v2 result;

	v2 delta_tile_xy = {
		(f32)a.tile_x - (f32)b.tile_x,
		(f32)a.tile_y - (f32)b.tile_y
	};

	result = delta_tile_xy * tile_side_in_meters + (a.xy - b.xy);

	return result;
}

internal b32 TestWall(f32 wall_x,
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

inline TilePosition MapIntoTilePosition(PlaygroundState* playground_state, TilePosition tile_position, v2 offset)
{
	TilePosition result = tile_position;
	result.xy += offset;
	NormalizePositions(&result, playground_state->tile_side_in_meters);

	return result;
}

#define INVALID_TILE_POSITION 1000000

inline TilePosition InvalidTilePosition()
{
	TilePosition result = {};

	result.tile_x = INVALID_TILE_POSITION;

	return result;
}

inline b32 IsTilePositionInvalid(TilePosition* tile_position)
{
	b32 result = tile_position->tile_x == INVALID_TILE_POSITION;

	return result;
}

inline u32 GetTileMapIndex(PlaygroundState* playground_state, u32 tile_x)
{
	u32 result = tile_x / playground_state->tile_count_x;

	return result;
}

inline TileMap* GetTileMap(PlaygroundState* playground_state, u32 tile_x)
{
	u32 tile_map_index = GetTileMapIndex(playground_state, tile_x);

	ASSERT(tile_map_index < playground_state->tile_map_count_x);

	TileMap* result = playground_state->tile_maps + tile_map_index;

	return result;
}

inline b32 AreOnSameTileMap(PlaygroundState* playground_state, TilePosition* tile_position_a, TilePosition* tile_position_b)
{
	u32 tile_position_a_index = GetTileMapIndex(playground_state, tile_position_a->tile_x);
	u32 tile_position_b_index = GetTileMapIndex(playground_state, tile_position_b->tile_x);

	b32 result = tile_position_a_index == tile_position_b_index;

	return result;
}

internal void UpdateEntityPositions(PlaygroundState* playground_state, u32 entity_index, TilePosition* new_tile_position)
{
	TilePosition* old_tile_position = &playground_state->entities[entity_index].tile_position;

	// if (!IsTilePositionInvalid(old_tile_position) &&
	// 	// !IsTilePositionInvalid(new_tile_position) &&
	// 	AreOnSameTileMap(playground_state, old_tile_position, new_tile_position)
	// 	) {
	// 	// NOTE(SSJSR): Do not have to move entity between tile maps.
	// }
	// else {
	if (!IsTilePositionInvalid(old_tile_position)) {
		TileMap* old_tile_map = GetTileMap(playground_state, old_tile_position->tile_x);

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

	if (!IsTilePositionInvalid(new_tile_position)) {
		TileMap* new_tile_map = GetTileMap(playground_state, new_tile_position->tile_x);

		Entity* entity = playground_state->entities + entity_index;
		entity->tile_position = *new_tile_position;

		new_tile_map->entity_indices[new_tile_map->entity_index_count++] = entity_index;
	}
	// }
}

internal void MoveEntity(PlaygroundState* playground_state, u32 entity_index, Entity* entity, v2 player_direction, f32 delta_time_for_frame)
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

	// v2 new_player_position = ((player_position_delta) +
	// 						  (entity->position));

	// NormalizePositions(&new_player_position, playground_state->tile_side_in_meters);

	// u32 min_tile_x = MINIMUM(playground_state->player.position.tile_x, new_player_position.tile_x);
	// u32 min_tile_y = MINIMUM(playground_state->player.position.tile_y, new_player_position.tile_y);
	// u32 max_tile_x = MAXIMUM(playground_state->player.position.tile_x, new_player_position.tile_x);
	// u32 max_tile_y = MAXIMUM(playground_state->player.position.tile_y, new_player_position.tile_y);

	// u32 player_width = CeilF32ToU32(playground_state->player_width / playground_state->tile_side_in_meters);
	// u32 player_height = CeilF32ToU32(playground_state->player_height / playground_state->tile_side_in_meters);

	// min_tile_x -= player_width;
	// min_tile_y -= player_height;
	// max_tile_x += player_width;
	// max_tile_y += player_height;

	for (u32 iteration = 0; iteration < 4; ++iteration) {
		f32 time_minimum = 1.0f;
		b32 hit = false;
		v2 wall_normal = v2(0.0f, 0.0f);

		v2 target_position = entity->position + player_position_delta;
		// for (u32 tile_y = min_tile_y; tile_y <= max_tile_y; ++tile_y) {
		// 	for (u32 tile_x = min_tile_x; tile_x <= max_tile_x; ++tile_x) {
		// TileMap* tile_map = GetTileMap(playground_state, entity->tile_position.tile_x);
		// for (u32 test_entity_index_index = 0;
		// 	 test_entity_index_index < tile_map->entity_index_count;
		// 	 ++test_entity_index_index) {
		for (u32 test_entity_index = 1;
			 test_entity_index < playground_state->entity_count;
			 ++test_entity_index) {
			
			// u32 test_entity_index = tile_map->entity_indices[test_entity_index_index];
			// if (!IsTileMapPointEmpty(playground_state, tile_x, tile_y)) {
			if (test_entity_index != entity_index) {
				Entity* test_entity = playground_state->entities + test_entity_index;

				if (test_entity->collides) {
					f32 diameter_width = test_entity->width + entity->width;
					f32 diameter_height = test_entity->height + entity->height;
					v2 wall_min_corner = -0.5f * v2(diameter_width, diameter_height);
					v2 wall_max_corner = 0.5f * v2(diameter_width,  diameter_height);

					// TilePosition test_position = CreateCenteredTilePosition(tile_x, tile_y);

					// v2 tile_relative_position = TilePositionDifference(playground_state->player.position, test_position, playground_state->tile_side_in_meters);
					v2 tile_relative_position = entity->position - test_entity->position;

					if (TestWall(wall_max_corner.x,
								 tile_relative_position.x, tile_relative_position.y,
								 wall_min_corner.y, wall_max_corner.y,
								 player_position_delta.x,
								 player_position_delta.y,
								 &time_minimum)) {
						wall_normal = v2(1.0f, 0.0f);
						hit = true;
					}

					if (TestWall(wall_min_corner.x,
								 tile_relative_position.x, tile_relative_position.y,
								 wall_min_corner.y, wall_max_corner.y,
								 player_position_delta.x,
								 player_position_delta.y,
								 &time_minimum)) {
						wall_normal = v2(-1.0f, 0.0f);
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

					if (TestWall(wall_min_corner.y,
								 tile_relative_position.y, tile_relative_position.x,
								 wall_min_corner.x, wall_max_corner.x,
								 player_position_delta.y,
								 player_position_delta.x,
								 &time_minimum)) {
						wall_normal = v2(0.0f, -1.0f);
						hit = true;
					}
				}
			}
			// 	}
			// }
		}

		entity->position = (player_position_delta * time_minimum) + entity->position;

		// NormalizePositions(&playground_state->player.position, playground_state->tile_side_in_meters);
		if (hit) {
			entity->velocity = entity->velocity - 1.0f * Dot(entity->velocity, wall_normal) * wall_normal;
			player_position_delta = target_position - entity->position;
			// player_position_delta = TilePositionDifference(new_player_position, playground_state->player.position, playground_state->tile_side_in_meters);
			player_position_delta = player_position_delta - 1.0f * Dot(player_position_delta, wall_normal) * wall_normal;
		}
		else {
			break;
		}
	}

	TilePosition new_tile_position = MapIntoTilePosition(playground_state, playground_state->camera, entity->position);
	UpdateEntityPositions(playground_state, entity_index, &new_tile_position);
	// entity->tile_position = new_tile_position;

	if (entity->velocity.x < 0.0f) {
		entity->facing_direction = 1; // Left
	}
	else if (entity->velocity.x > 0.0f) {
		entity->facing_direction = 2; // Right
	}
}

internal u32 AddEntity(PlaygroundState* playground_state, EntityType type, TilePosition* tile_position)
{
	ASSERT(playground_state->entity_count < ARRAY_COUNT(playground_state->entities));

	u32 entity_index = playground_state->entity_count++;

	Entity* entity = playground_state->entities + entity_index;
	*entity = {};
	entity->type = type;

	if (tile_position) {
		entity->tile_position = *tile_position;
		u32 tile_map_index = tile_position->tile_x / playground_state->tile_count_x;
		TileMap* tile_map = playground_state->tile_maps + tile_map_index;
		ASSERT(tile_map->entity_index_count < ARRAY_COUNT(tile_map->entity_indices));

		tile_map->entity_indices[tile_map->entity_index_count++] = entity_index;
	}
	else {
		entity->tile_position = InvalidTilePosition();
	}
	return entity_index;
}

internal Entity* GetEntity(PlaygroundState* playground_state, u32 entity_index)
{
	ASSERT(entity_index < playground_state->entity_count);

	Entity* entity = 0;

	if (entity_index < playground_state->entity_count) {
		entity = playground_state->entities + entity_index;
	}

	return entity;
}

internal u32 AddPlayer(PlaygroundState* playground_state)
{
	TilePosition tile_position = {};
	tile_position.tile_x = 5;
	tile_position.tile_y = 5;
	u32 entity_index = AddEntity(playground_state, EntityType::PLAYER_TYPE, &tile_position);

	Entity* entity = playground_state->entities + entity_index;

	entity->height = 0.9f;
	entity->width = 0.6f;
	entity->collides = true;

	v2 relative_position = TilePositionDifference(entity->tile_position, playground_state->camera, playground_state->tile_side_in_meters);
	entity->position = relative_position;

	return entity_index;
}

internal u32 AddWall(PlaygroundState* playground_state, i32 tile_x, i32 tile_y)
{
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;

	u32 entity_index = AddEntity(playground_state, EntityType::WALL_TYPE, &tile_position);

	Entity* entity = playground_state->entities + entity_index;

	entity->height = playground_state->tile_side_in_meters;
	entity->width = playground_state->tile_side_in_meters;
	entity->collides = true;

	v2 relative_position = TilePositionDifference(entity->tile_position, playground_state->camera, playground_state->tile_side_in_meters);
	entity->position = relative_position;

	return entity_index;
}

internal u32 AddMonster(PlaygroundState* playground_state, i32 tile_x, i32 tile_y)
{
	TilePosition tile_position = {};
	tile_position.tile_x = tile_x;
	tile_position.tile_y = tile_y;

	u32 entity_index = AddEntity(playground_state, EntityType::MONSTER_TYPE, &tile_position);

	Entity* entity = playground_state->entities + entity_index;

	entity->height = 0.9f;
	entity->width = 0.6f;
	entity->collides = true;
	entity->direction = v2(0.4f, -1.0f);
	entity->distance_remaining = 20.0f;

	v2 relative_position = TilePositionDifference(entity->tile_position, playground_state->camera, playground_state->tile_side_in_meters);
	entity->position = relative_position;

	return entity_index;
}

internal void SetCameraLocationAndUpdateEntities(PlaygroundState* playground_state, TilePosition new_camera, b32 first_time=false)
{
	v2 offset_to_new_camera = TilePositionDifference(playground_state->camera, new_camera, playground_state->tile_side_in_meters);

	if (first_time || (Length(offset_to_new_camera) != 0.0f)) {

		// u32 previous_tile_map_index = playground_state->camera.tile_x / playground_state->tile_count_x;
		// TileMap* previous_tile_map = playground_state->tile_maps + previous_tile_map_index;
		// for (u32 entity_index_index = 0; entity_index_index < previous_tile_map->entity_index_count; ++entity_index_index) {
		// 	u32 entity_index = previous_tile_map->entity_indices[entity_index_index];
		// 	if (entity_index == playground_state->player_entity_index) {
		// 		previous_tile_map->entity_indices[entity_index_index] = previous_tile_map->entity_indices[previous_tile_map->entity_index_count - 1];
		// 		--previous_tile_map->entity_index_count;
		// 		break;
		// 	}
		// }
		
		// TileMap* tile_map = GetTileMap(playground_state, new_camera.tile_x);
		i32 camera_span_x = playground_state->tile_count_x * 3;
		i32 lower_limit = (new_camera.tile_x - camera_span_x / 2) < 0 ? 0 : (i32)(new_camera.tile_x - (f32)camera_span_x / 2.0f);
		i32 upper_limit = (i32)(new_camera.tile_x + (f32)camera_span_x / 2.0f);
		i32 min_tile_map_index = GetTileMapIndex(playground_state, lower_limit);
		i32 max_tile_map_index = GetTileMapIndex(playground_state, upper_limit);

		for (i32 tile_map_index = min_tile_map_index; tile_map_index <= max_tile_map_index; ++tile_map_index)  {
			TileMap* tile_map = playground_state->tile_maps + tile_map_index;
			if (tile_map) {
				for (u32 entity_index_index = 0; entity_index_index < tile_map->entity_index_count; ++entity_index_index) {
					u32 entity_index = tile_map->entity_indices[entity_index_index];
					Entity* entity = GetEntity(playground_state, entity_index);
				
					v2 new_position = TilePositionDifference(entity->tile_position, new_camera, playground_state->tile_side_in_meters);
					entity->position = new_position;
				}
			}
		}

		playground_state->camera = new_camera;



		// i32 tile_span_x = playground_state->tile_count_x * 1;
		// i32 tile_span_y = playground_state->tile_count_y;
		// Rectangle2 camera_bounds =
		// 	Rectangle2CenterDimension(v2(0.0f, 0.0f),
		// 							  playground_state->tile_side_in_meters * v2((f32)tile_span_x, (f32)tile_span_y));

		// UpdateHighFrequencyEntitiesByCameraBounds(playground_state, camera_bounds, offset_to_new_camera);

		// i32 min_tile_x = new_camera.tile_x - tile_span_x / 2;
		// i32 min_tile_y = new_camera.tile_y - tile_span_y / 2;
		// i32 max_tile_x = new_camera.tile_x + tile_span_x / 2;
		// i32 max_tile_y = new_camera.tile_y + tile_span_y / 2;

		// for (u32 entity_index = 1;
		// 	 entity_index < playground_state->entity_count;
		// 	 ++entity_index) {
		// 	Entity* entity = playground_state->entities + entity_index;

		// 	if ((entity->tile_position.tile_x >= min_tile_x) &&
		// 		(entity->tile_position.tile_y >= min_tile_y) &&
		// 		(entity->tile_position.tile_x <= max_tile_x) &&
		// 		(entity->tile_position.tile_y <= max_tile_y)) {

		// 		v2 relative_position = TilePositionDifference(entity->tile_position, new_camera, playground_state->tile_side_in_meters);
		// 		entity->position = relative_position;
		// 		// MakeHighFrequencyEntity(playground_state, low_frequency_entity_index);
		// 	}
		// 	else {
		// 		entity->position += offset_to_new_camera;
		// 		if (!IsInRectangle2(camera_bounds, entity->position)) {
		// 			TilePosition new_position = {};
		// 			new_position.xy = entity->position;
		// 			NormalizePositions(&new_position, playground_state->tile_side_in_meters);
		// 			entity->tile_position = new_position;
		// 		}
		// 	}
		// }

		// playground_state->camera = new_camera;
	}
}

internal void AddPlayerBitmap(PlaygroundState* playground_state, LoadedBmp* bitmap, PlayerStateType state_type)
{
	PlayerBitmapState* player_bitmap_state = &playground_state->player_bitmap_state;
	if (state_type == PlayerStateType::MAX_STATE_TYPE) {
		player_bitmap_state->state_types[player_bitmap_state->state_count] = state_type;
		player_bitmap_state->index_offsets[player_bitmap_state->state_count] = player_bitmap_state->bitmap_count;
		player_bitmap_state->current_state = PlayerStateType::IDLE_STATE_TYPE;

	}
	else {
		ASSERT(player_bitmap_state->bitmap_count < ARRAY_COUNT(player_bitmap_state->bitmaps));
		if (player_bitmap_state->state_count == 0 || state_type != player_bitmap_state->state_types[player_bitmap_state->state_count - 1]) {
			player_bitmap_state->state_types[player_bitmap_state->state_count] = state_type;
			player_bitmap_state->index_offsets[player_bitmap_state->state_count++] = player_bitmap_state->bitmap_count;
			player_bitmap_state->bitmaps[player_bitmap_state->bitmap_count++] = *bitmap;
		}
		else {
			player_bitmap_state->bitmaps[player_bitmap_state->bitmap_count++] = *bitmap;
		}
	}
}

internal LoadedBmp* GetPlayerBitmap(PlaygroundState* playground_state)
{
	LoadedBmp* result = 0;

	PlayerBitmapState* player_bitmap_state = &playground_state->player_bitmap_state;

	ASSERT(player_bitmap_state->state_types[ARRAY_COUNT(player_bitmap_state->state_types) - 1] ==
		   PlayerStateType::MAX_STATE_TYPE);

	++player_bitmap_state->tick_counter;

	if (player_bitmap_state->last_state != player_bitmap_state->current_state) {
		player_bitmap_state->bitmap_index_without_offset = 0;
		player_bitmap_state->tick_counter = 0;

		player_bitmap_state->last_state = player_bitmap_state->current_state;
	}

	for (u32 state_index = 0; state_index < player_bitmap_state->state_count; ++state_index) {
		if (player_bitmap_state->state_types[state_index] == player_bitmap_state->current_state) {
			u32 bitmap_index = player_bitmap_state->index_offsets[state_index] +
				player_bitmap_state->bitmap_index_without_offset;

			result = player_bitmap_state->bitmaps + bitmap_index;

			if (player_bitmap_state->tick_counter % 4 == 0) {
				++player_bitmap_state->bitmap_index_without_offset;

				u32 total_state_count = player_bitmap_state->index_offsets[state_index + 1] -
					player_bitmap_state->index_offsets[state_index];
				player_bitmap_state->bitmap_index_without_offset %= total_state_count;
			}
		}
	}

	player_bitmap_state->current_state = PlayerStateType::IDLE_STATE_TYPE;

	return result;
}

inline void UpdateMonster(PlaygroundState* playground_state, u32 entity_index, f32 delta_time)
{
	Entity* monster_entity = GetEntity(playground_state, entity_index);

	// v2 monster_direction = v2(0.4f, -1.0f);
	
	if (monster_entity->distance_remaining < 0.0f) {
		monster_entity->direction.x *= -1.0f;
		monster_entity->distance_remaining = 20.0f;
	}

	v2 old_monster_position = monster_entity->position;
	
	MoveEntity(playground_state, entity_index, monster_entity, monster_entity->direction, delta_time);

	f32 monster_distance_travelled = Length(monster_entity->position - old_monster_position);
	
	monster_entity->distance_remaining -= monster_distance_travelled;
}

// NOTE(SSJSR): Function signature:
// PlaygroundUpdateAndRender(PlaygroundMemory* memory, PlaygroundDisplayBuffer* display_buffer, PlaygroundInput* input)
extern "C" PLAYGROUND_UPDATE_AND_RENDER(PlaygroundUpdateAndRender)
{
	ASSERT(&input->terminator - &input->buttons[0] == ARRAY_COUNT(input->buttons));
	ASSERT(sizeof(PlaygroundState) < memory->permanent_storage_size);

	PlaygroundState* playground_state = (PlaygroundState*)memory->permanent_storage;

	if (!memory->is_initialized) {
		AddEntity(playground_state, EntityType::NULL_TYPE, 0);

		playground_state->tile_side_in_pixels = 30.0f;
		playground_state->tile_side_in_meters = 1.0f;
		playground_state->meters_to_pixels = playground_state->tile_side_in_pixels / playground_state->tile_side_in_meters;
		playground_state->tile_count_y = (display_buffer->height / (u32)playground_state->tile_side_in_pixels) + 1;
		playground_state->tile_count_x = (display_buffer->width / (u32)playground_state->tile_side_in_pixels) + 1;

		InitializeArena(&playground_state->arena,
						memory->permanent_storage_size - sizeof(*playground_state),
						(u8*)memory->permanent_storage + sizeof(*playground_state));

		playground_state->tile_map_count_x = 3;
		playground_state->tile_map_count_y = 1;
		playground_state->tile_maps =
			PushArray(&playground_state->arena, playground_state->tile_map_count_x * playground_state->tile_map_count_y, TileMap);

		for (u32 tile_map_y = 0;
			 tile_map_y < playground_state->tile_map_count_y;
			 ++tile_map_y) {
			for (u32 tile_map_x = 0;
				 tile_map_x < playground_state->tile_map_count_x;
				 ++tile_map_x) {

				playground_state->tile_maps[tile_map_y * playground_state->tile_map_count_x + tile_map_x].tiles =
					PushArray(&playground_state->arena,
							  playground_state->tile_count_y * playground_state->tile_count_x,
							  u32);

				playground_state->tile_maps[tile_map_y * playground_state->tile_map_count_x + tile_map_x].tile_map_index = tile_map_x;

				for (i32 y = 0; y < playground_state->tile_count_y; ++y) {
					for (i32 x = 0; x < playground_state->tile_count_x; ++x) {
						i32 tile_x = tile_map_x * playground_state->tile_count_x + x;
						i32 tile_y = tile_map_y * playground_state->tile_count_y + y;
						
						if (y == 0) {
							AddWall(playground_state, tile_x, tile_y);
						}
						else if ((x == 0 || x == playground_state->tile_count_x - 1) &&
								 y != 0 &&
								 y != 1 &&
								 y != 2) {
							AddWall(playground_state, tile_x, tile_y);
						}
						// if (tile_x == playground_state->tile_count_x - 1 &&
						// 		 tile_y == 16) {
						// 	AddWall(playground_state, tile_x, tile_y);
						// }
					}
				}
			}
		}

		// NOTE(SSJSR): Add player to map.
		playground_state->player_entity_index =
			AddPlayer(playground_state);

		playground_state->background = LoadBmp("background_day_scaled.bmp", memory->PlaygroundReadFile, 0, 0);

		// NOTE(SSJSR): Idle state.

		LoadedBmp player_idle_00 = LoadBmp("adventurer/idle/adventurer-idle-2-00.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_idle_01 = LoadBmp("adventurer/idle/adventurer-idle-2-01.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_idle_02 = LoadBmp("adventurer/idle/adventurer-idle-2-02.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_idle_03 = LoadBmp("adventurer/idle/adventurer-idle-2-03.bmp", memory->PlaygroundReadFile, 25, 35);

		AddPlayerBitmap(playground_state, &player_idle_00, PlayerStateType::IDLE_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_idle_01, PlayerStateType::IDLE_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_idle_02, PlayerStateType::IDLE_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_idle_03, PlayerStateType::IDLE_STATE_TYPE);

		// NOTE(SSJSR): Run state.

		LoadedBmp player_run_00 = LoadBmp("adventurer/run/adventurer-run3-00.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_run_01 = LoadBmp("adventurer/run/adventurer-run3-01.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_run_02 = LoadBmp("adventurer/run/adventurer-run3-02.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_run_03 = LoadBmp("adventurer/run/adventurer-run3-03.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_run_04 = LoadBmp("adventurer/run/adventurer-run3-04.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_run_05 = LoadBmp("adventurer/run/adventurer-run3-05.bmp", memory->PlaygroundReadFile, 25, 35);

		AddPlayerBitmap(playground_state, &player_run_00, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_run_01, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_run_02, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_run_03, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_run_04, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_run_05, PlayerStateType::RUN_STATE_TYPE);

		// NOTE(SSJSR): Jump state.

		LoadedBmp player_jump_00 = LoadBmp("adventurer/jump/adventurer-jump-00.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_jump_01 = LoadBmp("adventurer/jump/adventurer-jump-01.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_jump_02 = LoadBmp("adventurer/jump/adventurer-jump-02.bmp", memory->PlaygroundReadFile, 25, 35);
		LoadedBmp player_jump_03 = LoadBmp("adventurer/jump/adventurer-jump-03.bmp", memory->PlaygroundReadFile, 25, 35);

		AddPlayerBitmap(playground_state, &player_jump_00, PlayerStateType::JUMP_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_jump_01, PlayerStateType::JUMP_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_jump_02, PlayerStateType::JUMP_STATE_TYPE);
		AddPlayerBitmap(playground_state, &player_jump_03, PlayerStateType::JUMP_STATE_TYPE);

		AddPlayerBitmap(playground_state, 0, PlayerStateType::MAX_STATE_TYPE);

		AddWall(playground_state, 7, 7);

		AddMonster(playground_state, playground_state->tile_count_x - 7, 10);

		playground_state->camera.tile_x = playground_state->tile_count_x / 2;
		playground_state->camera.tile_y = playground_state->tile_count_y / 2;
		playground_state->camera.xy = v2(0.0f, 0.0f);
		
		playground_state->desired_camera = playground_state->camera;

		playground_state->is_camera_moving = false;
		playground_state->camera_movement_duration = 5; // Frame
		playground_state->camera_movement_duration_remaining =
			playground_state->camera_movement_duration;

		// NOTE(SSJSR): Update first entities.
		SetCameraLocationAndUpdateEntities(playground_state, playground_state->camera, true);

		memory->is_initialized = true;
	}

	// if (input->mouse_left.is_down) {
	// 	if (input->mouse_x <= display_buffer->width && input->mouse_y <= display_buffer->height) {
	// 		u32 tile_x = RoundF32ToU32((f32)input->mouse_x / playground_state->tile_side_in_pixels);
	// 		u32 tile_y = RoundF32ToU32(((f32)display_buffer->height - (f32)input->mouse_y) / playground_state->tile_side_in_pixels);
	// 		SetTileValueRadius(playground_state, tile_x, tile_y, 2, playground_state->tile_radius);
	// 	}
	// }

	// if (input->mouse_right.is_down) {
	// 	if (input->mouse_x <= display_buffer->width && input->mouse_y <= display_buffer->height) {
	// 		u32 tile_x = RoundF32ToU32((f32)input->mouse_x / playground_state->tile_side_in_pixels);
	// 		u32 tile_y = RoundF32ToU32(((f32)display_buffer->height - (f32)input->mouse_y) / playground_state->tile_side_in_pixels);
	// 		SetTileValueRadius(playground_state, tile_x, tile_y, 1, playground_state->tile_radius);
	// 	}
	// }

	Entity* player_entity = GetEntity(playground_state, playground_state->player_entity_index);

	player_entity->direction = v2(0.0f, -1.0f);

	if (input->move_up.is_down) {
		player_entity->direction.y = 1.0f;
		player_entity->velocity.y = 5.0f;
		playground_state->player_bitmap_state.current_state = PlayerStateType::JUMP_STATE_TYPE;

	}
	if (input->move_down.is_down) {
		player_entity->direction.y = -1.0f;
	}
	if (input->move_left.is_down) {
		player_entity->direction.x = -1.0f;
		playground_state->player_bitmap_state.current_state = PlayerStateType::RUN_STATE_TYPE;
	}
	if (input->move_right.is_down) {
		player_entity->direction.x = 1.0f;
		playground_state->player_bitmap_state.current_state = PlayerStateType::RUN_STATE_TYPE;
	}

	f32 direction_length = Length(player_entity->direction);
	if (direction_length > 1.0f) {
		f32 ratio = 1.0f / direction_length;
		player_entity->direction *= ratio;
	}
	
	MoveEntity(playground_state, playground_state->player_entity_index, player_entity, player_entity->direction, input->delta_time_for_frame);
	
	TilePosition new_camera = playground_state->camera;
	
#if 0
	new_camera.tile_x = player_entity->tile_position.tile_x;
	new_camera.xy.x = player_entity->tile_position.xy.x;
#else
	if (!playground_state->is_camera_moving) {
		if (player_entity->position.x > 0.5f * playground_state->tile_count_x * playground_state->tile_side_in_meters) {
			playground_state->desired_camera.tile_x += playground_state->tile_count_x;
			playground_state->is_camera_moving = true;
		}

		if (player_entity->position.x < -0.5f * playground_state->tile_count_x * playground_state->tile_side_in_meters) {
			playground_state->desired_camera.tile_x -= playground_state->tile_count_x;
			playground_state->is_camera_moving = true;
		}
	}
	else {
		if (playground_state->camera_movement_duration_remaining > 0) {
			
			f32 camera_movement_per_frame = playground_state->tile_count_x / (f32)playground_state->camera_movement_duration;
			if (new_camera.tile_x <= playground_state->desired_camera.tile_x) {
				new_camera.xy.x += camera_movement_per_frame;
			}
			else {
				new_camera.xy.x -= camera_movement_per_frame;
			}
			NormalizePositions(&new_camera, playground_state->tile_side_in_meters);
			--playground_state->camera_movement_duration_remaining;
		}
		else {
			v2 difference = TilePositionDifference(playground_state->desired_camera, new_camera, playground_state->tile_side_in_meters);
			if (new_camera.tile_x <= playground_state->desired_camera.tile_x) {
				new_camera.xy.x += difference.x;

			}
			else {
				new_camera.xy.x -= -difference.x;
			}
			NormalizePositions(&new_camera, playground_state->tile_side_in_meters);
			playground_state->is_camera_moving = false;
			playground_state->camera_movement_duration_remaining = playground_state->camera_movement_duration;
		}
	}
	
#endif

	SetCameraLocationAndUpdateEntities(playground_state, new_camera);

	v2 screen_center = v2((f32)(display_buffer->width / 2), (f32)(display_buffer->height / 2));

	// DrawRectangle(display_buffer,
	// 			  0.0f, 0.0f, (f32)display_buffer->width, (f32)display_buffer->height,
	// 			  1.0f, 0.5f, 0.0f);

	DrawBitmap(display_buffer, &playground_state->background,
			   0, 0);

	// NOTE(SSJSR): Draw tile map.
#if 0
	for (i32 y = -playground_state->tile_count_y / 2; y <= playground_state->tile_count_y / 2; ++y) {
		for (i32 x = -playground_state->tile_count_x / 2; x <= playground_state->tile_count_x / 2; ++x) {
			i32 relative_x = x + playground_state->camera.tile_x;
			i32 relative_y = y + playground_state->camera.tile_y;
			f32 min_x = screen_center.x + (x * playground_state->tile_side_in_pixels) - (playground_state->tile_side_in_pixels / 2.0f);
			f32 min_y = screen_center.y - (y * playground_state->tile_side_in_pixels) - (playground_state->tile_side_in_pixels / 2.0f);
			f32 max_x = min_x + playground_state->tile_side_in_pixels;
			f32 max_y = min_y + playground_state->tile_side_in_pixels;

			i32 tile_value = GetTileValue(playground_state, relative_x, relative_y);

			if (tile_value > 0) {
				f32 r = 0.18f;
				f32 g = 0.20f;
				f32 b = 0.25f;

				if (tile_value == 2) {
					g = 0.6f;
				}

				// if (relative_y == playground_state->camera.tile_y &&
				// 	relative_x == playground_state->camera.tile_x) {
				// 	r = 1.0f;
				// 	g = 0.5f;
				// 	b = 0.0f;
				// }

				// if (relative_y == playground_state->player.position.tile_y &&
				// 	relative_x == playground_state->player.position.tile_x) {
				// 	r = 0.0f;
				// 	g = 0.0f;
				// 	b = 0.0f;
				// }

				DrawRectangleWithBorder(display_buffer,
										min_x, min_y, max_x, max_y,
										r, g, b,
										playground_state->tile_border_width,
										0.15f, 0.15f, 0.15f);
			}
			else {
				DrawRectangle(display_buffer,
							  min_x, min_y, max_x, max_y,
							  1.0f, 0.0f, 0.0f);
			}
		}
	}
#endif

	// f32 player_position_tile_x = (f32)(playground_state->player.position.tile_x % playground_state->tile_count_x);
	// f32 player_min_x = ((player_position_tile_x * playground_state->tile_side_in_pixels) +
	// 					(playground_state->player.position.xy.x * playground_state->meters_to_pixels) -
	// 					(playground_state->player_width * playground_state->meters_to_pixels * 0.5f));
	// f32 player_min_y = ((display_buffer->height - playground_state->player.position.tile_y * playground_state->tile_side_in_pixels) -
	// 					(playground_state->player.position.xy.y * playground_state->meters_to_pixels) -
	// 					(playground_state->player_height * playground_state->meters_to_pixels * 0.5f));
	// f32 player_max_x = player_min_x + (playground_state->player_width * playground_state->meters_to_pixels);
	// f32 player_max_y = player_min_y + (playground_state->player_height * playground_state->meters_to_pixels);
	// DrawRectangleWithBorder(display_buffer,
	// 						player_min_x, player_min_y,
	// 						player_max_x, player_max_y,
	// 						1.0f, 0.4f, 0.2f,
	// 						5,
	// 						0.7f, 0.3f, 0.5f,
	// 						true);

	// TileMap* current_tile_map = GetTileMap(playground_state, playground_state->camera.tile_x);
	// for (u32 entity_index_index = 0;
	// 	 entity_index_index < current_tile_map->entity_index_count;
	// 	 ++entity_index_index) {
	for (u32 entity_index = 1;
		 entity_index < playground_state->entity_count;
		 ++entity_index) {
		
		// u32 entity_index = current_tile_map->entity_indices[entity_index_index];
		Entity* entity = GetEntity(playground_state, entity_index);

		f32 entity_ground_point_x = screen_center.x + playground_state->meters_to_pixels * entity->position.x;
		f32 entity_ground_point_y = screen_center.y - playground_state->meters_to_pixels * entity->position.y;

		v2 entity_min = v2(entity_ground_point_x - (0.5f * playground_state->meters_to_pixels * entity->width),
						   entity_ground_point_y - (playground_state->meters_to_pixels * entity->height));
		v2 entity_max = v2(entity_min.x + entity->width * playground_state->meters_to_pixels,
						   entity_min.y + entity->height * playground_state->meters_to_pixels);

		if (entity->type == EntityType::PLAYER_TYPE) {
			// DrawRectangleWithBorder(display_buffer,
			// 						entity_min.x, entity_min.y,
			// 						entity_max.x, entity_max.y,
			// 						1.0f, 0.4f, 0.2f,
			// 						5,
			// 						0.7f, 0.3f, 0.5f,
			// 						true);

			LoadedBmp* player_bitmap = GetPlayerBitmap(playground_state);

			b32 flip_horizontally = false;
			if (entity->facing_direction == 1) {
				flip_horizontally = true;
			}

			DrawBitmap(display_buffer, player_bitmap,
					   entity_ground_point_x, entity_ground_point_y,
					   player_bitmap->align_x,
					   player_bitmap->align_y,
					   flip_horizontally);
		}
		else if (entity->type == EntityType::MONSTER_TYPE) {
			UpdateMonster(playground_state, entity_index, input->delta_time_for_frame);
			DrawRectangleWithBorder(display_buffer,
									entity_min.x, entity_min.y,
									entity_max.x, entity_max.y,
									1.0f, 0.5f, 0.0f,
									5,
									1.0f, 0.5f, 0.0f,
									true);
		}
		else {
			DrawRectangleWithBorder(display_buffer,
									entity_min.x, entity_min.y,
									entity_max.x, entity_max.y,
									0.18f, 0.60f, 0.25f,
									1,
									0.15f, 0.15f, 0.15f);
		}
	}
}
