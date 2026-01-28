#pragma once

//INFO: This part of the engine uses freetype to load fonts and to display them
#include <ft2build.h>
#include FT_FREETYPE_H
#include "VertexManager.h"
#include "Shader.h"

#include "utils/types.h"

namespace gfx
{
	struct CharInfo {
		u32 size_x, size_y;
		u32 bearing_x, bearing_y;
		//Refers to texture offset
		u32 offset_x, offset_y;
		s32 offset_from_top;
		u32 advance;
	};

	struct FreetypeInstance
	{
		FT_Library library;
		//Represents the font loaded by default, at the moment allow support for only one font at a time
		FT_Face face;

		VertexMesh batched_glyphs_buffer;
		u32        batched_glyphs_buffer_size = 0;
		u32        font_size;

		Shader     text_shader;

		u32 glyph_texture_handle;
		u32 texture_width, texture_height;
		//Have a free slot for each UTF8 glyph (not all of them will be used tho)
		CharInfo glyph_info[256];

		bool initialized;
	};

	void freetype_init(const char* font_name, const u32 screen_width, const u32 screen_height, const u32 texture_width, const u32 texture_height, FreetypeInstance* freetype_instance_ptr);
	//The rendering pipeline of the text is all handled by the function itself, including the shading process
	void draw_text(FreetypeInstance* freetype_instance_ptr, const char* str, u32 x, u32 y, f32 scale);
	void freetype_deinit(FreetypeInstance* freetype_instance);
}