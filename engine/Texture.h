#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include "containers.h"
#include "GL/glew.h"
#include "VertexManager.h"
#include <glm/glm.hpp>

#define GLError "[OpenGL]: Error in file:" << __FILE__ << ", line:" << __LINE__ << "\n"

//Wrapper written to create a more straight-forward implementation for texture and cubemaps
namespace gfx
{
	//INFO: tells how many textures can be generated for a texture type, the maximum is
	//weighted on the amount of textures supported by the GL_TEXTURE_CUBE_MAP object at the moment
	static constexpr u32 max_cubemap_textures = 6;
	static constexpr u32 max_texture_buffers_in_unit = max_cubemap_textures;

	struct TextureArgs
	{
		bool flip_axis_on_load;
		bool store_raw_buffer;
		GLint texture_filter;
		GLint texture_type;
		f32 cubemap_scaling;
		//Only matters when loading the texture for the first time
		u8 default_binding;
	};

	//TODO @C7 prob needs to be renamed to Texture, just it creates conflicts with the already existing class at the moment
	struct TextureData
	{
		u32 id;
		s32 width, height, bytes_per_pixel;
		GLint texture_type;
		u8* raw_buffers[max_texture_buffers_in_unit];
		VertexMesh* cubemap_mesh;
		bool initialized;
	};

	static_assert(std::is_trivially_copyable_v<TextureData>, "needs to be trivially copiable because of zero initialization");

	inline TextureArgs texture_default_args()
	{
		TextureArgs args = {};
		args.flip_axis_on_load = false;
		args.store_raw_buffer = false;
		args.texture_filter = GL_LINEAR;
		args.texture_type = GL_TEXTURE_2D;
		args.cubemap_scaling = 0.0f; //Ignored for non-cubemap initialization
		args.default_binding = 0;
		return args;
	}

	inline TextureArgs texture_cubemap_default_args()
	{
		TextureArgs args = {};
		args.flip_axis_on_load = false;
		args.store_raw_buffer = false;
		args.texture_filter = GL_LINEAR;
		args.texture_type = GL_TEXTURE_CUBE_MAP;
		args.cubemap_scaling = 200.0f; // average value valid for most conditions
		args.default_binding = 0;
		return args;
	}

	TextureData texture_create(const char* filepath);
	TextureData texture_create(const char* filepath, const TextureArgs& args);
	TextureData texture_cubemap_create(const String* locations, u32 count);
	TextureData texture_cubemap_create(const String* locations, u32 count, const TextureArgs& args);
	glm::ivec2  texture_get_width_and_height(const TextureData& data);
	void        texture_bind(const TextureData& data, u8 slot = 0);
	void        texture_cleanup(TextureData* data);

}

//TODO(C7) @Obsolete?
enum class TextureFilter {Linear = 0, Nearest};
enum class TexFormat : u8 {Rgb8 = 0, Rgba8, DepthComponentf32};

class Texture
{
public:
	Texture();
	Texture(const char* filepath, bool flipaxis = false, TextureFilter fmt = TextureFilter::Linear, u8 binding = 0);
	Texture(const Texture&) = delete;
	Texture(Texture&& tex) noexcept;
	~Texture();

	void Load(const char* filepath, bool flipaxis = false, TextureFilter fmt = TextureFilter::Linear, u8 binding = 0);
	glm::ivec2 GetWidthAndHeight();
	void Bind(unsigned int slot = 0) const;
	inline u32 ID() const { return m_TextureID; }
	inline bool IsLoaded() const { return is_loaded; }
private:
	bool is_loaded;
	unsigned char* m_Data;
	u32 m_TextureID;
	s32 m_Width, m_Height, m_BPP;
};

class CubeMap
{
public:
	CubeMap(const std::vector<std::string>& files, f32 fScalingFactor);
	~CubeMap();
	CubeMap(CubeMap&& right) noexcept;

	void BindTexture(u32 slot = 0);
	u32 ID() const { return m_TextureID; }
	u32 GetVertexArray() const { return m_VertexManager.GetVertexArray(); }
	void BindVertexArray() const;
	const VertexManager& GetVertexManager() { return m_VertexManager; }
private:
	unsigned char* m_Data;
	u32 m_TextureID;
	s32 m_Width, m_Height, m_BPP;

	VertexManager m_VertexManager;
};

/*
*	USAGE EXAMPLE
* 	if (window->IsKeyboardEvent({ GLFW_KEY_M, GLFW_PRESS })) {
		f32* buf = new float[texture_width * texture_height];
		std::memset(buf, 0, texture_width * texture_height * sizeof(f32));
		texture.Bind();
		glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, buf);
		DumpTexture("output.ppm", buf, texture_width, texture_height, TexFormat::Rgb8);
		delete[] buf;
	}
*/
void DumpTexture(const std::string& filepath, const void* data, u32 width, u32 height, TexFormat format);
void DumpTexture_PPM(std::ofstream& file, const void* data, u32 width, u32 height, TexFormat format);