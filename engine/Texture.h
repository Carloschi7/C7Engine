#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include "GL/glew.h"
#include "VertexManager.h"

#define GLError "[OpenGL]: Error in file:" << __FILE__ << ", line:" << __LINE__ << "\n"

//Defines how a texture is loaded
enum class TextureFilter {Linear = 0, Nearest};
enum class TexFormat : u8 {Rgb8 = 0, Rgba8, DepthComponentf32};

class Texture
{
public:
	Texture(const char* filepath, bool flipaxis = false, TextureFilter fmt = TextureFilter::Linear, u8 binding = 0);
	Texture(const Texture&) = delete;
	Texture(Texture&& tex) noexcept;
	~Texture();

	void Bind(unsigned int slot = 0) const;
	//Useful if the texture has been temporarily unbound
	static void ForceBind(unsigned int slot = 0);
	u32 ID() const { return m_TextureID; }
private:
	unsigned char* m_Data;
	u32 m_TextureID;
	s32 m_Width, m_Height, m_BPP;

	static u32 s_CurrentlyBoundTex;
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