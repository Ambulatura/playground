#include "playground.h"
#include "playground_draw.cpp"
#include "playground_world.cpp"
#include "playground_entity.cpp"

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

		playground_state->screen_center = v2((f32)(display_buffer->width / 2),
											 (f32)(display_buffer->height / 2));

		InitializeWorld(world, display_buffer->width, display_buffer->height);

		InitializeArena(&playground_state->arena,
						memory->permanent_storage_size - sizeof(*playground_state),
						(u8*)memory->permanent_storage + sizeof(*playground_state));

		world->world_arena = &playground_state->arena;

		for (i32 tile_map_y = 0;
			 tile_map_y < (i32)world->tile_map_count_y;
			 ++tile_map_y) {
			for (i32 tile_map_x = 0;
				 tile_map_x < (i32)world->tile_map_count_x;
				 ++tile_map_x) {

				i32 tile_x_0 = tile_map_x * world->tile_count_x + 0;
				i32 tile_y_0 = tile_map_y * world->tile_count_y + 0;

				if (tile_map_x == 0) {
					AddWall(world, tile_x_0, tile_y_0 + 1,
							world->tile_side_in_meters, (f32)world->tile_count_y - 2);
				}
				else {
					AddWall(world, tile_x_0, tile_y_0 + 3,
							world->tile_side_in_meters, (f32)world->tile_count_y - 4);
				}

				AddWall(world, tile_x_0, tile_y_0 + world->tile_count_y - 1,
						(f32)world->tile_count_x, world->tile_side_in_meters);

				AddWall(world, tile_x_0 + world->tile_count_x - 1, tile_y_0 + 3,
						world->tile_side_in_meters, (f32)world->tile_count_y - 4);

				AddWall(world, tile_x_0, tile_y_0,
						(f32)world->tile_count_x, world->tile_side_in_meters);

				// for (i32 y = 0; y < world->tile_count_y; ++y) {
				// 	for (i32 x = 0; x < world->tile_count_x; ++x) {
				// 		i32 tile_x = tile_map_x * world->tile_count_x + x;
				// 		i32 tile_y = tile_map_y * world->tile_count_y + y;

				// 		if (x == 0 && tile_map_x == 0) {
				// 			AddWall(world, tile_x, tile_y,world->tile_side_in_meters, world->tile_side_in_meters);
				// 		}
				// 		else if (y == 0) {
				// 			AddWall(world, tile_x, tile_y, world->tile_side_in_meters, world->tile_side_in_meters);
				// 		}
				// 		else if ((x == 0 || x == world->tile_count_x - 1) &&
				// 				 y != 0 &&
				// 				 y != 1 &&
				// 				 y != 2) {
				// 			AddWall(world, tile_x, tile_y, world->tile_side_in_meters, world->tile_side_in_meters);
				// 		}
				// 		else if (y == world->tile_count_y - 1) {
				// 			AddWall(world, tile_x, tile_y, world->tile_side_in_meters, world->tile_side_in_meters);
				// 		}
				// 	}
				// }
			}
		}

		playground_state->background = LoadBmp("background_day_scaled.bmp", memory->PlaygroundReadFile, 0, 0);
		playground_state->background = ScaleBmp(&playground_state->arena,
												&playground_state->background,
												playground_state->background.width * 2, playground_state->background.height * 2);

		// NOTE(SSJSR): PLAYER!
		{
			// playground_state->fireball_00 = LoadBmp("fireball/FB001.bmp", memory->PlaygroundReadFile, 44, 15);
			// playground_state->fireball_01 = LoadBmp("fireball/FB002.bmp", memory->PlaygroundReadFile, 44, 15);
			// playground_state->fireball_02 = LoadBmp("fireball/FB003.bmp", memory->PlaygroundReadFile, 44, 15);
			// playground_state->fireball_03 = LoadBmp("fireball/FB004.bmp", memory->PlaygroundReadFile, 44, 15);
			// playground_state->fireball_04 = LoadBmp("fireball/FB005.bmp", memory->PlaygroundReadFile, 44, 15);

			// NOTE(SSJSR): Idle state.

			f32 player_scale = 2.0f;

			playground_state->player_idle_00 = LoadBmp("adventurer/idle/adventurer-idle-2-00.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_idle_01 = LoadBmp("adventurer/idle/adventurer-idle-2-01.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_idle_02 = LoadBmp("adventurer/idle/adventurer-idle-2-02.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_idle_03 = LoadBmp("adventurer/idle/adventurer-idle-2-03.bmp", memory->PlaygroundReadFile, 25, 22);

			playground_state->player_idle_00 = ScaleBmp(&playground_state->arena,
														&playground_state->player_idle_00,
														(i32)(playground_state->player_idle_00.width * player_scale),
														(i32)(playground_state->player_idle_00.height * player_scale));
			playground_state->player_idle_01 = ScaleBmp(&playground_state->arena,
														&playground_state->player_idle_01,
														(i32)(playground_state->player_idle_01.width * player_scale),
														(i32)(playground_state->player_idle_01.height * player_scale));
			playground_state->player_idle_02 = ScaleBmp(&playground_state->arena,
														&playground_state->player_idle_02,
														(i32)(playground_state->player_idle_02.width * player_scale),
														(i32)(playground_state->player_idle_02.height * player_scale));
			playground_state->player_idle_03 = ScaleBmp(&playground_state->arena,
														&playground_state->player_idle_03,
														(i32)(playground_state->player_idle_03.width * player_scale),
														(i32)(playground_state->player_idle_03.height * player_scale));

			Bitmap* player_idle_sprites[4] = {
				&playground_state->player_idle_00,
				&playground_state->player_idle_01,
				&playground_state->player_idle_02,
				&playground_state->player_idle_03
			};
			u32 player_idle_frame_counts[1] = { 4 };
			playground_state->player_idle_animations = MakeAnimationGroup(playground_state,
																		  1, 0.4f,
																		  AnimationType::IDLE_ANIMATION_TYPE,
																		  player_idle_sprites, player_idle_frame_counts);
			
			// NOTE(SSJSR): Run state.

			playground_state->player_run_00 = LoadBmp("adventurer/run/adventurer-run3-00.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_run_01 = LoadBmp("adventurer/run/adventurer-run3-01.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_run_02 = LoadBmp("adventurer/run/adventurer-run3-02.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_run_03 = LoadBmp("adventurer/run/adventurer-run3-03.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_run_04 = LoadBmp("adventurer/run/adventurer-run3-04.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_run_05 = LoadBmp("adventurer/run/adventurer-run3-05.bmp", memory->PlaygroundReadFile, 25, 22);

			playground_state->player_run_00 = ScaleBmp(&playground_state->arena,
													   &playground_state->player_run_00,
													   (i32)(playground_state->player_run_00.width * player_scale),
													   (i32)(playground_state->player_run_00.height * player_scale));
			playground_state->player_run_01 = ScaleBmp(&playground_state->arena,
													   &playground_state->player_run_01,
													   (i32)(playground_state->player_run_01.width * player_scale),
													   (i32)(playground_state->player_run_01.height * player_scale));
			playground_state->player_run_02 = ScaleBmp(&playground_state->arena,
													   &playground_state->player_run_02,
													   (i32)(playground_state->player_run_02.width * player_scale),
													   (i32)(playground_state->player_run_02.height * player_scale));
			playground_state->player_run_03 = ScaleBmp(&playground_state->arena,
													   &playground_state->player_run_03,
													   (i32)(playground_state->player_run_03.width * player_scale),
													   (i32)(playground_state->player_run_03.height * player_scale));
			playground_state->player_run_04 = ScaleBmp(&playground_state->arena,
													   &playground_state->player_run_04,
													   (i32)(playground_state->player_run_04.width * player_scale),
													   (i32)(playground_state->player_run_04.height * player_scale));
			playground_state->player_run_05 = ScaleBmp(&playground_state->arena,
													   &playground_state->player_run_05,
													   (i32)(playground_state->player_run_05.width * player_scale),
													   (i32)(playground_state->player_run_05.height * player_scale));

			Bitmap* player_run_sprites[6] = {
				&playground_state->player_run_00,
				&playground_state->player_run_01,
				&playground_state->player_run_02,
				&playground_state->player_run_03,
				&playground_state->player_run_04,
				&playground_state->player_run_05
			};
			u32 player_run_frame_counts[1] = { 6 };
			playground_state->player_run_animations = MakeAnimationGroup(playground_state,
																		 1, 0.6f,
																		 AnimationType::RUN_ANIMATION_TYPE,
																		 player_run_sprites, player_run_frame_counts);

			// NOTE(SSJSR): Jump state.

			// Bitmap player_jump_00 = LoadBmp("adventurer/jump/adventurer-jump-00.bmp", memory->PlaygroundReadFile, 25, 22);
			// Bitmap player_jump_01 = LoadBmp("adventurer/jump/adventurer-jump-01.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_jump_02 = LoadBmp("adventurer/jump/adventurer-jump-02.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_jump_03 = LoadBmp("adventurer/jump/adventurer-jump-03.bmp", memory->PlaygroundReadFile, 25, 22);

			playground_state->player_jump_02 = ScaleBmp(&playground_state->arena,
														&playground_state->player_jump_02,
														(i32)(playground_state->player_jump_02.width * player_scale),
														(i32)(playground_state->player_jump_02.height * player_scale));
			playground_state->player_jump_03 = ScaleBmp(&playground_state->arena,
														&playground_state->player_jump_03,
														(i32)(playground_state->player_jump_03.width * player_scale),
														(i32)(playground_state->player_jump_03.height * player_scale));

			Bitmap* player_jump_sprites[2] = {
				&playground_state->player_jump_02,
				&playground_state->player_jump_03
			};
			u32 player_jump_frame_counts[1] = { 2 };
			playground_state->player_jump_animations = MakeAnimationGroup(playground_state,
																		  1, 0.2f,
																		  AnimationType::JUMP_ANIMATION_TYPE,
																		  player_jump_sprites, player_jump_frame_counts);

			// NOTE(SSJSR): Jump 2 state.

			playground_state->player_jump_2_00 = LoadBmp("adventurer/jump/adventurer-smrslt-00.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_jump_2_01 = LoadBmp("adventurer/jump/adventurer-smrslt-01.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_jump_2_02 = LoadBmp("adventurer/jump/adventurer-smrslt-02.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_jump_2_03 = LoadBmp("adventurer/jump/adventurer-smrslt-03.bmp", memory->PlaygroundReadFile, 25, 22);

			playground_state->player_jump_2_00 = ScaleBmp(&playground_state->arena,
														  &playground_state->player_jump_2_00,
														  (i32)(playground_state->player_jump_2_00.width * player_scale),
														  (i32)(playground_state->player_jump_2_00.height * player_scale));
			playground_state->player_jump_2_01 = ScaleBmp(&playground_state->arena,
														  &playground_state->player_jump_2_01,
														  (i32)(playground_state->player_jump_2_01.width * player_scale),
														  (i32)(playground_state->player_jump_2_01.height * player_scale));
			playground_state->player_jump_2_02 = ScaleBmp(&playground_state->arena,
														  &playground_state->player_jump_2_02,
														  (i32)(playground_state->player_jump_2_02.width * player_scale),
														  (i32)(playground_state->player_jump_2_02.height * player_scale));
			playground_state->player_jump_2_03 = ScaleBmp(&playground_state->arena,
														  &playground_state->player_jump_2_03,
														  (i32)(playground_state->player_jump_2_03.width * player_scale),
														  (i32)(playground_state->player_jump_2_03.height * player_scale));

			Bitmap* player_jump_2_sprites[4] = {
				&playground_state->player_jump_2_00,
				&playground_state->player_jump_2_01,
				&playground_state->player_jump_2_02,
				&playground_state->player_jump_2_03
			};
			u32 player_jump_2_frame_counts[1] = { 4 };
			playground_state->player_jump_2_animations = MakeAnimationGroup(playground_state,
																			1, 0.2f,
																			AnimationType::JUMP_2_ANIMATION_TYPE,
																			player_jump_2_sprites, player_jump_2_frame_counts);

			// NOTE(SSJSR): Fall state.

			playground_state->player_fall_00 = LoadBmp("adventurer/fall/adventurer-fall-00.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_fall_01 = LoadBmp("adventurer/fall/adventurer-fall-01.bmp", memory->PlaygroundReadFile, 25, 22);

			playground_state->player_fall_00 = ScaleBmp(&playground_state->arena,
														&playground_state->player_fall_00,
														(i32)(playground_state->player_fall_00.width * player_scale),
														(i32)(playground_state->player_fall_00.height * player_scale));
			playground_state->player_fall_01 = ScaleBmp(&playground_state->arena,
														&playground_state->player_fall_01,
														(i32)(playground_state->player_fall_01.width * player_scale),
														(i32)(playground_state->player_fall_01.height * player_scale));

			Bitmap* player_fall_sprites[2] = {
				&playground_state->player_fall_00,
				&playground_state->player_fall_01,
			};
			u32 player_fall_frame_counts[1] = { 2 };
			playground_state->player_fall_animations = MakeAnimationGroup(playground_state, 1, 0.2f, AnimationType::FALL_ANIMATION_TYPE, player_fall_sprites, player_fall_frame_counts);

			// NOTE(SSJSR): Wall slide state.

			playground_state->player_wall_slide_00 = LoadBmp("adventurer/wall_slide/adventurer-wall-slide-00.bmp", memory->PlaygroundReadFile, 25, 22);
			playground_state->player_wall_slide_01 = LoadBmp("adventurer/wall_slide/adventurer-wall-slide-01.bmp", memory->PlaygroundReadFile, 25, 22);

			playground_state->player_wall_slide_00 = ScaleBmp(&playground_state->arena,
															  &playground_state->player_wall_slide_00,
															  (i32)(playground_state->player_wall_slide_00.width * player_scale),
															  (i32)(playground_state->player_wall_slide_00.height * player_scale));
			playground_state->player_wall_slide_01 = ScaleBmp(&playground_state->arena,
															  &playground_state->player_wall_slide_01,
															  (i32)(playground_state->player_wall_slide_01.width * player_scale),
															  (i32)(playground_state->player_wall_slide_01.height * player_scale));

			Bitmap* player_wall_slide_sprites[2] = {
				&playground_state->player_wall_slide_00,
				&playground_state->player_wall_slide_01,
			};
			u32 player_wall_slide_frame_counts[1] = { 2 };
			playground_state->player_wall_slide_animations = MakeAnimationGroup(playground_state,
																				1, 0.2f,
																				AnimationType::WALL_SLIDE_ANIMATION_TYPE,
																				player_wall_slide_sprites, player_wall_slide_frame_counts);

			// NOTE(SSJSR): Cast state.

			// playground_state->player_cast_00 = LoadBmp("adventurer/cast/adventurer-cast-00.bmp", memory->PlaygroundReadFile, 25, 22);
			// playground_state->player_cast_01 = LoadBmp("adventurer/cast/adventurer-cast-01.bmp", memory->PlaygroundReadFile, 25, 22);
			// playground_state->player_cast_02 = LoadBmp("adventurer/cast/adventurer-cast-02.bmp", memory->PlaygroundReadFile, 25, 22);
			// playground_state->player_cast_03 = LoadBmp("adventurer/cast/adventurer-cast-03.bmp", memory->PlaygroundReadFile, 25, 22);
		}

		// NOTE(SSJSR): FAMILIAR!
		{

			f32 familiar_scale = 1.0f;
			// NOTE(SSJSR): Idle state.
			
			playground_state->familiar_idle_00 = LoadBmp("familiar/idle/owlet-monster-idle-00.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_idle_01 = LoadBmp("familiar/idle/owlet-monster-idle-01.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_idle_02 = LoadBmp("familiar/idle/owlet-monster-idle-02.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_idle_03 = LoadBmp("familiar/idle/owlet-monster-idle-03.bmp", memory->PlaygroundReadFile, 15, 14);

			playground_state->familiar_idle_00 = ScaleBmp(&playground_state->arena,
														  &playground_state->familiar_idle_00,
														  (i32)(playground_state->familiar_idle_00.width * familiar_scale),
														  (i32)(playground_state->familiar_idle_00.height * familiar_scale));
			playground_state->familiar_idle_01 = ScaleBmp(&playground_state->arena,
														  &playground_state->familiar_idle_01,
														  (i32)(playground_state->familiar_idle_01.width * familiar_scale),
														  (i32)(playground_state->familiar_idle_01.height * familiar_scale));
			playground_state->familiar_idle_02 = ScaleBmp(&playground_state->arena,
														  &playground_state->familiar_idle_02,
														  (i32)(playground_state->familiar_idle_02.width * familiar_scale),
														  (i32)(playground_state->familiar_idle_02.height * familiar_scale));
			playground_state->familiar_idle_03 = ScaleBmp(&playground_state->arena,
														  &playground_state->familiar_idle_03,
														  (i32)(playground_state->familiar_idle_03.width * familiar_scale),
														  (i32)(playground_state->familiar_idle_03.height * familiar_scale));

			Bitmap* familiar_idle_sprites[4] = {
				&playground_state->familiar_idle_00,
				&playground_state->familiar_idle_01,
				&playground_state->familiar_idle_02,
				&playground_state->familiar_idle_03
			};
			u32 familiar_idle_frame_counts[1] = { 4 };
			playground_state->familiar_idle_animations = MakeAnimationGroup(playground_state,
																			1, 0.4f,
																			AnimationType::IDLE_ANIMATION_TYPE,
																			familiar_idle_sprites, familiar_idle_frame_counts);

			// NOTE(SSJSR): Run state.
			
			playground_state->familiar_run_00 = LoadBmp("familiar/run/owlet-monster-run-00.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_01 =	LoadBmp("familiar/run/owlet-monster-run-01.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_02 =	LoadBmp("familiar/run/owlet-monster-run-02.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_03 =	LoadBmp("familiar/run/owlet-monster-run-03.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_04 = LoadBmp("familiar/run/owlet-monster-run-04.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_05 = LoadBmp("familiar/run/owlet-monster-run-05.bmp", memory->PlaygroundReadFile, 15, 14);
			
			playground_state->familiar_run_00 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_00,
														 (i32)(playground_state->familiar_run_00.width * familiar_scale),
														 (i32)(playground_state->familiar_run_00.height * familiar_scale));
			playground_state->familiar_run_01 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_01,
														 (i32)(playground_state->familiar_run_01.width * familiar_scale),
														 (i32)(playground_state->familiar_run_01.height * familiar_scale));
			playground_state->familiar_run_02 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_02,
														 (i32)(playground_state->familiar_run_02.width * familiar_scale),
														 (i32)(playground_state->familiar_run_02.height * familiar_scale));
			playground_state->familiar_run_03 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_03,
														 (i32)(playground_state->familiar_run_03.width * familiar_scale),
														 (i32)(playground_state->familiar_run_03.height * familiar_scale));
			playground_state->familiar_run_04 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_04,
														 (i32)(playground_state->familiar_run_04.width * familiar_scale),
														 (i32)(playground_state->familiar_run_04.height * familiar_scale));
			playground_state->familiar_run_05 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_05,
														 (i32)(playground_state->familiar_run_05.width * familiar_scale),
														 (i32)(playground_state->familiar_run_05.height * familiar_scale));

			playground_state->familiar_run_dust_00 = LoadBmp("familiar/run/owlet-run-dust-00.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_dust_01 = LoadBmp("familiar/run/owlet-run-dust-01.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_dust_02 = LoadBmp("familiar/run/owlet-run-dust-02.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_dust_03 = LoadBmp("familiar/run/owlet-run-dust-03.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_dust_04 = LoadBmp("familiar/run/owlet-run-dust-04.bmp", memory->PlaygroundReadFile, 15, 14);
			playground_state->familiar_run_dust_05 = LoadBmp("familiar/run/owlet-run-dust-05.bmp", memory->PlaygroundReadFile, 15, 14);
			
			playground_state->familiar_run_dust_00 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_dust_00,
														 (i32)(playground_state->familiar_run_dust_00.width * familiar_scale),
														 (i32)(playground_state->familiar_run_dust_00.height * familiar_scale));
			playground_state->familiar_run_dust_01 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_dust_01,
														 (i32)(playground_state->familiar_run_dust_01.width * familiar_scale),
														 (i32)(playground_state->familiar_run_dust_01.height * familiar_scale));
			playground_state->familiar_run_dust_02 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_dust_02,
														 (i32)(playground_state->familiar_run_dust_02.width * familiar_scale),
														 (i32)(playground_state->familiar_run_dust_02.height * familiar_scale));
			playground_state->familiar_run_dust_03 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_dust_03,
														 (i32)(playground_state->familiar_run_dust_03.width * familiar_scale),
														 (i32)(playground_state->familiar_run_dust_03.height * familiar_scale));
			playground_state->familiar_run_dust_04 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_dust_04,
														 (i32)(playground_state->familiar_run_dust_04.width * familiar_scale),
														 (i32)(playground_state->familiar_run_dust_04.height * familiar_scale));
			playground_state->familiar_run_dust_05 = ScaleBmp(&playground_state->arena,
														 &playground_state->familiar_run_dust_05,
														 (i32)(playground_state->familiar_run_dust_05.width * familiar_scale),
														 (i32)(playground_state->familiar_run_dust_05.height * familiar_scale));

			Bitmap* familiar_run_sprites[12] = {
				&playground_state->familiar_run_00,
				&playground_state->familiar_run_01,
				&playground_state->familiar_run_02,
				&playground_state->familiar_run_03,
				&playground_state->familiar_run_04,
				&playground_state->familiar_run_05,

				&playground_state->familiar_run_dust_00,
				&playground_state->familiar_run_dust_01,
				&playground_state->familiar_run_dust_02,
				&playground_state->familiar_run_dust_03,
				&playground_state->familiar_run_dust_04,
				&playground_state->familiar_run_dust_05
			};
			u32 familiar_run_frame_counts[2] = { 6, 6 };
			playground_state->familiar_run_animations = MakeAnimationGroup(playground_state,
																		   2, 0.6f,
																		   AnimationType::RUN_ANIMATION_TYPE,
																		   familiar_run_sprites, familiar_run_frame_counts);
		}

		world->player_entity_index =
			AddPlayer(playground_state);

		AddWall(world, 3, 3, 2.0f, 13.0f);
		AddWall(world, 8, 2, 13.0f, 2.0f);
		AddWall(world, 10, 6, 13.0f, 2.0f);

		AddMonster(world, world->tile_count_x - 7, 10);
		AddFamiliar(playground_state);

		world->camera.tile_x = world->tile_count_x / 2;
		world->camera.tile_y = world->tile_count_y / 2;
		world->camera.xy = v2(0.0f, 0.0f);

		world->desired_camera = world->camera;

		world->is_camera_moving = false;
		world->camera_movement_duration = 10; // Frame
		world->camera_movement_duration_remaining =
			world->camera_movement_duration;

		memory->is_initialized = true;
	}

	if (input->mouse_left.is_down) {
		playground_state->screen_center +=
			(v2((f32)(display_buffer->width / 2), (f32)(display_buffer->height / 2)) -
			 v2((f32)input->mouse_x, (f32)input->mouse_y)) * (1.0f / world->tile_side_in_pixels);
	}

	if (input->mouse_right.is_down) {
		playground_state->screen_center = v2((f32)(display_buffer->width / 2), (f32)(display_buffer->height / 2));
	}

	if (input->scrolling) {
		if (input->wheel_moving_forward) {
			world->meters_to_pixels += 1.0f;
		}
		else {
			world->meters_to_pixels -= 1.0f;
		}
	}

	if (input->mouse_middle.is_down) {
		world->meters_to_pixels = world->tile_side_in_pixels / world->tile_side_in_meters;
	}

	SetCameraLocationAndUpdateEntities(world,
									   world->camera,
									   input->delta_time_for_frame);

	Bitmap draw_buffer;
	draw_buffer.memory = display_buffer->memory;
	draw_buffer.width = display_buffer->width;
	draw_buffer.height = display_buffer->height;
	draw_buffer.pitch = display_buffer->pitch;
	DrawRectangle(&draw_buffer,
				  v2(),
				  v2((f32)display_buffer->width, (f32)display_buffer->height),
				  0.6f, 0.6f, 0.6f);
	// DrawBitmap(&draw_buffer, &playground_state->background, 0, 0);

	// for (u32 entity_index = 1;
	// 	 entity_index < world->entity_count;
	// 	 ++entity_index) {
	for (u32 active_entity_index_index = 0; active_entity_index_index < world->active_entity_count; ++active_entity_index_index) {

		u32 entity_index = world->active_entity_indices[active_entity_index_index];
		Entity* entity = GetEntity(world, entity_index);

		if (entity->updatable) {

			MoveFeature move_feature = {};

			v2 entity_position = GetEntityAlignedPosition(entity);
			v2 dimension = GetEntityDimension(entity);
			v2 half_dimension = 0.5f * dimension * world->meters_to_pixels;
			
			v2 entity_center = v2(playground_state->screen_center.x + world->meters_to_pixels * entity_position.x,
								  playground_state->screen_center.y - world->meters_to_pixels * entity_position.y);

			if (entity->type == EntityType::PLAYER_TYPE) {
				AnimationGroup* entity_animation_group = EntityStateControl(playground_state, entity, input);

				move_feature.direction = entity->direction;
				move_feature.acceleration = v2(80.0f, entity->acceleration.y);
				move_feature.friction_coefficient = 8.0;
				move_feature.max_unit_vector_length = true;

				Entity* ball_entity = GetEntity(world, entity->ball_index);
				if (input->numpad_1.is_down &&
					IsFlagSet(ball_entity, EntityFlag::NONSPATIAL_FLAG)) {
					ball_entity->distance_limit = 5.0f;
					f32 direction_x = entity->facing_direction == 1 ? -1.0f : 1.0f;
					MakeEntitySpatialAndAddToTileMap(world, ball_entity,
													 entity->ball_index,
													 v2(direction_x, 0.0f),
													 v2(entity->position.x + 1.0f * direction_x,
														entity->position.y + dimension.y * 0.14f),
													 v2(15.0f * direction_x, 0.0f));
				}

				DrawRectangleWithBorder(&draw_buffer,
										entity_center - half_dimension,
										entity_center + half_dimension,
										1.0f, 0.4f, 0.2f,
										1,
										0.7f, 0.3f, 0.5f,
										true);

				if (entity_animation_group) {
					for (u32 animation_index = 0; animation_index < entity_animation_group->animation_count; ++animation_index) {
						Animation* entity_animation = entity_animation_group->animations + animation_index;
						Bitmap* sprite = entity_animation->frames[entity_animation->frame_index].sprite;
						b32 flip_horizontally = entity->facing_direction == 1 ?
							true : false;

						DrawBitmap(&draw_buffer, sprite,
								   entity_center.x, entity_center.y,
								   sprite->align_x, sprite->align_y,
								   flip_horizontally);
					}
				}
			}
			else if (entity->type == EntityType::FAMILIAR_TYPE) {
				Entity* player_entity = GetEntity(world, world->player_entity_index);
				
				entity->position = player_entity->position;
				entity->position.x -= player_entity->facing_direction == 1 ?
					-0.7f : 0.7f;
				entity->position.y -= 0.1f;

				// DrawRectangleWithBorder(display_buffer,
				// 						entity_min.x, entity_min.y,
				// 						entity_max.x, entity_max.y,
				// 						1.0f, 0.4f, 0.2f,
				// 						1,
				// 						1.0f, 1.0f, 1.0f,
				// 						true);

				AnimationGroup* entity_animation_group = EntityStateControl(playground_state, entity, input);

				if (entity_animation_group) {
					for (u32 animation_index = 0; animation_index < entity_animation_group->animation_count; ++animation_index) {
						Animation* entity_animation = entity_animation_group->animations + animation_index;
						Bitmap* sprite = entity_animation->frames[entity_animation->frame_index].sprite;
						b32 flip_horizontally = player_entity->facing_direction == 1 ?
							true : false;

						DrawBitmap(&draw_buffer, sprite,
								   entity_center.x, entity_center.y,
								   sprite->align_x, sprite->align_y,
								   flip_horizontally);
					}
				}
			}
			else if (entity->type == EntityType::BALL_TYPE) {
				// move_feature.direction = entity->direction;
				move_feature.acceleration = v2();
				move_feature.friction_coefficient = 0.0f;
				move_feature.max_unit_vector_length = false;

				b32 flip_horizontally = entity->facing_direction == 1 ? true : false;

				if (entity->distance_limit == 0.0f) {
					MakeEntityNonspatialAndDeleteFromTileMap(world, entity, entity_index);
				}

				DrawRectangle(&draw_buffer,
							  entity_center - half_dimension,
							  entity_center + half_dimension,
							  0.0f, 0.0f, 0.0f);
			}
			else if (entity->type == EntityType::MONSTER_TYPE) {

				if (entity->distance_limit == 0.0f) {
					if (!IsFlagSet(entity, EntityFlag::MOVEABLE_FLAG)) {
						AddFlags(entity, EntityFlag::MOVEABLE_FLAG);
						entity->distance_limit = 20.0f;
						entity->direction = v2(1.0f, -1.0f);
					}
					else {
						entity->distance_limit = 20.0f;
						entity->direction.x *= -1.0f;
					}
				}

				move_feature.direction = entity->direction;
				move_feature.acceleration = v2(30.0f, 100.0f);
				move_feature.friction_coefficient = 8.0f;
				move_feature.max_unit_vector_length = true;

				DrawRectangleWithBorder(&draw_buffer,
										entity_center - half_dimension,
										entity_center + half_dimension,
										1.0f, 0.5f, 0.0f,
										5,
										1.0f, 0.5f, 0.0f,
										true);
			}
			else {
				DrawRectangle(&draw_buffer,
							  entity_center - half_dimension,
							  entity_center + half_dimension,
							  0.18f, 0.18f, 0.18f);

				// f32 thickness = world->meters_to_pixels * 0.1f;

				// DrawRectangle(&draw_buffer,
				// 			  entity_min.x, entity_min.y - thickness * 0.5f,
				// 			  entity_max.x, entity_min.y + thickness * 0.5f,
				// 			  0.18f, 0.18f, 0.18f);

				// DrawRectangle(&draw_buffer,
				// 			  entity_min.x, entity_max.y - thickness * 0.5f,
				// 			  entity_max.x, entity_max.y + thickness * 0.5f,
				// 			  0.18f, 0.18f, 0.18f);

				// DrawRectangle(&draw_buffer,
				// 			  entity_min.x - thickness * 0.5f, entity_min.y,
				// 			  entity_min.x + thickness * 0.5f, entity_max.y,
				// 			  0.18f, 0.18f, 0.18f);

				// DrawRectangle(&draw_buffer,
				// 			  entity_max.x - thickness * 0.5f, entity_min.y,
				// 			  entity_max.x + thickness * 0.5f, entity_max.y,
				// 			  0.18f, 0.18f, 0.18f);
			}

			if (IsFlagSet(entity, EntityFlag::MOVEABLE_FLAG) &&
				!IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG)) {
				MoveEntity(world, entity_index, entity, input->delta_time_for_frame, &move_feature);
			}

			TilePosition new_tile_position =
				IsFlagSet(entity, EntityFlag::NONSPATIAL_FLAG) ?
				InvalidTilePosition() :
				MapIntoTilePosition(world->camera, entity->position, world->tile_side_in_meters);

			UpdateEntityTileMapAndTilePosition(world, entity, entity_index, &new_tile_position);
		}
	}

	TilePosition new_camera = world->camera;

	Entity* player_entity = GetEntity(world, world->player_entity_index);
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

	// for (i32 y = 0; y < world->tile_count_y; ++y) {
	// 	for (i32 x = 0; x < world->tile_count_x; ++x) {
	// 		f32 min_x = x * world->tile_side_in_pixels;
	// 		f32 min_y = y * world->tile_side_in_pixels;
	// 		f32 max_x = min_x + world->tile_side_in_pixels;
	// 		f32 max_y = min_y + world->tile_side_in_pixels;

	// 		DrawRectangleWithBorder(&draw_buffer,
	// 								v2(min_x, min_y),
	// 								v2(max_x, max_y),
	// 								1.0f, 0.5f, 0.0f,
	// 								1,
	// 								0.7f, 0.7f, 0.7f,
	// 								true);
	// 	}
	// }
}
