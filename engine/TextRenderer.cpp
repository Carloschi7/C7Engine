#include "TextRenderer.h"
#include <iostream>

TextRenderer::TextRenderer(uint32_t texture_binding) :
	m_TextureBinding(texture_binding)
{
	m_Stride = 0.0555f;
	m_CharWidth = 0.05f;
	m_Start = 0.055f;
	m_SpacingOfY = -0.127f;
	m_NumStart = { m_Start, 0.88f }; // .13f average offset
	m_NumEnd = { m_Start + m_CharWidth, 0.78f};

	//Load resources
	float verts[]{
		-0.5f, -0.5f, m_NumStart.x, m_NumEnd.y,
		0.5f, -0.5f, m_NumEnd.x, m_NumEnd.y,
		0.5f, 0.5f, m_NumEnd.x, m_NumStart.y,
		-0.5f, 0.5f, m_NumStart.x, m_NumStart.y,
	};

	std::string texture_path = std::string(C7MACRO_ENGINE_PATH) + "/assets/textures/text_bitmap.png";
	std::string shader_path = std::string(C7MACRO_ENGINE_PATH) + "/assets/shaders/texture_bitmap.shader";

	Layout lyt;
	lyt.PushAttribute({ 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0 });
	lyt.PushAttribute({ 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, sizeof(float) * 2});
	m_VertexManager = std::make_shared<VertexManager>(verts, sizeof(verts), lyt);
	m_TextBitmap = std::make_shared<Texture>(texture_path.c_str(), true, TextureFilter::Nearest, 10);
	m_Shader = std::make_shared<Shader>(shader_path);
}

TextRenderer::~TextRenderer()
{
}

void TextRenderer::PrintString(const std::string& str, const glm::mat4& model)
{
	//Just numbers supported for now
	if (str.empty())
		return;

	//White space is the first char
	uint32_t base_char_index = static_cast<uint32_t>(' ');

	m_Shader->Use();
	m_VertexManager->BindVertexArray();
	m_Shader->Uniform1i(m_TextureBinding, "bitmap");
	m_Shader->UniformMat4f(model, "model");

	for(uint32_t i = 0; i < str.size(); i++){
		uint32_t cur_char_index = static_cast<uint32_t>(str[i]) - base_char_index;
		float xoffset = m_Stride * static_cast<float>(cur_char_index % 16);
		float yoffset = m_SpacingOfY * static_cast<float>(cur_char_index / 16);
		m_Shader->Uniform1f(xoffset, "xoffset");
		m_Shader->Uniform1f(yoffset, "yoffset");
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}	
}
