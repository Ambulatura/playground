internal void DrawRectangle(PlaygroundDisplayBuffer* display_buffer,
							f32 min_x, f32 min_y, f32 max_x, f32 max_y,
							f32 r, f32 g, f32 b)
{
	i32 rounded_min_x = RoundF32ToI32(min_x);
	i32 rounded_min_y = RoundF32ToI32(min_y);
	i32 rounded_max_x = RoundF32ToI32(max_x);
	i32 rounded_max_y = RoundF32ToI32(max_y);

	if (rounded_min_x < 0) {
		rounded_min_x = 0;
	}

	if (rounded_min_y < 0) {
		rounded_min_y = 0;
	}

	if (rounded_max_x > display_buffer->width) {
		rounded_max_x = display_buffer->width;
	}

	if (rounded_max_y > display_buffer->height) {
		rounded_max_y = display_buffer->height;
	}

	u32 color = ((RoundF32ToU32(r * 255.0f) << 16) |
				 (RoundF32ToU32(g * 255.0f) << 8) |
				 (RoundF32ToU32(b * 255.0f) << 0));

	u8* pixels = ((u8*)display_buffer->memory +
				  rounded_min_x * display_buffer->bytes_per_pixel +
				  rounded_min_y * display_buffer->pitch);

	for (i32 y = rounded_min_y; y < rounded_max_y; ++y) {
		u32* row = (u32*)pixels;
		for (i32 x = rounded_min_x; x < rounded_max_x; ++x) {
			*row++ = color;
		}
		pixels += display_buffer->pitch;
	}
}

internal void DrawRectangleWithBorder(PlaygroundDisplayBuffer* display_buffer,
									  f32 min_x, f32 min_y, f32 max_x, f32 max_y,
									  f32 r, f32 g, f32 b,
									  u8 border_width,
									  f32 border_r, f32 border_g, f32 border_b,
									  b32 only_border=false)
{
	i32 rounded_min_x = RoundF32ToI32(min_x);
	i32 rounded_min_y = RoundF32ToI32(min_y);
	i32 rounded_max_x = RoundF32ToI32(max_x);
	i32 rounded_max_y = RoundF32ToI32(max_y);

	if (rounded_min_x < 0) {
		rounded_min_x = 0;
	}

	if (rounded_min_y < 0) {
		rounded_min_y = 0;
	}

	if (rounded_max_x > display_buffer->width) {
		rounded_max_x = display_buffer->width;
	}

	if (rounded_max_y > display_buffer->height) {
		rounded_max_y = display_buffer->height;
	}

	u32 color = ((RoundF32ToU32(r * 255.0f) << 16) |
				 (RoundF32ToU32(g * 255.0f) << 8) |
				 (RoundF32ToU32(b * 255.0f) << 0));

	u32 border_color = ((RoundF32ToU32(border_r * 255.0f) << 16) |
						(RoundF32ToU32(border_g * 255.0f) << 8) |
						(RoundF32ToU32(border_b * 255.0f) << 0));

	u8* pixels = ((u8*)display_buffer->memory +
				  rounded_min_x * display_buffer->bytes_per_pixel +
				  rounded_min_y * display_buffer->pitch);

	for (i32 y = rounded_min_y; y < rounded_max_y; ++y) {
		u32* row = (u32*)pixels;
		for (i32 x = rounded_min_x; x < rounded_max_x; ++x) {
			if (y < rounded_min_y + border_width ||
				y >= rounded_max_y - border_width ||
				x < rounded_min_x + border_width ||
				x >= rounded_max_x - border_width) {
				*row++ = border_color;
			}
			else {
				if (!only_border) {
					*row++ = color;
				}
				else {
					++row;
				}
			}
		}
		pixels += display_buffer->pitch;
	}
}

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

