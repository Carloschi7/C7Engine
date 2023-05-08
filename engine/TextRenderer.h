#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>

#include "Texture.h"
#include "Shader.h"

//Supports only numbers for now, work in progress
class TextRenderer
{
public:
	TextRenderer(uint32_t texture_binding);
	~TextRenderer();

	void PrintString(const std::string& str, const glm::mat4& model);
private:
	uint32_t m_TextureBinding;
	std::shared_ptr<VertexManager> m_VertexManager;
	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Texture> m_TextBitmap;

	//Draw data
	float m_Stride, m_CharWidth, m_Start;
	glm::vec2 m_NumStart, m_NumEnd;
};