#pragma once
#include <iostream>
#include <vector>
#include "GL/glew.h"
#include "VertexManager.h"

#define GLError "[OpenGL]: Error in file:" << __FILE__ << ", line:" << __LINE__ << "\n"

//Defines how a texture is loaded
enum class TextureFilter {Linear = 0, Nearest};

class Texture
{
public:
	Texture(const char* filepath, bool flipaxis = false, TextureFilter fmt = TextureFilter::Linear, uint8_t binding = 0);
	Texture(const Texture&) = delete;
	Texture(Texture&& tex) noexcept;
	~Texture();

	void Bind(unsigned int slot = 0) const;
	//Useful if the texture has been temporarily unbound
	static void ForceBind(unsigned int slot = 0);
	uint32_t ID() const { return m_TextureID; }
private:
	unsigned char* m_Data;
	uint32_t m_TextureID;
	int32_t m_Width, m_Height, m_BPP;

	static uint32_t s_CurrentlyBoundTex;
};

class CubeMap
{
public:
	CubeMap(const std::vector<std::string>& files, float fScalingFactor);
	~CubeMap();
	CubeMap(CubeMap&& right) noexcept;

	void BindTexture(uint32_t slot = 0);
	uint32_t ID() const { return m_TextureID; }
	uint32_t GetVertexArray() const { return m_VertexManager.GetVertexArray(); }
	void BindVertexArray() const;
	const VertexManager& GetVertexManager() { return m_VertexManager; }
private:
	unsigned char* m_Data;
	uint32_t m_TextureID;
	int32_t m_Width, m_Height, m_BPP;

	VertexManager m_VertexManager;
};