#pragma once
#include <iostream>
#include <vector>
#include "GL/glew.h"
#include "VertexManager.h"

#define GLError "[OpenGL]: Error in file:" << __FILE__ << ", line:" << __LINE__ << "\n"

class Texture
{
public:
	Texture(const char* filepath, bool flipaxis = false);
	Texture(const Texture&) = delete;
	Texture(Texture&& tex) noexcept;
	~Texture();

	void Bind(unsigned int slot = 0) const;
	uint32_t ID() const { return m_TextureID; }
private:
	unsigned char* m_Data;
	uint32_t m_TextureID;
	int32_t m_Width, m_Height, m_BPP;
};

class CubeMap
{
public:
	CubeMap(const std::vector<std::string>& files, float fScalingFactor);
	~CubeMap();
	CubeMap(CubeMap&& right) noexcept;

	void BindTexture(uint32_t slot = 0);
	uint32_t GetVertexArray() const { return m_VertexManager.GetVertexArray(); }
	const VertexManager& GetVertexManager() { return m_VertexManager; }
private:
	unsigned char* m_Data;
	uint32_t m_TextureID;
	int32_t m_Width, m_Height, m_BPP;

	VertexManager m_VertexManager;
};