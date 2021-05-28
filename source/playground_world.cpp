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

inline u32 GetTileMapIndex(World* world, u32 tile_x)
{
	u32 result = tile_x / world->tile_count_x;

	return result;
}

inline TileMap* GetTileMap(World* world, u32 tile_x)
{
	u32 tile_map_index = GetTileMapIndex(world, tile_x);

	ASSERT(tile_map_index < world->tile_map_count_x);

	TileMap* result = world->tile_maps + tile_map_index;

	return result;
}

inline b32 AreOnSameTileMap(World* world, TilePosition* tile_position_a, TilePosition* tile_position_b)
{
	u32 tile_position_a_index = GetTileMapIndex(world, tile_position_a->tile_x);
	u32 tile_position_b_index = GetTileMapIndex(world, tile_position_b->tile_x);

	b32 result = tile_position_a_index == tile_position_b_index;

	return result;
}
