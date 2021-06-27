#ifndef PLAYGROUND_MEMORY_H

struct PlaygroundMemoryArena
{
	u8* base_pointer;
	u32 size;
	u32 used;
	
	u32 begin_end_counter;
};

internal void InitializeArena(PlaygroundMemoryArena* arena, u32 size, void* base_pointer)
{
	arena->base_pointer = (u8*)base_pointer;
	arena->size = size;
	arena->used = 0;
	arena->begin_end_counter = 0;
}

#define PushStruct(arena, type) (type*)PushSize_((arena), sizeof(type))
#define PushArray(arena, count, type) (type*)PushSize_((arena), (count) * sizeof(type))
#define PushSize(arena, size) PushSize_((arena), (size))

internal void* PushSize_(PlaygroundMemoryArena* arena, u32 size)
{
	ASSERT(arena->used + size <= arena->size);
	void* result = arena->base_pointer + arena->used;
	arena->used += size;

	return result;
}

struct TemporaryMemory
{
	PlaygroundMemoryArena* arena;
	u32 used;
};

internal TemporaryMemory BeginTemporaryMemory(PlaygroundMemoryArena* arena)
{
	TemporaryMemory temporary_memory = {};
	temporary_memory.arena = arena;
	temporary_memory.used = arena->used;

	++temporary_memory.arena->begin_end_counter;

	return temporary_memory;
}

internal void EndTemporaryMemory(TemporaryMemory* temporary_memory)
{
	ASSERT(temporary_memory->used < temporary_memory->arena->used);
	temporary_memory->arena->used = temporary_memory->used;
	--temporary_memory->arena->begin_end_counter;
}

internal void CheckArena(PlaygroundMemoryArena* arena)
{
	ASSERT(arena->begin_end_counter == 0);
}

#define PLAYGROUND_MEMORY_H
#endif
