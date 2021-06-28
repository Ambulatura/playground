#ifndef PLAYGROUND_RENDER_H

struct Bitmap
{
	void* memory;
	i32 width;
	i32 height;
	i32 pitch;

	i32 align_x;
	i32 align_y;
};

#pragma pack(push, 1)
struct BmpHeader
{
	u16  file_type;             /* File type, always 4D42h ("BM") */
	u32  file_size;             /* Size of the file in bytes */
	u16  reserved1;             /* Always 0 */
	u16  reserved2;             /* Always 0 */
	u32  bitmap_offset;         /* Starting position of image data in bytes */

	u32 size;                   /* Size of this header in bytes */
	i32 width;                  /* Image width in pixels */
	i32 height;                 /* Image height in pixels */
	u16 planes;                 /* Number of color planes */
	u16 bits_per_pixel;         /* Number of bits per pixel */
	u32 compression;            /* Compression methods used */
	u32 size_of_bitmap;         /* Size of bitmap in bytes */
	i32 horizontal_resolution;  /* Horizontal resolution in pixels per meter */
	i32 vertical_resolution;    /* Vertical resolution in pixels per meter */
	u32 colors_used;            /* Number of colors in the image */
	u32 colors_important;       /* Minimum number of important colors */
	
	u32 red_mask;               /* Mask identifying bits of red component */
	u32 green_mask;             /* Mask identifying bits of green component */
	u32 blue_mask;              /* Mask identifying bits of blue component */
	u32 alpha_mask;             /* Mask identifying bits of alpha component */
};
#pragma pack(pop)


enum RenderGroupElementType
{
	RENDER_GROUP_ELEMENT_TYPE_RenderGroupElementBitmap,
	RENDER_GROUP_ELEMENT_TYPE_RenderGroupElementRectangle,
	RENDER_GROUP_ELEMENT_TYPE_RenderGroupElementClear,
};

struct RenderElementSpec
{
	v2* position;
	v2 offset;
};

struct RenderGroupElementHeader
{
	RenderGroupElementType type;
};

struct RenderGroupElementRectangle
{
	RenderElementSpec spec;
	v2 dimension;
	v4 color;
};

struct RenderGroupElementBitmap
{
	RenderElementSpec spec;
	Bitmap* bitmap;
	v4 color;
	b32 flip_horizontally;
};

struct RenderGroupElementClear
{
	v4 color;
};

struct RenderGroup
{
	v2* position;
	f32 meters_to_pixels;

	u32 max_buffer_size;
	u32 buffer_size;
	u8* base_buffer;
};

#define PLAYGROUND_RENDER_H
#endif
