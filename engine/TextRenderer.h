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
	TextRenderer(const glm::vec2& canvas_resolution, u32 texture_binding);
	~TextRenderer();

	void DrawString(const std::string& str, glm::vec2 pos);
private:
	u32 m_TextureBinding;
	std::shared_ptr<VertexManager> m_VertexManager;
	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Texture> m_TextBitmap;

	//Draw data
	f32 m_Stride, m_CharWidth, m_Start, m_SpacingOfY;
	glm::vec2 m_NumStart, m_NumEnd;
};