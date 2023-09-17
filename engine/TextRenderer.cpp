#include "TextRenderer.h"
#include <iostream>

TextRenderer::TextRenderer(const glm::vec2& canvas_resolution, u32 texture_binding) :
	m_TextureBinding(texture_binding)
{
	m_Stride = 0.0555f;
	m_CharWidth = 0.05f;
	m_Start = 0.055f;
	m_SpacingOfY = -0.127f;
	m_NumStart = { m_Start, 0.78f }; // .13f average offset
	m_NumEnd = { m_Start + m_CharWidth, 0.88f};

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

	//Uniforming proj matrix
	glm::mat4 proj = glm::ortho(0.0f, canvas_resolution.x, canvas_resolution.y, 0.0f);
	m_Shader->UniformMat4f(proj, "proj");
}

TextRenderer::~TextRenderer()
{
}

void TextRenderer::DrawString(const std::string& str, glm::vec2 pos)
{
	//Just numbers supported for now
	if (str.empty())
		return;

	//White space is the first char in the bitmap texture
	u32 base_char_index = static_cast<u32>(' ');

	m_Shader->Use();
	m_VertexManager->BindVertexArray();
	m_Shader->Uniform1i(m_TextureBinding, "bitmap");

	glm::vec3 scaling_factor{ 35.0f, 35.0f, 0.0f };
	//Account for the scale in the translation so that the input coordinates define the actual top_left pixel


	for(u32 i = 0; i < str.size(); i++) {
		//Prepare the model matrix for the character
		glm::vec3 translation_vector = glm::vec3(pos.x + scaling_factor.x * i, pos.y, 0.0f) + scaling_factor / 2.0f;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), translation_vector);
		m_Shader->UniformMat4f(glm::scale(model, glm::vec3(35.0f)), "model");

		u32 cur_char_index = static_cast<u32>(str[i]) - base_char_index;
		float xoffset = m_Stride * static_cast<float>(cur_char_index % 16);
		float yoffset = m_SpacingOfY * static_cast<float>(cur_char_index / 16);
		m_Shader->Uniform1f(xoffset, "xoffset");
		m_Shader->Uniform1f(yoffset, "yoffset");
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}	
}
