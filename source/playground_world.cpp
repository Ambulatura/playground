inline v2 InvalidPosition()
{
	v2 result = INVALID_POSITION;

	return result;
}

inline TilePosition InvalidTilePosition()
{
	TilePosition result = {};

	result.tile_x = INVALID_TILE_POSITION;

	return result;
}

inline void NormalizeCoordinates(i32* tile_position, f32* position, f32 tile_side_in_meters)
{
	i32 offset = RoundF32ToI32(*position / tile_side_in_meters);
	*tile_position += offset;
	f32 normalized_tile_coordinate = offset * tile_side_in_meters;
	*position -= normalized_tile_coordinate;
}

inline void NormalizePositions(TilePosition* position, f32 tile_side_in_meters)
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

inline TilePosition MapIntoTilePosition(TilePosition tile_position, v2 offset, f32 tile_side_in_meters)
{
	TilePosition result = tile_position;
	result.xy += offset;
	NormalizePositions(&result, tile_side_in_meters);

	return result;
}

inline b32 IsTilePositionInvalid(TilePosition* tile_position)
{
	b32 result = tile_position->tile_x == INVALID_TILE_POSITION;

	return result;
}

inline u32 GetTileMapIndex(World* world, i32 tile_x)
{
	i32 result = tile_x / world->tile_count_x;

	return result;
}

inline TileMap* GetTileMap(World* world, i32 tile_map_index)
{
	// NOTE(SSJSR): We don't need to use hash function(sparse storage)
	// because world map is small(128 tile_map long now, probably will be less?)
	// and fixed size.
	// NOTE(SSJSR): This operation is for mapping negative tile_map_index
	// between 0 - ARRAY_COUNT(world->tile_maps).
	// TODO(SSJSR): This mapping is very likely to be buggy.
	u32 index = (tile_map_index & (ARRAY_COUNT(world->tile_maps) - 1));
	ASSERT(index < ARRAY_COUNT(world->tile_maps));
	TileMap* tile_map = world->tile_maps + index;
	
	return tile_map;
}

internal void InitializeWorld(World* world, u32 display_width, u32 display_height)
{
	world->tile_side_in_pixels = 60.0f;
	world->tile_side_in_meters = 1.0f;
	world->meters_to_pixels = world->tile_side_in_pixels / world->tile_side_in_meters;
	world->tile_count_y = (display_height / (u32)world->tile_side_in_pixels);
	world->tile_count_x = (display_width / (u32)world->tile_side_in_pixels);
	world->tile_map_count_x = 30;
	world->tile_map_count_y = 1;
}

inline b32 AreOnSameTileMap(World* world, TilePosition* tile_position_a, TilePosition* tile_position_b)
{
	u32 tile_position_a_index = GetTileMapIndex(world, tile_position_a->tile_x);
	u32 tile_position_b_index = GetTileMapIndex(world, tile_position_b->tile_x);

	b32 result = tile_position_a_index == tile_position_b_index;

	return result;
}
