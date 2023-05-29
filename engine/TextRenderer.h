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
	TextRenderer(const glm::vec2& canvas_resolution, uint32_t texture_binding);
	~TextRenderer();

	void DrawString(const std::string& str, glm::vec2 pos);
private:
	uint32_t m_TextureBinding;
	std::shared_ptr<VertexManager> m_VertexManager;
	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Texture> m_TextBitmap;

	//Draw data
	float m_Stride, m_CharWidth, m_Start, m_SpacingOfY;
	glm::vec2 m_NumStart, m_NumEnd;
};