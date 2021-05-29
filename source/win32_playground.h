#ifndef WIN32_PLAYGROUND_H

struct Win32WindowSize
{
	i32 width;
	i32 height;
};

struct Win32DisplayBuffer
{
	BITMAPINFO bitmap_info;
	void* memory;
	i32 width;
	i32 height;
	i32 bytes_per_pixel;
	i32 pitch;
	i32 size;
	i32 destination_width;
	i32 destination_height;
};

struct Win32PlaygroundCode
{
	b32 is_valid;
	HMODULE code_dll;
	FILETIME last_write_time_dll;
	PlaygroundUpdateAndRenderCallback* update_and_render;
};

struct Win32Replay
{
	HANDLE state_file_handle;
	HANDLE state_file_map;
	void* map_view;
};

#define MAX_FILE_PATH 256
struct Win32State
{
	void* memory;
	u32 memory_size;

	Win32Replay replay;
	b32 input_record;
	b32 input_playback;
	char* replay_state_file_name;
	char* replay_input_file_name;
	HANDLE record_input_file_handle;
	HANDLE playback_input_file_handle;
	
	char executable_file_path[MAX_FILE_PATH];
	char* executable_file_name;
};

struct Win32MessageLoopInformation
{
	Win32State state;
	PlaygroundInput new_input;
};

#define WIN32_PLAYGROUND_H
#endif
