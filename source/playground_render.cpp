internal void DrawRectangle(Bitmap* display_buffer,
							v2 min, v2 max,
							v4 color)
{
	i32 rounded_min_x = RoundF32ToI32(min.x);
	i32 rounded_min_y = RoundF32ToI32(min.y);
	i32 rounded_max_x = RoundF32ToI32(max.x);
	i32 rounded_max_y = RoundF32ToI32(max.y);

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

	u32 color32 = ((RoundF32ToU32(color.r * 255.0f) << 16) |
				   (RoundF32ToU32(color.g * 255.0f) << 8) |
				   (RoundF32ToU32(color.b * 255.0f) << 0));

	u8* pixels = ((u8*)display_buffer->memory +
				  rounded_min_x * BITMAP_BYTES_PER_PIXEL +
				  rounded_min_y * display_buffer->pitch);

	for (i32 y = rounded_min_y; y < rounded_max_y; ++y) {
		u32* row = (u32*)pixels;
		for (i32 x = rounded_min_x; x < rounded_max_x; ++x) {
			*row++ = color32;
		}
		pixels += display_buffer->pitch;
	}
}

internal void DrawRectangleWithBorder(Bitmap* display_buffer,
									  v2 min, v2 max,
									  f32 r, f32 g, f32 b,
									  u8 border_width,
									  f32 border_r, f32 border_g, f32 border_b,
									  b32 only_border=false)
{
	i32 rounded_min_x = RoundF32ToI32(min.x);
	i32 rounded_min_y = RoundF32ToI32(min.y);
	i32 rounded_max_x = RoundF32ToI32(max.x);
	i32 rounded_max_y = RoundF32ToI32(max.y);

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
				  rounded_min_x * BITMAP_BYTES_PER_PIXEL +
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

internal Bitmap LoadBmp(char* file_name, PlaygroundReadFileCallback* PlaygroundReadFile, i32 bitmap_align_x=0, i32 bitmap_align_y=0)
{
	Bitmap loaded_bmp = {};
	loaded_bmp.align_x = bitmap_align_x;
	loaded_bmp.align_y = bitmap_align_y;
	
	PlaygroundFile bmp_file = PlaygroundReadFile(file_name);
	if (bmp_file.size != 0) {
		BmpHeader* bmp_header = (BmpHeader*)bmp_file.contents;

		loaded_bmp.memory = (u32*)((u8*)bmp_header + bmp_header->bitmap_offset);
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

		u32* pixels = (u32*)loaded_bmp.memory;
		for (i32 y = 0; y < bmp_header->height; ++y) {
			for (i32 x = 0; x < bmp_header->width; ++x) {
				u32 color = *pixels;

				f32 r = (f32)(((color & bmp_header->red_mask) >> red_scan.index));
				f32 g = (f32)(((color & bmp_header->green_mask) >> green_scan.index));
				f32 b = (f32)(((color & bmp_header->blue_mask) >> blue_scan.index));
				f32 a = (f32)(((color & bmp_header->alpha_mask) >> alpha_scan.index));
				f32 a_normalized = a / 255.0f;

				r *= a_normalized;
				g *= a_normalized;
				b *= a_normalized;
				
				// *pixels++ = (BitsRotateLeft(color & bmp_header->red_mask, red_rotate) |
				// 			 BitsRotateLeft(color & bmp_header->green_mask, green_rotate) |
				// 			 BitsRotateLeft(color & bmp_header->blue_mask, blue_rotate) |
				// 			 BitsRotateLeft(color & bmp_header->alpha_mask, alpha_rotate));

				*pixels++ = (((u32)(a + 0.5f) << 24) |
							 ((u32)(r + 0.5f) << 16) |
							 ((u32)(g + 0.5f) << 8) |
							 ((u32)(b + 0.5f) << 0));
			}
		}
	}

	loaded_bmp.pitch = -loaded_bmp.width * BITMAP_BYTES_PER_PIXEL;
	loaded_bmp.memory = (u8*)loaded_bmp.memory - loaded_bmp.pitch * (loaded_bmp.height - 1);
	
	return loaded_bmp;
}

internal Bitmap ScaleBmp(PlaygroundMemoryArena* arena,
						 Bitmap* bitmap, i32 new_width, i32 new_height)
{
	Bitmap scaled_bitmap;
	scaled_bitmap.memory = (void*)PushArray(arena, new_width * new_height, u32);
	scaled_bitmap.width = new_width;
	scaled_bitmap.height = new_height;
	scaled_bitmap.pitch = -scaled_bitmap.width * BITMAP_BYTES_PER_PIXEL;
	scaled_bitmap.memory = (u8*)scaled_bitmap.memory - scaled_bitmap.pitch * (scaled_bitmap.height - 1);
	
	f32 x_ratio = (f32)(bitmap->width - 1) / (f32)new_width;
	f32 y_ratio = (f32)(bitmap->height - 1) / (f32)new_height;

	scaled_bitmap.align_x = (i32)(bitmap->align_x / x_ratio);
	scaled_bitmap.align_y = (i32)(bitmap->align_y / y_ratio);

	for (i32 y = 0; y < new_height; ++y) {
		for (i32 x = 0; x < new_width; ++x) {
			i32 scaled_x = FloorF32ToI32(x_ratio * x);
			i32 scaled_y = FloorF32ToI32(y_ratio * y);

			u32* source_color = (u32*)((u8*)bitmap->memory + scaled_y * bitmap->pitch + scaled_x * BITMAP_BYTES_PER_PIXEL);
			u32* destination_color = (u32*)((u8*)scaled_bitmap.memory + y * scaled_bitmap.pitch + x * BITMAP_BYTES_PER_PIXEL);
			*destination_color = *source_color;

			// scaled_bitmap_pixels[y * new_width + x] = bitmap_pixels[scaled_y * bitmap->width + scaled_x];
		}
	}

	return scaled_bitmap;
}

internal void DrawBitmap(Bitmap* display_buffer,
						 Bitmap* bitmap,
						 f32 x, f32 y,
						 b32 flip_horizontally,
						 v4 color)
{
	// x -= flip_horizontally ? bitmap->width - align_x : align_x;
	// y -= align_y;
	
	i32 rounded_min_x = RoundF32ToI32(x);
	i32 rounded_min_y = RoundF32ToI32(y);
	i32 rounded_max_x = rounded_min_x + bitmap->width;
	i32 rounded_max_y = rounded_min_y + bitmap->height;

	i32 rounded_min_offset_x = 0;
	if (rounded_min_x < 0) {
		rounded_min_offset_x = -rounded_min_x;
		rounded_min_x = 0;
	}

	i32 rounded_min_offset_y = 0;
	if (rounded_min_y < 0) {
		rounded_min_offset_y = -rounded_min_y;
		rounded_min_y = 0;
	}

	if (rounded_max_x > display_buffer->width) {
		rounded_max_x = display_buffer->width;
	}
	
	if (rounded_max_y > display_buffer->height) {
		rounded_max_y = display_buffer->height;
	}

	rounded_min_offset_x = flip_horizontally ?
		-rounded_min_offset_x : rounded_min_offset_x; 
	
	u8* source_pixels = (u8*)bitmap->memory +
		bitmap->pitch * rounded_min_offset_y +
		BITMAP_BYTES_PER_PIXEL * rounded_min_offset_x;
	// if (flip_horizontally) {
	// 	source_pixels -= BITMAP_BYTES_PER_PIXEL * rounded_min_offset_x;
	// }
	// else {
	// 	source_pixels += BITMAP_BYTES_PER_PIXEL * rounded_min_offset_x;
	// }
	u8* destination_pixels = ((u8*)display_buffer->memory +
							  rounded_min_y * display_buffer->pitch +
							  rounded_min_x * BITMAP_BYTES_PER_PIXEL);
	
	for (i32 yy = rounded_min_y; yy < rounded_max_y; ++yy) {
		u32* source_color = (u32*)source_pixels;
		if (flip_horizontally) {
			source_color += bitmap->width;
		}
		u32* destination_color = (u32*)destination_pixels;
		for (i32 xx = rounded_min_x; xx < rounded_max_x; ++xx) {
			f32 source_alpha = (f32)((*source_color >> 24) & 0xFF);
			f32 source_alpha_normalized = (source_alpha / 255.0f) * color.a;
			
			f32 source_red = color.a * (f32)((*source_color >> 16) & 0xFF);
			f32 source_green = color.a * (f32)((*source_color >> 8) & 0xFF);
			f32 source_blue = color.a * (f32)((*source_color >> 0) & 0xFF);

			f32 destination_red = (f32)((*destination_color >> 16) & 0xFF);
			f32 destination_green = (f32)((*destination_color >> 8) & 0xFF);
			f32 destination_blue = (f32)((*destination_color >> 0) & 0xFF);
			f32 destination_alpha = (f32)((*destination_color >> 24) & 0xFF);

			f32 destination_alpha_normalized = destination_alpha / 255.0f;

			// f32 red = (1.0f - source_alpha_normalized) * destination_red + source_red * source_alpha_normalized;
			// f32 green = (1.0f - source_alpha_normalized) * destination_green + source_green * source_alpha_normalized;
			// f32 blue = (1.0f - source_alpha_normalized) * destination_blue + source_blue * source_alpha_normalized;

			f32 alpha = 255.0f * (source_alpha_normalized + destination_alpha_normalized - (source_alpha_normalized * destination_alpha_normalized));
			f32 red = (1.0f - source_alpha_normalized) * destination_red + source_red;
			f32 green = (1.0f - source_alpha_normalized) * destination_green + source_green;
			f32 blue = (1.0f - source_alpha_normalized) * destination_blue + source_blue;
			
			*destination_color++ = (((u32)(alpha + 0.5f) << 24) |
									((u32)(red + 0.5f) << 16) |
									((u32)(green + 0.5f) << 8) |
									((u32)(blue + 0.5f) << 0));

			if (flip_horizontally) {
				--source_color;
			}
			else {
				++source_color;
			}
		}

		source_pixels += bitmap->pitch;
		destination_pixels += display_buffer->pitch;
	}
}

internal RenderGroup* AllocateRenderGroup(PlaygroundMemoryArena* arena, u32 size, f32 meters_to_pixels)
{
	RenderGroup* render_group = PushStruct(arena, RenderGroup);
	
	v2* position = PushStruct(arena, v2);
	*position = v2();
	render_group->position = position;
	render_group->meters_to_pixels = meters_to_pixels;
	render_group->max_buffer_size = size;
	render_group->buffer_size = 0;
	render_group->base_buffer = (u8*)PushSize(arena, render_group->max_buffer_size);

	return render_group;
}

#define PushRenderGroupElement(render_group, type) (type*)PushRenderGroupElement_(render_group, sizeof(type), RENDER_GROUP_ELEMENT_TYPE_##type)
internal void* PushRenderGroupElement_(RenderGroup* render_group, u32 size, RenderGroupElementType type)
{
	size += sizeof(RenderGroupElementHeader);
	
	ASSERT(render_group->buffer_size + size < render_group->max_buffer_size);
	
	RenderGroupElementHeader* render_group_element_header = (RenderGroupElementHeader*)(render_group->base_buffer + render_group->buffer_size);
	render_group_element_header->type = type;
	
	void* render_group_element = (u8*)render_group_element_header + sizeof(*render_group_element_header);
	
	render_group->buffer_size += size;

	return render_group_element;
}

internal void BitmapCall(RenderGroup* render_group, Bitmap* bitmap, v2 offset, v2 align, v4 color, b32 flip_horizontally=false)
{
	RenderGroupElementBitmap* render_group_element = PushRenderGroupElement(render_group, RenderGroupElementBitmap);
	
	if (render_group_element) {
		render_group_element->bitmap = bitmap;
		render_group_element->flip_horizontally = flip_horizontally;
		render_group_element->spec.position = render_group->position;
		align = v2(flip_horizontally ? bitmap->width - align.x : align.x,
				   align.y);
		render_group_element->spec.offset = render_group->meters_to_pixels * v2(offset.x, -offset.y) - align;
		render_group_element->color = color;
	}
}

internal void RectangleCall(RenderGroup* render_group, v2 offset, v2 dimension, v4 color)
{
	RenderGroupElementRectangle* render_group_element = PushRenderGroupElement(render_group, RenderGroupElementRectangle);
	
	if (render_group_element) {
		render_group_element->dimension = render_group->meters_to_pixels * dimension;
		render_group_element->spec.position = render_group->position;
		render_group_element->spec.offset = render_group->meters_to_pixels * v2(offset.x, -offset.y);
		render_group_element->color = color;
	}
}

internal void RectangleOutlineCall(RenderGroup* render_group, v2 offset, v2 dimension, v4 color)
{
	f32 thickness = 0.1f;
	v2 half_dimension = 0.5f * dimension;
	
	RectangleCall(render_group,
				  v2(offset.x, offset.y + half_dimension.y),
				  v2(dimension.x, thickness), color);
	RectangleCall(render_group,
				  v2(offset.x, offset.y - half_dimension.y),
				  v2(dimension.x, thickness), color);

	RectangleCall(render_group,
				  v2(offset.x + half_dimension.x, offset.y),
				  v2(thickness, dimension.y), color);
	RectangleCall(render_group,
				  v2(offset.x - half_dimension.x, offset.y),
				  v2(thickness, dimension.y), color);
}

internal void ClearCall(RenderGroup* render_group, v4 color)
{
	RenderGroupElementClear* render_group_element = PushRenderGroupElement(render_group, RenderGroupElementClear);
	
	if (render_group_element) {
		render_group_element->color = color;
	}
}

internal v2 GetScreenPosition(RenderGroup* render_group, RenderElementSpec* spec, v2 screen_center)
{
	v2 position = *(spec->position);
			
	v2 center_position = v2(screen_center.x + spec->offset.x + render_group->meters_to_pixels * position.x,
							screen_center.y + spec->offset.y - render_group->meters_to_pixels * position.y);

	return center_position;
}

internal void RenderGroupToTargetBuffer(RenderGroup* render_group, Bitmap* target_buffer, v2 center)
{
	for (u32 address = 0;
		 address < render_group->buffer_size;
		 ) {
		RenderGroupElementHeader* element_header = (RenderGroupElementHeader*)(render_group->base_buffer + address);
		address += sizeof(*element_header);
		
		void* element = (u8*)element_header + sizeof(*element_header);
		switch (element_header->type) {
			case RENDER_GROUP_ELEMENT_TYPE_RenderGroupElementBitmap: {
				RenderGroupElementBitmap* render_group_element = (RenderGroupElementBitmap*)element;

				v2 position = GetScreenPosition(render_group, &render_group_element->spec, center);

				DrawBitmap(target_buffer, render_group_element->bitmap,
						   position.x, position.y, render_group_element->flip_horizontally, render_group_element->color);

				address += sizeof(*render_group_element);
			} break;

			case RenderGroupElementType::RENDER_GROUP_ELEMENT_TYPE_RenderGroupElementRectangle: {
				RenderGroupElementRectangle* render_group_element = (RenderGroupElementRectangle*)element;

				v2 position = GetScreenPosition(render_group, &render_group_element->spec, center);
				v2 half_dimension = 0.5f * render_group_element->dimension;

				DrawRectangle(target_buffer,
							  position - half_dimension,
							  position + half_dimension,
							  render_group_element->color);

				address += sizeof(*render_group_element);
			} break;

			case RenderGroupElementType::RENDER_GROUP_ELEMENT_TYPE_RenderGroupElementClear: {
				RenderGroupElementClear* render_group_element = (RenderGroupElementClear*)element;

				DrawRectangle(target_buffer,
							  v2(),
							  v2((f32)target_buffer->width, (f32)target_buffer->height),
							  render_group_element->color);

				address += sizeof(*render_group_element);
			} break;

			INVALID_DEFAULT_CASE;
		};

	}
}
