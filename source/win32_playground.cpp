#include "playground_platform.h"

#include <windows.h>
#include <stdio.h>

#include "win32_playground.h"
#include "win32_timer.h"
#include "win32_timer.cpp"

global_variable b32 global_running;
// global_variable Win32DisplayBuffer global_display_buffer;
// global_variable WINDOWPLACEMENT global_window_position = { sizeof(global_window_position) };

PLATFORM_FREE_FILE(Win32FreeFile)
{
	if (file_memory) {
		VirtualFree(file_memory, 0, MEM_RELEASE);
		file_memory = 0;
	}
}

PLATFORM_READ_FILE(Win32ReadFile)
{
	PlaygroundFile result = {};

	HANDLE file_handle = CreateFileA(file_name,
									 GENERIC_READ,
									 0,
									 0,
									 OPEN_EXISTING,
									 FILE_ATTRIBUTE_NORMAL,
									 0);

	if (file_handle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER file_size;
		if (GetFileSizeEx(file_handle, &file_size)) {
			ASSERT(file_size.QuadPart < 0xFFFFFFFF);
			u32 file_size_u32 = (u32)file_size.QuadPart;
			result.contents = VirtualAlloc(0, file_size_u32,
										   MEM_COMMIT | MEM_RESERVE,
										   PAGE_READWRITE);
			if (result.contents) {
				DWORD bytes_read;
				if (ReadFile(file_handle, result.contents, file_size_u32, &bytes_read, 0) &&
					file_size_u32 == bytes_read) {
					result.size = file_size_u32;
				}
				else {
					Win32FreeFile(result.contents);
				}
			}
			else {
				// TODO(SSJSR): Logging.
			}
		}
		else {
			// TODO(SSJSR): Logging.
		}

		CloseHandle(file_handle);
	}
	else {
		// TODO(SSJSR): Logging.
	}

	return result;
}

