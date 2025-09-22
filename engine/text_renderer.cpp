#include "text_renderer.h"
#include "containers.h"
#include "memory.h"

#include "memory.h"
#include <iostream>
#include "macros.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#define APPEND_STR(a, b) a##b

#ifdef WINDOWS_OS
#	define FONT_PATH "C:/Windows/Fonts/"
#elif defined LINUX_OS
	//TODO @C7 find the default linux path
#	define FONT_PATH ""
#else
	static_assert(false, "This OS is not supported");
#endif

namespace gfx
{

	void freetype_init(const char* font_name, const u32 screen_width, const u32 screen_height, const u32 texture_width, const u32 texture_height, FreetypeInstance* freetype_instance_ptr)
	{
		assert(freetype_instance_ptr, "This parameter needs to be defined");

		auto& freetype_instance = *freetype_instance_ptr;
		auto& library           = freetype_instance.library;
		auto& face              = freetype_instance.face;

		if(FT_Init_FreeType(&library)) {
			std::cout << "Cannot init FreeType\n";
			return;
		}

		{
			char* const font_path = string_merge(FONT_PATH, font_name);
			defer { temporary_free(font_path); };

			if(FT_New_Face(library, font_path, 0, &face)) {
				std::cout << "Cannot init the chosen font\n";
				return;
			}
		}

		//Load shader
		{
			char* const shader_path = string_merge(SOLUTION_PATH, "assets/shaders/text.shader");
			defer { temporary_free(shader_path); };

			freetype_instance.text_shader.Load(shader_path);
		}

		glm::mat4 proj;
		{
			f32 fwidth = (f32)screen_width, fheight = (f32)screen_height;
			//The origin from which the text is drawn considers top-left the (0,0) coordinate
		 	proj = glm::ortho(0.0f, fwidth, fheight, 0.0f);
		}
		freetype_instance.text_shader.UniformMat4f(proj, "proj");

		//Init buffer for batch rendering
        glGenVertexArrays(1, &freetype_instance.batched_glyphs_buffer.vertex_array);
        glBindVertexArray(freetype_instance.batched_glyphs_buffer.vertex_array);
        defer { glBindVertexArray(0); };

		glGenBuffers(1, &freetype_instance.batched_glyphs_buffer.vertex_buffer);

		glGenTextures(1, &freetype_instance.glyph_texture_handle);
		glBindTexture(GL_TEXTURE_2D, freetype_instance.glyph_texture_handle);

		s32 saved_unpack_alignment;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &saved_unpack_alignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        freetype_instance.texture_width  = texture_width;
        freetype_instance.texture_height = texture_height;

		const u32 glyph_height = 60;
		FT_Set_Pixel_Sizes(face, 0, glyph_height);

		//Load just the visible chars
		const u8 first_loaded_char = 32;
		const u8 last_loaded_char  = 126;

		s32 max_bearing_y = 0;

		s32 x_offset = 0, y_offset = 0;
		//Load basic info and figure out the max_bearing_y
		for(u8 i = first_loaded_char; i <= first_loaded_char; i++) {
			if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
				std::cout << "Cannot load char:" << i << "\n";
			}

			if(face->glyph->bitmap_top > max_bearing_y)
				max_bearing_y = face->glyph->bitmap_top;
		}

		//Draw glyphs onto the texture
		for(u8 i = first_loaded_char; i <= last_loaded_char; i++) {
			if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
				std::cout << "Cannot load char:" << i << "\n";
			}

			if((i - first_loaded_char) % 16 == 0) {
				//Create an initial padding
				x_offset = face->glyph->bitmap_left;
				if(x_offset < 0) x_offset = 0;

				//Create a y offset a little bit higher than the glyph height
				if(i != first_loaded_char)
					y_offset += glyph_height + 5;
			}

			auto& info = freetype_instance.glyph_info[i];
			info.size_x    = face->glyph->bitmap.width;
			info.size_y    = face->glyph->bitmap.rows;
			info.bearing_x = face->glyph->bitmap_left;
			info.bearing_y = face->glyph->bitmap_top;
			info.advance   = (face->glyph->advance.x >> 6) + 3;
			info.offset_x  = x_offset;
			info.offset_y  = y_offset;
			info.offset_from_top = max_bearing_y - info.bearing_y;
			//Convert from local font unit to pixels, plus add 3 padding pixels to make everything clearer

			glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