internal LoadedBmp LoadBmp(char* file_name, PlaygroundReadFileCallback* PlaygroundReadFile, u32 bitmap_align_x=0, u32 bitmap_align_y=0)
{
	LoadedBmp loaded_bmp = {};
	loaded_bmp.align_x = bitmap_align_x;
	loaded_bmp.align_y = bitmap_align_y;
	
	PlaygroundFile bmp_file = PlaygroundReadFile(file_name);
	if (bmp_file.size != 0) {
		BmpHeader* bmp_header = (BmpHeader*)bmp_file.contents;

		loaded_bmp.pixels = (u32*)((u8*)bmp_header + bmp_header->bitmap_offset);
		loaded_bmp.width = bmp_header->width;
		loaded_bmp.height = bmp_header->height;

		ASSERT(bmp_header->compression == 3);

		LeastSignificantSetBitScan red_scan = FindLeastSignificantSetBit(bmp_header->red_mask);
		LeastSignificantSetBitScan green_scan = FindLeastSignificantSetBit(bmp_header->green_mask);
		LeastSignificantSetBitScan blue_scan = FindLeastSignificantSetBit(bmp_header->blue_mask);
		LeastSignificantSetBitScan alpha_scan = FindLeastSignificantSetBit(bmp_header->alpha_mask);

		ASSERT(red_scan.found);
		ASSERT(green_scan.found);
		ASSERT(blue_scan.found);
		ASSERT(alpha_scan.found);

		u32 red_rotate = 16 - red_scan.index;
		u32 green_rotate = 8 - green_scan.index;
		u32 blue_rotate = 0 - blue_scan.index;
		u32 alpha_rotate = 24 - alpha_scan.index;

		u32* pixels = loaded_bmp.pixels;
		for (i32 y = 0; y < bmp_header->height; ++y) {
			for (i32 x = 0; x < bmp_header->width; ++x) {
				u32 color = *pixels;
				*pixels++ = (BitsRotateLeft(color & bmp_header->red_mask, red_rotate) |
							 BitsRotateLeft(color & bmp_header->green_mask, green_rotate) |
							 BitsRotateLeft(color & bmp_header->blue_mask, blue_rotate) |
							 BitsRotateLeft(color & bmp_header->alpha_mask, alpha_rotate));
			}
		}
	}
	
	return loaded_bmp;
}

internal void DrawBitmap(PlaygroundDisplayBuffer* display_buffer,
						 LoadedBmp* bitmap,
						 f32 x, f32 y,
						 u32 align_x=0, u32 align_y=0,
						 b32 flip_horizontally=false)
{
	x -= align_x;
	y -= align_y;
	
	i32 rounded_min_x = RoundF32ToI32(x);
	i32 rounded_min_y = RoundF32ToI32(y);
	i32 rounded_max_x = rounded_min_x + bitmap->width;
	i32 rounded_max_y = rounded_min_y + bitmap->height;

	i32 rounded_min_offset_x = 0;
	if (rounded_min_x < 0) {
		rounded_min_offset_x = -rounded_min_x;
		rounded_min_x = 0;
	}

	if (rounded_min_y < 0) {
		rounded_min_y = 0;
	}

	if (rounded_max_x > display_buffer->width) {
		rounded_max_x = display_buffer->width;
	}
	
	if (rounded_max_y > display_buffer->height) {
		rounded_max_y = display_buffer->height;
	}

	u32* source_pixels = bitmap->pixels + (bitmap->width * (bitmap->height - 1));
	if (flip_horizontally) {
		source_pixels -= rounded_min_offset_x;
	}
	else {
		source_pixels += rounded_min_offset_x;
	}
	u8* destination_pixels = ((u8*)display_buffer->memory +
							  rounded_min_y * display_buffer->pitch +
							  rounded_min_x * display_buffer->bytes_per_pixel);
	
	for (i32 yy = rounded_min_y; yy < rounded_max_y; ++yy) {
		u32* source_color = source_pixels;
		if (flip_horizontally) {
			source_color += bitmap->width;
		}
		u32* destination_color = (u32*)destination_pixels;
		for (i32 xx = rounded_min_x; xx < rounded_max_x; ++xx) {
			f32 source_red = (f32)((*source_color >> 16) & 0xFF);
			f32 source_green = (f32)((*source_color >> 8) & 0xFF);
			f32 source_blue = (f32)((*source_color >> 0) & 0xFF);
			f32 source_alpha = (f32)((*source_color >> 24) & 0xFF);
			
			f32 source_alpha_scaled = source_alpha / 255.0f;

			f32 destination_red = (f32)((*destination_color >> 16) & 0xFF);
			f32 destination_green = (f32)((*destination_color >> 8) & 0xFF);
			f32 destination_blue = (f32)((*destination_color >> 0) & 0xFF);

			f32 red = (1.0f - source_alpha_scaled) * destination_red + source_red * source_alpha_scaled;
			f32 green = (1.0f - source_alpha_scaled) * destination_green + source_green * source_alpha_scaled;
			f32 blue = (1.0f - source_alpha_scaled) * destination_blue + source_blue * source_alpha_scaled;
			
			*destination_color++ = (((u32)(red + 0.5f) << 16) |
									((u32)(green + 0.5f) << 8) |
									((u32)(blue + 0.5f) << 0));

			if (flip_horizontally) {
				--source_color;
			}
			else {
				++source_color;
			}
		}

		source_pixels -= bitmap->width;
		destination_pixels += display_buffer->pitch;
	}
}

