#ifndef PLAYGROUND_PLATFORM_H

#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef i32 b32; 

#define internal static
#define local_persist static
#define global_variable static

#define KILOBYTES(value) ((value) * 1024LL)
#define MEGABYTES(value) ((KILOBYTES(value)) * 1024LL)
#define GIGABYTES(value) ((MEGABYTES(value)) * 1024LL)
#define TERABYTES(value) ((GIGABYTES(value)) * 1024LL)

#define ASSERT(x) if (!(x)) { *(int*)0 = 0; }
#define INVALID_CODE_PATH ASSERT(!"INVALID CODE PATH")
#define INVALID_DEFAULT_CASE default: { INVALID_CODE_PATH; } break
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define SWAP(a, b, t) do { t temp = (a); a = (b); b = (temp); } while (0)
#define MINIMUM(a, b) ((a) < (b) ? (a) : (b))
#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))

struct PlaygroundFile
{
	void* contents;
	u32 size;
};

#define PLATFORM_READ_FILE(name) PlaygroundFile name(char* file_name)
typedef PLATFORM_READ_FILE(PlaygroundReadFileCallback);

#define PLATFORM_FREE_FILE(name) void name(void* file_memory)
typedef PLATFORM_FREE_FILE(PlaygroundFreeFileCallback);

struct PlaygroundMemory
{
	b32 is_initialized;
	
	u32 permanent_storage_size;
	void* permanent_storage;
	
	u32 transient_storage_size;
	void* transient_storage;

	PlaygroundReadFileCallback* PlaygroundReadFile;
	PlaygroundFreeFileCallback* PlaygroundFreeFile;
};

#define BITMAP_BYTES_PER_PIXEL 4
struct PlaygroundDisplayBuffer
{
	void* memory;
	i32 width;
	i32 height;
	i32 pitch;
	i32 size;
};

struct PlaygroundButton
{
	b32 is_down;
	b32 is_released;
};

struct PlaygroundInput
{
	f32 delta_time_for_frame;
	
	i32 mouse_x;
	i32 mouse_y;

	b32 scrolling;
	b32 wheel_moving_forward;
	
	union
	{
		PlaygroundButton buttons[22];

		struct
		{
			PlaygroundButton move_up;
			PlaygroundButton move_left;
			PlaygroundButton move_down;
			PlaygroundButton move_right;

			PlaygroundButton up;
			PlaygroundButton left;
			PlaygroundButton down;
			PlaygroundButton right;

			PlaygroundButton attack;

			PlaygroundButton space;

			PlaygroundButton numpad_0;
			PlaygroundButton numpad_1;
			PlaygroundButton numpad_2;
			PlaygroundButton numpad_3;
			PlaygroundButton numpad_4;
			PlaygroundButton numpad_5;

			PlaygroundButton alt_key;
			
			PlaygroundButton mouse_left;
			PlaygroundButton mouse_middle;
			PlaygroundButton mouse_right;
			PlaygroundButton mouse_extra_1;
			PlaygroundButton mouse_extra_2;

			PlaygroundButton terminator;
		};
	};
};

#define PLAYGROUND_UPDATE_AND_RENDER(name) void name(PlaygroundMemory* memory, PlaygroundDisplayBuffer* display_buffer, PlaygroundInput* input)
typedef PLAYGROUND_UPDATE_AND_RENDER(PlaygroundUpdateAndRenderCallback);

#define PLAYGROUND_PLATFORM_H
#endif