			x_offset += info.advance;
		}

		//TODO @C7 the fact that defer is defined in MainIncl.h is ridiculus, just create a macro header
		glPixelStorei(GL_UNPACK_ALIGNMENT, saved_unpack_alignment);
		freetype_instance.initialized = true;
	}

	void draw_text(FreetypeInstance* freetype_instance_ptr, const char* str, u32 x, u32 y, f32 scale)
	{
		//INFO: @C7 the shader is provided externally at the moment, just requires:
		//layout = 0 => vec2 pos;
		//layout = 1 => vec2 tex_coords;

		assert(freetype_instance_ptr, "This parameter needs to be defined");
		auto& freetype_instance = *freetype_instance_ptr;

		u32 get_c_string_length_no_null_terminating(const char*);
		const u32 string_length = get_c_string_length_no_null_terminating(str);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, freetype_instance.glyph_texture_handle);
		freetype_instance.text_shader.Uniform1i(0, "glyph_texture");

		auto& mesh = freetype_instance.batched_glyphs_buffer;
        const u32 projected_array_size = string_length * 6 * 4 * sizeof(f32);
        //extend the buffer if more room is required
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
        if(projected_array_size > freetype_instance.batched_glyphs_buffer_size) {

        	freetype_instance.batched_glyphs_buffer_size = projected_array_size;
        	glBufferData(GL_ARRAY_BUFFER, projected_array_size, nullptr, GL_DYNAMIC_DRAW);
        }
		//INFO @C7 for good practices bind the vertex array AFTER the vbo resizing as if done incorrectly
		//that operation could invalidate the VAO binding
        glBindVertexArray(mesh.vertex_array);

		LayoutElement elements[2];
		elements[0] = {2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0};
		elements[1] = {2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 2 * sizeof(f32)};
		gfx::push_mesh_attributes(&mesh, elements, sizeof(elements));

		const u32 texture_width  = freetype_instance.texture_width;
		const u32 texture_height = freetype_instance.texture_height;

		//Create a temporary buffer to store all the batched data and then send everything to opengl in a single glBufferSubData api call
		f32* batched_data_buffer = temporary_allocate<f32>(6 * 4 * string_length);
		defer { temporary_free(batched_data_buffer); };

		u32 buffer_offset = 0;
		for(u32 i = 0; i < string_length; i++) {
			const char current_char = str[i];
			auto& info = freetype_instance.glyph_info[current_char];

			f32 start_x = 100.0f, start_y = 100.0f, size_x = 0.0f, size_y = 0.0f, offset_x = 0.0f, offset_y = 0.0f, tex_size_x = 0.0f, tex_size_y = 0.0f;

			start_x    = (f32)x + (f32)info.bearing_x;
			start_y    = (f32)y + (f32)info.offset_from_top;
			size_x     = (f32)info.size_x;
			size_y     = (f32)info.size_y;
			offset_x   = ((f32)info.offset_x)    / (f32)texture_width;
			offset_y   = ((f32)info.offset_y)    / (f32)texture_height;
			tex_size_x = ((f32)info.size_x)        / (f32)texture_width;
			tex_size_y = ((f32)info.size_y) / (f32)texture_height;


			size_x    *= scale;
			size_y    *= scale;

			f32 data[] = {
				start_x,            start_y,            offset_x,               offset_y,
				start_x + size_x,   start_y,            offset_x + tex_size_x,  offset_y,
				start_x + size_x,   start_y + size_y,   offset_x + tex_size_x,  offset_y + tex_size_y,

				start_x + size_x,   start_y + size_y,   offset_x + tex_size_x,  offset_y + tex_size_y,
				start_x,            start_y + size_y,   offset_x,               offset_y + tex_size_y,
				start_x,            start_y,            offset_x,               offset_y,
			};


			u32 single_letter_buffer_size = 6 * 4 * sizeof(f32);
			u8* batched_data_buffer_u8 = (u8*)batched_data_buffer;
			std::memcpy(batched_data_buffer_u8 + buffer_offset, data, single_letter_buffer_size);
			buffer_offset += single_letter_buffer_size;

			x += (u32)((f32)info.advance * scale);
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, projected_array_size, batched_data_buffer);
		glDrawArrays   (GL_TRIANGLES, 0, 6 * string_length);
	}

	void freetype_deinit(FreetypeInstance* freetype_instance)
	{
		cleanup_mesh(&freetype_instance->batched_glyphs_buffer);

		FT_Done_Face(freetype_instance->face);
		FT_Done_FreeType(freetype_instance->library);
	}
}
