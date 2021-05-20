#ifndef PLAYGROUND_MEMORY_H

struct PlaygroundMemoryArena
{
	u8* base_pointer;
	u32 size;
	u32 used;
};

internal void InitializeArena(PlaygroundMemoryArena* arena, u32 size, void* base_pointer)
{
	arena->base_pointer = (u8*)base_pointer;
	arena->size = size;
	arena->used = 0;
}

#define PushStruct(arena, type) (type*)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type) (type*)PushSize_(arena, (count) * sizeof(type))

internal void* PushSize_(PlaygroundMemoryArena* arena, u32 size)
{
	ASSERT(arena->used + size <= arena->size);
	void* result = arena->base_pointer + arena->used;
	arena->used += size;

	return result;
}

#define PLAYGROUND_MEMORY_H
#endif