internal void Win32ToggleFullscreen(HWND window_handle, WINDOWPLACEMENT* window_position)
{
	// NOTE(SSJSR): This follows Raymond Chen's prescription
	// for fullscreen toggling, see:
	// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353

	DWORD style = GetWindowLong(window_handle, GWL_STYLE);
	if (style & WS_OVERLAPPEDWINDOW) {
		MONITORINFO monitor_info = { sizeof(monitor_info) };
		if (GetWindowPlacement(window_handle, window_position) &&
			GetMonitorInfo(MonitorFromWindow(window_handle,
											 MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
			SetWindowLong(window_handle, GWL_STYLE,
						  style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(window_handle, HWND_TOP,
						 monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
						 monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
						 monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else {
		SetWindowLong(window_handle, GWL_STYLE,
					  style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window_handle, window_position);
		SetWindowPos(window_handle, NULL, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

internal Win32WindowSize Win32GetWindowSize(HWND window_handle)
{
	Win32WindowSize result;

	RECT window_size;
	GetClientRect(window_handle, &window_size);

	result.width = window_size.right;
	result.height = window_size.bottom;

	return result;
}

internal void Win32ResizeDisplayBuffer(Win32DisplayBuffer* display_buffer, i32 width, i32 height)
{
	if (display_buffer->memory) {
		VirtualFree(display_buffer->memory, 0, MEM_RELEASE);
	}

	display_buffer->width = width;
	display_buffer->height = height;
	display_buffer->bytes_per_pixel = 4;
	display_buffer->pitch = display_buffer->width * display_buffer->bytes_per_pixel;
	display_buffer->size = display_buffer->width * display_buffer->height * display_buffer->bytes_per_pixel;
	display_buffer->destination_width = display_buffer->width;
	display_buffer->destination_height = display_buffer->height;

	display_buffer->bitmap_info.bmiHeader.biSize = sizeof(display_buffer->bitmap_info.bmiHeader);
	display_buffer->bitmap_info.bmiHeader.biWidth = display_buffer->width;
	display_buffer->bitmap_info.bmiHeader.biHeight = -display_buffer->height;
	display_buffer->bitmap_info.bmiHeader.biPlanes = 1;
	display_buffer->bitmap_info.bmiHeader.biBitCount = 32;
	display_buffer->bitmap_info.bmiHeader.biCompression = BI_RGB;

	display_buffer->memory = VirtualAlloc(0, display_buffer->size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32ShowDisplayBufferInWindow(HDC device_context, Win32DisplayBuffer* display_buffer, i32 window_width, i32 window_height)
{
	if (window_width >= display_buffer->width * 2 &&
		window_height >= display_buffer->height * 2) {

		display_buffer->destination_width = display_buffer->width * 2;
		display_buffer->destination_height = display_buffer->height * 2;

		StretchDIBits(device_context,
					  0, 0, display_buffer->destination_width, display_buffer->destination_height,
					  0, 0, display_buffer->width, display_buffer->height,
					  display_buffer->memory,
					  &display_buffer->bitmap_info,
					  DIB_RGB_COLORS,
					  SRCCOPY);
	}
	else {
		display_buffer->destination_width = display_buffer->width;
		display_buffer->destination_height = display_buffer->height;

		if (display_buffer->destination_width < window_width) {
			PatBlt(device_context,
				   display_buffer->width, 0, window_width, window_height,
				   BLACKNESS);
		}

		if (display_buffer->destination_height < window_height) {
			PatBlt(device_context,
				   0,  display_buffer->height, window_width, window_height,
				   BLACKNESS);
		}

		StretchDIBits(device_context,
					  0, 0, window_width, window_height,// display_buffer->destination_width, display_buffer->destination_height,
					  0, 0, display_buffer->width, display_buffer->height,
					  display_buffer->memory,
					  &display_buffer->bitmap_info,
					  DIB_RGB_COLORS,
					  SRCCOPY);
	}
}

internal void Win32ProcessKeyboardInput(PlaygroundButton* button, b32 is_down)
{
	// if (button->is_down != is_down) {
	button->is_released = (button->is_down == 1 && is_down == 0);
	button->is_down = is_down;
	// }
}

internal void Win32BeginInputRecord(Win32State* state)
{
	state->record_input_file_handle =
		CreateFileA(state->replay_input_file_name,
					GENERIC_WRITE,
					0,
					0,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					0);

	CopyMemory(state->replay.map_view,
			   state->memory,
			   state->memory_size);
}

internal void Win32EndInputRecord(Win32State* state)
{
	CloseHandle(state->record_input_file_handle);
}

internal void Win32InputRecord(Win32State* state, PlaygroundInput* new_playground_input)
{
	DWORD bytes_written;
	WriteFile(state->record_input_file_handle,
			  new_playground_input,
			  sizeof(*new_playground_input),
			  &bytes_written,
			  0);
}

internal void Win32BeginInputPlayback(Win32State* state)
{
	state->playback_input_file_handle =
		CreateFileA(state->replay_input_file_name,
					GENERIC_READ,
					0,
					0,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					0);

	CopyMemory(state->memory,
			   state->replay.map_view,
			   state->memory_size);
}

internal void Win32EndInputPlayback(Win32State* state)
{
	CloseHandle(state->playback_input_file_handle);
}

internal void Win32InputPlayback(Win32State* state, PlaygroundInput* new_playground_input)
{
	DWORD bytes_read;
	if (ReadFile(state->playback_input_file_handle,
				 new_playground_input,
				 sizeof(*new_playground_input),
				 &bytes_read,
				 0)) {
		if (bytes_read == 0) {
			Win32EndInputPlayback(state);
			Win32BeginInputPlayback(state);
			ReadFile(state->playback_input_file_handle,
					 new_playground_input,
					 sizeof(*new_playground_input),
					 &bytes_read,
					 0);
		}
	}
}

// internal void Win32ProcessMessages(Win32State* state, PlaygroundInput* input)
// {
// 	MSG message;
// 	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
// 		switch (message.message) {
// 			case WM_SYSKEYDOWN:
// 			case WM_SYSKEYUP:
// 			case WM_KEYDOWN:
// 			case WM_KEYUP: {
// 				u32 key_code = (u32)message.wParam;
// 				b32 was_down = (message.lParam & (1 << 30));
// 				b32 is_down = !(message.lParam & (1 << 31));
// 				b32 alt_is_down = (message.lParam & (1 << 29));

// 				Win32ProcessKeyboardInput(&input->alt_key, alt_is_down);

// 				if (key_code == VK_RETURN) {
// 					if (is_down && alt_is_down) {
// 						Win32ToggleFullscreen(message.hwnd);
// 					}
// 				}
// 				else if (key_code == 'W') {
// 					Win32ProcessKeyboardInput(&input->move_up, is_down);
// 				}
// 				else if (key_code == 'A') {
// 					Win32ProcessKeyboardInput(&input->move_left, is_down);
// 				}
// 				else if (key_code == 'S') {
// 					Win32ProcessKeyboardInput(&input->move_down, is_down);
// 				}
// 				else if (key_code == 'D') {
// 					Win32ProcessKeyboardInput(&input->move_right, is_down);
// 				}
// 				else if (key_code == VK_UP) {
// 					Win32ProcessKeyboardInput(&input->up, is_down);
// 				}
// 				else if (key_code == VK_LEFT) {
// 					Win32ProcessKeyboardInput(&input->left, is_down);
// 				}
// 				else if (key_code == VK_DOWN) {
// 					Win32ProcessKeyboardInput(&input->down, is_down);
// 				}
// 				else if (key_code == VK_RIGHT) {
// 					Win32ProcessKeyboardInput(&input->right, is_down);
// 				}
// 				else if (key_code == VK_NUMPAD0) {
// 					Win32ProcessKeyboardInput(&input->numpad_0, is_down);
// 				}
// 				else if (key_code == VK_NUMPAD1) {
// 					Win32ProcessKeyboardInput(&input->numpad_1, is_down);
// 				}
// 				else if (key_code == VK_NUMPAD2) {
// 					Win32ProcessKeyboardInput(&input->numpad_2, is_down);
// 				}
// 				else if (key_code == VK_NUMPAD3) {
// 					Win32ProcessKeyboardInput(&input->numpad_3, is_down);
// 				}
// 				else if (key_code == VK_NUMPAD4) {
// 					Win32ProcessKeyboardInput(&input->numpad_4, is_down);
// 				}
// 				else if (key_code == VK_NUMPAD5) {
// 					Win32ProcessKeyboardInput(&input->numpad_5, is_down);
// 				}
// 				else if (key_code == VK_SPACE) {
// 					Win32ProcessKeyboardInput(&input->space, is_down);
// 				}
// 				else if (key_code == 'L') {
// 					if (is_down) {
// 						if (state->input_record && !state->input_playback) {
// 							Win32EndInputRecord(state);
// 							Win32BeginInputPlayback(state);
// 							state->input_record = false;
// 							state->input_playback = true;
// 						}
// 						else if (state->input_playback) {
// 							Win32EndInputPlayback(state);
// 							state->input_playback = false;
// 						}
// 						else if (!state->input_record) {
// 							Win32BeginInputRecord(state);
// 							state->input_record = true;
// 						}
// 					}
// 				}
// 			} break;
// 			case WM_LBUTTONDOWN: {
// 				Win32ProcessKeyboardInput(&input->mouse_left, true);
// 			} break;
// 			case WM_LBUTTONUP: {
// 				Win32ProcessKeyboardInput(&input->mouse_left, false);
// 			} break;
// 			case WM_RBUTTONDOWN: {
// 				Win32ProcessKeyboardInput(&input->mouse_right, true);
// 			} break;
// 			case WM_RBUTTONUP: {
// 				Win32ProcessKeyboardInput(&input->mouse_right, false);
// 			} break;
// 			case WM_MBUTTONDOWN: {
// 				Win32ProcessKeyboardInput(&input->mouse_middle, true);
// 			} break;
// 			case WM_MBUTTONUP: {
// 				Win32ProcessKeyboardInput(&input->mouse_middle, false);
// 			} break;
// 			case WM_MOUSEWHEEL: {
// 				input->scrolling = true;
// 				i16 wheel_delta = (i16)(message.wParam >> 16);
// 				if (wheel_delta > 0) {
// 					input->wheel_moving_forward = true;
// 				}
// 				else {
// 					input->wheel_moving_forward = false;
// 				}
// 			} break;
// 			default: {
// 				TranslateMessage(&message);
// 				DispatchMessage(&message);
// 			} break;
// 		}
// 	}
// }

internal LRESULT CALLBACK Win32WindowProc(HWND window_handle,
										  UINT message,
										  WPARAM w_param,
										  LPARAM l_param)
{
	LRESULT result = 0;

	switch (message) {
		case WM_CREATE: {
			Win32MessageLoopInformation* information =
				(Win32MessageLoopInformation*)((CREATESTRUCTA*)l_param)->lpCreateParams;
			SetWindowLongPtr(window_handle, GWLP_USERDATA, (LONG_PTR)information);

			SetWindowPos(window_handle, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		} break;

		case WM_PAINT: {
			PAINTSTRUCT paint_struct;
			HDC device_context = BeginPaint(window_handle, &paint_struct);

			Win32WindowSize window_size = Win32GetWindowSize(window_handle);
			Win32MessageLoopInformation* information =
				(Win32MessageLoopInformation*)GetWindowLongPtr(window_handle, GWLP_USERDATA);
			Win32ShowDisplayBufferInWindow(device_context, &information->display_buffer,
										   window_size.width, window_size.height);

			EndPaint(window_handle, &paint_struct);
		} break;

		case WM_SYSCHAR: {
			// NOTE(SSJSR): This WM_SYSCHAR message is handled to prevent
			// *beep* sound that Windows produces when ALT key is pressed.
		} break;

		case WM_DESTROY:
		case WM_CLOSE:
		case WM_QUIT: {
			global_running = false;
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP: {
			u32 key_code = (u32)w_param;
			b32 was_down = (l_param & (1 << 30));
			b32 is_down = !(l_param & (1 << 31));
			b32 alt_is_down = (l_param & (1 << 29));

			Win32MessageLoopInformation* information =
				(Win32MessageLoopInformation*)GetWindowLongPtr(window_handle, GWLP_USERDATA);

			PlaygroundInput* input = information->new_playground_input;

			Win32ProcessKeyboardInput(&input->alt_key, alt_is_down);

			if (key_code == VK_RETURN) {
				if (is_down && alt_is_down) {
					Win32ToggleFullscreen(window_handle, &information->window_position);
				}
			}
			else if (key_code == 'W') {
				Win32ProcessKeyboardInput(&input->move_up, is_down);
			}
			else if (key_code == 'A') {
				Win32ProcessKeyboardInput(&input->move_left, is_down);
			}
			else if (key_code == 'S') {
				Win32ProcessKeyboardInput(&input->move_down, is_down);
			}
			else if (key_code == 'D') {
				Win32ProcessKeyboardInput(&input->move_right, is_down);
			}
			else if (key_code == VK_UP) {
				Win32ProcessKeyboardInput(&input->up, is_down);
			}
			else if (key_code == VK_LEFT) {
				Win32ProcessKeyboardInput(&input->left, is_down);
			}
			else if (key_code == VK_DOWN) {
				Win32ProcessKeyboardInput(&input->down, is_down);
			}
			else if (key_code == VK_RIGHT) {
				Win32ProcessKeyboardInput(&input->right, is_down);
			}
			else if (key_code == VK_NUMPAD0) {
				Win32ProcessKeyboardInput(&input->numpad_0, is_down);
			}
			else if (key_code == VK_NUMPAD1) {
				Win32ProcessKeyboardInput(&input->numpad_1, is_down);
			}
			else if (key_code == VK_NUMPAD2) {
				Win32ProcessKeyboardInput(&input->numpad_2, is_down);
			}
			else if (key_code == VK_NUMPAD3) {
				Win32ProcessKeyboardInput(&input->numpad_3, is_down);
			}
			else if (key_code == VK_NUMPAD4) {
				Win32ProcessKeyboardInput(&input->numpad_4, is_down);
			}
			else if (key_code == VK_NUMPAD5) {
				Win32ProcessKeyboardInput(&input->numpad_5, is_down);
			}
			else if (key_code == VK_SPACE) {
				Win32ProcessKeyboardInput(&input->space, is_down);
			}
			else if (key_code == 'L') {
				Win32State* state = &information->state;
				if (is_down) {
					if (state->input_record && !state->input_playback) {
						Win32EndInputRecord(state);
						Win32BeginInputPlayback(state);
						state->input_record = false;
						state->input_playback = true;
					}
					else if (state->input_playback) {
						Win32EndInputPlayback(state);
						state->input_playback = false;
					}
					else if (!state->input_record) {
						Win32BeginInputRecord(state);
						state->input_record = true;
					}
				}
			}
		} break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL: {
			Win32MessageLoopInformation* information =
				(Win32MessageLoopInformation*)GetWindowLongPtr(window_handle, GWLP_USERDATA);
			PlaygroundInput* input = information->new_playground_input;

			if (message == WM_LBUTTONDOWN) {
				Win32ProcessKeyboardInput(&input->mouse_left, true);
			}
			else if (message == WM_LBUTTONUP) {
				Win32ProcessKeyboardInput(&input->mouse_left, false);
			}
			else if (message == WM_RBUTTONDOWN) {
				Win32ProcessKeyboardInput(&input->mouse_right, true);
			}
			else if (message == WM_RBUTTONUP) {
				Win32ProcessKeyboardInput(&input->mouse_right, false);
			}
			else if (message == WM_MBUTTONDOWN) {
				Win32ProcessKeyboardInput(&input->mouse_middle, true);
			}
			else if (message == WM_MBUTTONUP) {
				Win32ProcessKeyboardInput(&input->mouse_middle, false);
			}
			else if (message == WM_MOUSEWHEEL) {
				input->scrolling = true;
				i16 wheel_delta = (i16)(w_param >> 16);
				if (wheel_delta > 0) {
					input->wheel_moving_forward = true;
				}
				else {
					input->wheel_moving_forward = false;
				}
			}

		} break;

		default: {
			result = DefWindowProcA(window_handle, message, w_param, l_param);
		} break;
	};

	return result;
}

internal FILETIME Win32GetLastWriteTime(char* file_name)
{
	FILETIME last_write_time = {};

	WIN32_FILE_ATTRIBUTE_DATA file_data;
	if (GetFileAttributesExA(file_name, GetFileExInfoStandard, &file_data)) {
		last_write_time = file_data.ftLastWriteTime;
	}

	return last_write_time;
}

internal Win32PlaygroundCode Win32LoadPlaygroundCode(char* dll_file_name, char* temp_dll_file_name, char* lock_file_name)
{
	Win32PlaygroundCode result = {};

	WIN32_FILE_ATTRIBUTE_DATA ignored_data;
	if (!GetFileAttributesExA(lock_file_name, GetFileExInfoStandard, &ignored_data)) {

		result.last_write_time_dll = Win32GetLastWriteTime(dll_file_name);

		CopyFile(dll_file_name, temp_dll_file_name, FALSE);

		result.code_dll = LoadLibraryA(temp_dll_file_name);
		if (result.code_dll) {
			result.update_and_render = (PlaygroundUpdateAndRenderCallback*)GetProcAddress(result.code_dll, "PlaygroundUpdateAndRender");

			if (result.update_and_render) {
				result.is_valid = true;
			}
		}
	}

	return result;
}

internal void Win32UnloadPlaygroundCode(Win32PlaygroundCode* playground_code)
{
	if (playground_code->code_dll) {
		FreeLibrary(playground_code->code_dll);
		playground_code->code_dll = 0;
	}

	playground_code->is_valid = false;
	playground_code->update_and_render = 0;
}

internal void Win32FindExecutableFilePath(Win32State* state)
{
	DWORD size_of_executable_file_path = GetModuleFileNameA(0, state->executable_file_path, sizeof(state->executable_file_path));

	ASSERT(size_of_executable_file_path);
	if (size_of_executable_file_path) {
		state->executable_file_name = state->executable_file_path;
		for (char* scan = state->executable_file_path; *scan; ++scan) {
			if (*scan == '\\') {
				state->executable_file_name = scan + 1;
			}
		}
	}
}

internal void Win32ConcatenateStrings(char* string_a, u32 string_a_length, char* string_b, u32 string_b_length, char* destination, u32 destination_length)
{
	ASSERT(string_a_length + string_b_length < destination_length);

	for (u32 index = 0; index < string_a_length; ++index) {
		*destination++ = *string_a++;
	}

	for (u32 index = 0; index < string_b_length; ++index) {
		*destination++ = *string_b++;
	}

	*destination = '\0';
}

internal u32 Win32StringLength(char* string)
{
	u32 length = 0;
	while (*string++) {
		++length;
	}

	return length;
}

internal void Win32BuildFilenameOnExecutablePath(Win32State* state, char* file_name, char* destination, u32 destination_length)
{
	Win32ConcatenateStrings(state->executable_file_path, (u32)(state->executable_file_name - state->executable_file_path),
							file_name, Win32StringLength(file_name),
							destination, destination_length);
}

int WINAPI WinMain(HINSTANCE instance,
				   HINSTANCE prev_instance,
				   LPSTR     lp_cmd_line,
				   int       n_cmd_show)
{
	Win32Timer timer;
	Win32TimerInitialize(&timer);

	Win32MessageLoopInformation information = {};
	information.window_position = { sizeof(information.window_position) };
	
	Win32ResizeDisplayBuffer(&information.display_buffer, 1920, 1080);

	Win32State* state = &information.state;

	Win32FindExecutableFilePath(state);

	char dll_file_destination[MAX_FILE_PATH];
	Win32BuildFilenameOnExecutablePath(state, "playground.dll", dll_file_destination, sizeof(dll_file_destination));
	char temp_dll_file_destination[MAX_FILE_PATH];
	Win32BuildFilenameOnExecutablePath(state, "playground_temp.dll", temp_dll_file_destination, sizeof(temp_dll_file_destination));
	char lock_file_destination[MAX_FILE_PATH];
	Win32BuildFilenameOnExecutablePath(state, "lock.tmp", lock_file_destination, sizeof(lock_file_destination));

	char replay_state_file_destination[MAX_FILE_PATH];
	Win32BuildFilenameOnExecutablePath(state, "replay_state.bin", replay_state_file_destination, sizeof(replay_state_file_destination));
	char replay_input_file_destination[MAX_FILE_PATH];
	Win32BuildFilenameOnExecutablePath(state, "replay_input.bin", replay_input_file_destination, sizeof(replay_input_file_destination));

	state->replay_state_file_name = (char*)replay_state_file_destination;
	state->replay_input_file_name = (char*)replay_input_file_destination;

	WNDCLASSA window_class = {};

	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = Win32WindowProc;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursor(0, IDC_CROSS);
	window_class.lpszClassName = "PlaygroundWindowClass";

	if (!RegisterClassA(&window_class)) {
		// TODO(SSJSR): Logging and assert?
		return 0;
	}

	HWND window_handle =
		CreateWindowExA(0, // WS_EX_TOPMOST | WS_EX_LAYERED,
						window_class.lpszClassName,
						"Playground",
						WS_OVERLAPPEDWINDOW | WS_VISIBLE,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						0,
						0,
						instance,
						&information);

	if (!window_handle) {
		// TODO(SSJSR): Logging and assert?
		return 0;
	}

	i32 monitor_refresh_hz = 60;
	HDC refresh_device_context = GetDC(window_handle);
	i32 refresh_rate = GetDeviceCaps(refresh_device_context, VREFRESH);
	ReleaseDC(window_handle, refresh_device_context);
	if (refresh_rate > 1) {
		monitor_refresh_hz = refresh_rate;
	}
	f32 game_update_hz = (f32)monitor_refresh_hz; // / 2.0f
	f32 target_seconds_per_frame = 1.0f / game_update_hz;

	PlaygroundMemory playground_memory = {};

	playground_memory.PlaygroundReadFile = Win32ReadFile;
	playground_memory.PlaygroundFreeFile = Win32FreeFile;

	playground_memory.permanent_storage_size = MEGABYTES(256);
	playground_memory.transient_storage_size = GIGABYTES(1);

	state->memory_size = playground_memory.permanent_storage_size + playground_memory.transient_storage_size;

	// TODO(SSJSR): Maybe we will need base address for hot loading.
	// void* storage = VirtualAlloc(0, (SIZE_T)playground_memory.permanent_storage_size + playground_memory.transient_storage_size,
	// 							 MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	state->memory = VirtualAlloc(0, (SIZE_T)state->memory_size,
								 MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	playground_memory.permanent_storage = state->memory;
	playground_memory.transient_storage = (u8*)playground_memory.permanent_storage + playground_memory.permanent_storage_size;

	state->replay.state_file_handle = CreateFileA(state->replay_state_file_name,
												 GENERIC_READ | GENERIC_WRITE,
												 0,
												 0,
												 CREATE_ALWAYS,
												 FILE_ATTRIBUTE_NORMAL,
												 0);

	LARGE_INTEGER total_size;
	total_size.QuadPart = state->memory_size;
	state->replay.state_file_map = CreateFileMappingA(state->replay.state_file_handle,
													 0,
													 PAGE_READWRITE,
													 total_size.HighPart,
													 total_size.LowPart,
													 0);

	state->replay.map_view =  MapViewOfFile(state->replay.state_file_map,
										   FILE_MAP_WRITE | FILE_MAP_READ,
										   0,
										   0,
										   state->memory_size);

	Win32PlaygroundCode playground_code =
		Win32LoadPlaygroundCode(dll_file_destination,
								temp_dll_file_destination,
								lock_file_destination);

	PlaygroundInput playground_input[2] = {};

	information.old_playground_input = &playground_input[0];
	information.new_playground_input = &playground_input[1];

	f32 delta_time_for_frame = 0.0f;
	global_running = true;
	while (global_running) {
		Win32TimerBeginFrame(&timer);

		*information.new_playground_input = {};

		information.new_playground_input->delta_time_for_frame = delta_time_for_frame;

		for (u32 button_index = 0; button_index < ARRAY_COUNT(information.new_playground_input->buttons); ++button_index) {
			information.new_playground_input->buttons[button_index] = information.old_playground_input->buttons[button_index];
			information.new_playground_input->buttons[button_index].is_released = 0;
		}

		{
			MSG message;
			while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE)) {

				TranslateMessage(&message);
				DispatchMessageA(&message);
			}
		}

		POINT mouse_position;
		GetCursorPos(&mouse_position);

		RECT client_rect;
		GetClientRect(window_handle, &client_rect);
		POINT client_rect_left_top = { client_rect.left, client_rect.top };
		POINT client_rect_bottom_right = { client_rect.bottom, client_rect.right };
		ClientToScreen(window_handle, &client_rect_left_top);
		ClientToScreen(window_handle, &client_rect_bottom_right);

		information.new_playground_input->mouse_x = mouse_position.x - client_rect_left_top.x;
		information.new_playground_input->mouse_y = mouse_position.y - client_rect_left_top.y;

		information.new_playground_input->mouse_x = information.display_buffer.width * information.new_playground_input->mouse_x / information.display_buffer.destination_width;
		information.new_playground_input->mouse_y = information.display_buffer.height * information.new_playground_input->mouse_y / information.display_buffer.destination_height;

		// char mouse_buffer[256];
		// _snprintf_s(mouse_buffer, sizeof(mouse_buffer), "(%d, %d)\n",
		// 			information.new_playground_input->mouse_x,
		// 			information.new_playground_input->mouse_y);

		// OutputDebugStringA(mouse_buffer);

		FILETIME new_dll_write_time = Win32GetLastWriteTime(dll_file_destination);
		if (CompareFileTime(&new_dll_write_time, &playground_code.last_write_time_dll) != 0) {
			Win32UnloadPlaygroundCode(&playground_code);
			playground_code = Win32LoadPlaygroundCode(dll_file_destination,
													  temp_dll_file_destination,
													  lock_file_destination);
		}

		// Win32ProcessMessages(state, new_playground_input);

		PlaygroundDisplayBuffer playground_display_buffer = {};
		playground_display_buffer.memory = information.display_buffer.memory;
		playground_display_buffer.width = information.display_buffer.width;
		playground_display_buffer.height = information.display_buffer.height;
		playground_display_buffer.pitch = information.display_buffer.pitch;
		playground_display_buffer.size = information.display_buffer.size;

		if (state->input_record) {
			Win32InputRecord(state, information.new_playground_input);
		}

		if (state->input_playback) {
			Win32InputPlayback(state, information.new_playground_input);
		}

		if (playground_code.is_valid) {
			playground_code.update_and_render(&playground_memory, &playground_display_buffer, information.new_playground_input);
		}

		HDC device_context = GetDC(window_handle);
		Win32WindowSize window_size = Win32GetWindowSize(window_handle);
		Win32ShowDisplayBufferInWindow(device_context, &information.display_buffer,
									   window_size.width, window_size.height);
		ReleaseDC(window_handle, device_context);

		SWAP(information.old_playground_input, information.new_playground_input, PlaygroundInput*);

		Win32TimerEndFrame(&timer, target_seconds_per_frame);

		LARGE_INTEGER flip_counts = Win32GetCounts();
		delta_time_for_frame = Win32GetSecondsElapsed(timer.begin_counts, flip_counts, timer.frequency);

		f32 milliseconds_per_frame = 1000.0f * delta_time_for_frame;
		// char fps_buffer[256];
		// _snprintf_s(fps_buffer, sizeof(fps_buffer), "%.02f milliseconds/frame\n", milliseconds_per_frame);

		// OutputDebugStringA(fps_buffer);
	}

	return 0;
}
