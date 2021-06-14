#ifndef PLAYGROUND_DRAW_H

struct Bitmap
{
	void* memory;
	i32 width;
	i32 height;
	i32 pitch;

	i32 align_x;
	i32 align_y;
};

#define PLAYGROUND_DRAW_H
#endif
