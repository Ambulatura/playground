#include "playground.h"
#include "playground_draw.cpp"
#include "playground_world.cpp"
#include "playground_entity.cpp"

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
			player_bitmap_state->bitmaps[player_bitmap_state->bitmap_count++] = bitmap;
		}
		else {
			player_bitmap_state->bitmaps[player_bitmap_state->bitmap_count++] = bitmap;
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

			result = player_bitmap_state->bitmaps[bitmap_index];

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

internal void AddFireballBitmap(PlaygroundState* playground_state, LoadedBmp* bitmap, FireballStateType state_type)
{
	FireballBitmapState* fireball_bitmap_state = &playground_state->fireball_bitmap_state;
	if (state_type == FireballStateType::MAX_FIREBALL_STATE_TYPE) {
		fireball_bitmap_state->state_types[fireball_bitmap_state->state_count] = state_type;
		fireball_bitmap_state->index_offsets[fireball_bitmap_state->state_count] = fireball_bitmap_state->bitmap_count;
		fireball_bitmap_state->current_state = FireballStateType::NULL_FIREBALL_STATE_TYPE;

	}
	else {
		ASSERT(fireball_bitmap_state->bitmap_count < ARRAY_COUNT(fireball_bitmap_state->bitmaps));
		if (fireball_bitmap_state->state_count == 0 || state_type != fireball_bitmap_state->state_types[fireball_bitmap_state->state_count - 1]) {
			fireball_bitmap_state->state_types[fireball_bitmap_state->state_count] = state_type;
			fireball_bitmap_state->index_offsets[fireball_bitmap_state->state_count++] = fireball_bitmap_state->bitmap_count;
			fireball_bitmap_state->bitmaps[fireball_bitmap_state->bitmap_count++] = bitmap;
		}
		else {
			fireball_bitmap_state->bitmaps[fireball_bitmap_state->bitmap_count++] = bitmap;
		}
	}
}

internal LoadedBmp* GetFireballBitmap(PlaygroundState* playground_state)
{
	LoadedBmp* result = 0;

	FireballBitmapState* fireball_bitmap_state = &playground_state->fireball_bitmap_state;

	ASSERT(fireball_bitmap_state->state_types[ARRAY_COUNT(fireball_bitmap_state->state_types) - 1] ==
		   FireballStateType::MAX_FIREBALL_STATE_TYPE);

	++fireball_bitmap_state->tick_counter;

	if (fireball_bitmap_state->last_state != fireball_bitmap_state->current_state) {
		fireball_bitmap_state->bitmap_index_without_offset = 0;
		fireball_bitmap_state->tick_counter = 0;

		fireball_bitmap_state->last_state = fireball_bitmap_state->current_state;
	}

	for (u32 state_index = 0; state_index < fireball_bitmap_state->state_count; ++state_index) {
		if (fireball_bitmap_state->state_types[state_index] == fireball_bitmap_state->current_state) {
			u32 bitmap_index = fireball_bitmap_state->index_offsets[state_index] +
				fireball_bitmap_state->bitmap_index_without_offset;

			result = fireball_bitmap_state->bitmaps[bitmap_index];

			if (fireball_bitmap_state->tick_counter % 4 == 0) {
				++fireball_bitmap_state->bitmap_index_without_offset;

				u32 total_state_count = fireball_bitmap_state->index_offsets[state_index + 1] -
					fireball_bitmap_state->index_offsets[state_index];
				fireball_bitmap_state->bitmap_index_without_offset %= total_state_count;
			}
		}
	}

	// fireball_bitmap_state->current_state = FireballStateType::NULL_FIREBALL_STATE_TYPE;

	return result;
}

// NOTE(SSJSR): Function signature:
// PlaygroundUpdateAndRender(PlaygroundMemory* memory, PlaygroundDisplayBuffer* display_buffer, PlaygroundInput* input)
extern "C" PLAYGROUND_UPDATE_AND_RENDER(PlaygroundUpdateAndRender)
{
	ASSERT(&input->terminator - &input->buttons[0] == ARRAY_COUNT(input->buttons));
	ASSERT(sizeof(PlaygroundState) < memory->permanent_storage_size);

	PlaygroundState* playground_state = (PlaygroundState*)memory->permanent_storage;
	World* world = &playground_state->world;

	if (!memory->is_initialized) {
		AddEntity(world, EntityType::NULL_TYPE, 0);

		playground_state->screen_center = v2((f32)(display_buffer->width / 2), (f32)(display_buffer->height / 2));
		world->tile_side_in_pixels = 30.0f;
		world->tile_side_in_meters = 1.0f;
		world->meters_to_pixels = world->tile_side_in_pixels / world->tile_side_in_meters;
		world->tile_count_y = (display_buffer->height / (u32)world->tile_side_in_pixels) + 1;
		world->tile_count_x = (display_buffer->width / (u32)world->tile_side_in_pixels) + 1;

		InitializeArena(&playground_state->arena,
						memory->permanent_storage_size - sizeof(*playground_state),
						(u8*)memory->permanent_storage + sizeof(*playground_state));

		world->tile_map_count_x = 10;
		world->tile_map_count_y = 1;
		world->tile_maps =
			PushArray(&playground_state->arena, world->tile_map_count_x * world->tile_map_count_y, TileMap);

		for (u32 tile_map_y = 0;
			 tile_map_y < world->tile_map_count_y;
			 ++tile_map_y) {
			for (u32 tile_map_x = 0;
				 tile_map_x < world->tile_map_count_x;
				 ++tile_map_x) {

				world->tile_maps[tile_map_y * world->tile_map_count_x + tile_map_x].tiles =
					PushArray(&playground_state->arena,
							  world->tile_count_y * world->tile_count_x,
							  u32);

				world->tile_maps[tile_map_y * world->tile_map_count_x + tile_map_x].tile_map_index = tile_map_x;

				for (i32 y = 0; y < world->tile_count_y; ++y) {
					for (i32 x = 0; x < world->tile_count_x; ++x) {
						i32 tile_x = tile_map_x * world->tile_count_x + x;
						i32 tile_y = tile_map_y * world->tile_count_y + y;

						if (y == 0) {
							AddWall(world, tile_x, tile_y);
						}
						else if ((x == 0 || x == world->tile_count_x - 1) &&
								 y != 0 &&
								 y != 1 &&
								 y != 2) {
							AddWall(world, tile_x, tile_y);
						}
						// if (tile_x == world->tile_count_x - 1 &&
						// 		 tile_y == 16) {
						// 	AddWall(world, tile_x, tile_y);
						// }
					}
				}
			}
		}

		// NOTE(SSJSR): Add player to map.
		world->player_entity_index =
			AddPlayer(world);

		playground_state->background = LoadBmp("background_day_scaled.bmp", memory->PlaygroundReadFile, 0, 0);

		playground_state->fireball_00 = LoadBmp("fireball/FB001.bmp", memory->PlaygroundReadFile, 44, 22);
		playground_state->fireball_01 = LoadBmp("fireball/FB002.bmp", memory->PlaygroundReadFile, 44, 22);
		playground_state->fireball_02 = LoadBmp("fireball/FB003.bmp", memory->PlaygroundReadFile, 44, 22);
		playground_state->fireball_03 = LoadBmp("fireball/FB004.bmp", memory->PlaygroundReadFile, 44, 22);
		playground_state->fireball_04 = LoadBmp("fireball/FB005.bmp", memory->PlaygroundReadFile, 44, 22);

		AddFireballBitmap(playground_state, &playground_state->fireball_00, FireballStateType::CASTING_FIREBALL_STATE_TYPE);
		AddFireballBitmap(playground_state, &playground_state->fireball_01, FireballStateType::CASTING_FIREBALL_STATE_TYPE);
		AddFireballBitmap(playground_state, &playground_state->fireball_02, FireballStateType::CASTING_FIREBALL_STATE_TYPE);
		AddFireballBitmap(playground_state, &playground_state->fireball_03, FireballStateType::CASTING_FIREBALL_STATE_TYPE);
		AddFireballBitmap(playground_state, &playground_state->fireball_04, FireballStateType::CASTING_FIREBALL_STATE_TYPE);

		// NOTE(SSJSR): Idle state.

		playground_state->player_idle_00 = LoadBmp("adventurer/idle/adventurer-idle-2-00.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_idle_01 = LoadBmp("adventurer/idle/adventurer-idle-2-01.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_idle_02 = LoadBmp("adventurer/idle/adventurer-idle-2-02.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_idle_03 = LoadBmp("adventurer/idle/adventurer-idle-2-03.bmp", memory->PlaygroundReadFile, 25, 35);

		AddPlayerBitmap(playground_state, &playground_state->player_idle_00, PlayerStateType::IDLE_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_idle_01, PlayerStateType::IDLE_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_idle_02, PlayerStateType::IDLE_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_idle_03, PlayerStateType::IDLE_STATE_TYPE);

		// NOTE(SSJSR): Run state.

		playground_state->player_run_00 = LoadBmp("adventurer/run/adventurer-run3-00.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_run_01 = LoadBmp("adventurer/run/adventurer-run3-01.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_run_02 = LoadBmp("adventurer/run/adventurer-run3-02.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_run_03 = LoadBmp("adventurer/run/adventurer-run3-03.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_run_04 = LoadBmp("adventurer/run/adventurer-run3-04.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_run_05 = LoadBmp("adventurer/run/adventurer-run3-05.bmp", memory->PlaygroundReadFile, 25, 35);

		AddPlayerBitmap(playground_state, &playground_state->player_run_00, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_run_01, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_run_02, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_run_03, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_run_04, PlayerStateType::RUN_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_run_05, PlayerStateType::RUN_STATE_TYPE);

		// NOTE(SSJSR): Jump state.

		playground_state->player_jump_00 = LoadBmp("adventurer/jump/adventurer-jump-00.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_jump_01 = LoadBmp("adventurer/jump/adventurer-jump-01.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_jump_02 = LoadBmp("adventurer/jump/adventurer-jump-02.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_jump_03 = LoadBmp("adventurer/jump/adventurer-jump-03.bmp", memory->PlaygroundReadFile, 25, 35);

		AddPlayerBitmap(playground_state, &playground_state->player_jump_00, PlayerStateType::JUMP_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_jump_01, PlayerStateType::JUMP_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_jump_02, PlayerStateType::JUMP_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_jump_03, PlayerStateType::JUMP_STATE_TYPE);

		// NOTE(SSJSR): Cast state.

		playground_state->player_cast_00 = LoadBmp("adventurer/cast/adventurer-cast-00.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_cast_01 = LoadBmp("adventurer/cast/adventurer-cast-01.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_cast_02 = LoadBmp("adventurer/cast/adventurer-cast-02.bmp", memory->PlaygroundReadFile, 25, 35);
		playground_state->player_cast_03 = LoadBmp("adventurer/cast/adventurer-cast-03.bmp", memory->PlaygroundReadFile, 25, 35);

		AddPlayerBitmap(playground_state, &playground_state->player_cast_00, PlayerStateType::CAST_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_cast_01, PlayerStateType::CAST_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_cast_02, PlayerStateType::CAST_STATE_TYPE);
		AddPlayerBitmap(playground_state, &playground_state->player_cast_03, PlayerStateType::CAST_STATE_TYPE);

		AddPlayerBitmap(playground_state, 0, PlayerStateType::MAX_STATE_TYPE);
		AddFireballBitmap(playground_state, 0, FireballStateType::MAX_FIREBALL_STATE_TYPE);

		AddWall(world, 7, 7);

		AddMonster(world, world->tile_count_x - 7, 10);

		world->camera.tile_x = world->tile_count_x / 2;
		world->camera.tile_y = world->tile_count_y / 2;
		world->camera.xy = v2(0.0f, 0.0f);

		world->desired_camera = world->camera;

		world->is_camera_moving = false;
		world->camera_movement_duration = 5; // Frame
		world->camera_movement_duration_remaining =
			world->camera_movement_duration;

		// NOTE(SSJSR): Update first entities.
		// SetCameraLocationAndUpdateEntities(world, world->camera, true);

		memory->is_initialized = true;
	}

	Entity* player_entity = GetEntity(world, world->player_entity_index);

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

	if (input->mouse_left.is_down) {
		playground_state->screen_center +=
			(v2((f32)(display_buffer->width / 2), (f32)(display_buffer->height / 2)) -
			 v2((f32)input->mouse_x, (f32)input->mouse_y)) * (1.0f / world->tile_side_in_pixels);
	}

	if (input->mouse_middle.is_down) {
		playground_state->screen_center = v2((f32)(display_buffer->width / 2), (f32)(display_buffer->height / 2));
	}

	if (input->numpad_2.is_down) {
		world->meters_to_pixels += 1.0f;
	}

	if (input->numpad_3.is_down) {
		world->meters_to_pixels -= 1.0f;
	}

	if (input->numpad_4.is_down) {
		world->meters_to_pixels = world->tile_side_in_pixels / world->tile_side_in_meters;
	}

	SetCameraLocationAndUpdateEntities(world, world->camera);

	DrawBitmap(display_buffer, &playground_state->background,
			   0, 0);
	
	local_persist u32 ball = 0;
	// for (u32 entity_index = 1;
	// 	 entity_index < world->entity_count;
	// 	 ++entity_index) {
	for (u32 active_entity_index_index = 0; active_entity_index_index < world->active_entity_count; ++active_entity_index_index) {

		u32 entity_index = world->active_entity_indices[active_entity_index_index];
		Entity* entity = GetEntity(world, entity_index);

		if (!IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG)) {

			f32 entity_ground_point_x = playground_state->screen_center.x + world->meters_to_pixels * entity->position.x;
			f32 entity_ground_point_y = playground_state->screen_center.y - world->meters_to_pixels * entity->position.y;

			v2 entity_min = v2(entity_ground_point_x - (0.5f * world->meters_to_pixels * entity->width),
							   entity_ground_point_y - (world->meters_to_pixels * entity->height));
			v2 entity_max = v2(entity_min.x + entity->width * world->meters_to_pixels,
							   entity_min.y + entity->height * world->meters_to_pixels);

			if (entity->type == EntityType::PLAYER_TYPE) {
				MoveEntity(world, world->player_entity_index, player_entity, player_entity->direction, input->delta_time_for_frame);

				Entity* ball_entity = GetEntity(world, entity->ball_index);
				if (input->numpad_1.is_down && IsFlagSet(ball_entity, EntityFlag::NONSPATIAL_FLAG)) {
					playground_state->player_bitmap_state.current_state = PlayerStateType::CAST_STATE_TYPE;
				}

				LoadedBmp* player_bitmap = GetPlayerBitmap(playground_state);

				if (player_bitmap == &playground_state->player_cast_03) {
					ball_entity->distance_limit = 5.0f;
					f32 direction_x = entity->facing_direction == 1 ? -1.0f : 1.0f;
					MakeEntitySpatial(ball_entity,
									  v2(direction_x, 0.0f),
									  v2(entity->position.x + 1.0f * direction_x,
										 entity->position.y + entity->height * 0.4f),
									  v2(10.0f * direction_x, 0.0f));
				}

				// DrawRectangleWithBorder(display_buffer,
				// 						entity_min.x, entity_min.y,
				// 						entity_max.x, entity_max.y,
				// 						1.0f, 0.4f, 0.2f,
				// 						5,
				// 						0.7f, 0.3f, 0.5f,
				// 						true);

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
			else if (entity->type == EntityType::BALL_TYPE) {
				playground_state->fireball_bitmap_state.current_state = FireballStateType::CASTING_FIREBALL_STATE_TYPE;
				LoadedBmp* fireball = GetFireballBitmap(playground_state);

				b32 flip_horizontally = entity->facing_direction == 1 ? true : false;

				// UpdateBall(world, entity_index, entity, input->delta_time_for_frame);

				MoveEntity(world, entity_index, entity, entity->direction, input->delta_time_for_frame);
				++ball;
				if (entity->distance_limit == 0.0f) {
					MakeEntityNonspatialAndDeleteFromTileMap(world, entity, entity_index);
					ball = 0;
				}
					
				DrawBitmap(display_buffer, fireball,
						   entity_ground_point_x, entity_ground_point_y,
						   fireball->align_x,
						   fireball->align_y,
						   flip_horizontally);

				// DrawRectangleWithBorder(display_buffer,
				// 						entity_min.x, entity_min.y,
				// 						entity_max.x, entity_max.y,
				// 						1.0f, 0.4f, 0.2f,
				// 						5,
				// 						0.7f, 0.3f, 0.5f,
				// 						true);
			}
			else if (entity->type == EntityType::MONSTER_TYPE) {
				UpdateMonster(world, entity_index, input->delta_time_for_frame);
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

			TilePosition new_tile_position =
				IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG) ?
				InvalidTilePosition() :
				MapIntoTilePosition(world->camera, entity->position, world->tile_side_in_meters);
				
			UpdateEntityTileMapAndTilePosition(world, entity, entity_index, &new_tile_position);
		}
	}

	TilePosition new_camera = world->camera;

#if 0
	new_camera.tile_x = player_entity->tile_position.tile_x;
	new_camera.xy.x = player_entity->tile_position.xy.x;
#else
	// if (player_entity->position.x > 0.5f * world->tile_count_x * world->tile_side_in_meters) {
	// 	new_camera.tile_x += world->tile_count_x;
	// }
	// if (player_entity->position.x < -0.5f * world->tile_count_x * world->tile_side_in_meters) {
	// 	new_camera.tile_x -= world->tile_count_x;
	// }

	if (!world->is_camera_moving) {
		if (player_entity->position.x > 0.5f * world->tile_count_x * world->tile_side_in_meters) {
			world->desired_camera.tile_x += world->tile_count_x;
			world->is_camera_moving = true;
		}

		if (player_entity->position.x < -0.5f * world->tile_count_x * world->tile_side_in_meters) {
			world->desired_camera.tile_x -= world->tile_count_x;
			world->is_camera_moving = true;
		}
	}
	else {
		if (world->camera_movement_duration_remaining > 0) {

			f32 camera_movement_per_frame = world->tile_count_x / (f32)world->camera_movement_duration;
			if (new_camera.tile_x <= world->desired_camera.tile_x) {
				new_camera.xy.x += camera_movement_per_frame;
			}
			else {
				new_camera.xy.x -= camera_movement_per_frame;
			}
			NormalizePositions(&new_camera, world->tile_side_in_meters);
			--world->camera_movement_duration_remaining;
		}
		else {
			v2 difference = TilePositionDifference(world->desired_camera, new_camera, world->tile_side_in_meters);
			if (new_camera.tile_x <= world->desired_camera.tile_x) {
				new_camera.xy.x += difference.x;

			}
			else {
				new_camera.xy.x -= -difference.x;
			}
			NormalizePositions(&new_camera, world->tile_side_in_meters);
			world->is_camera_moving = false;
			world->camera_movement_duration_remaining = world->camera_movement_duration;
		}
	}

#endif

	world->camera = new_camera;

}
