#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char* filepath, bool flipaxis)
{
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(flipaxis);
	m_Data = stbi_load(filepath, &m_Width, &m_Height, &m_BPP, 4);

	if (m_Data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, 
			GL_RGBA, GL_UNSIGNED_BYTE, m_Data);
	}
	else
	{
		std::cout << GLError << "Failed to load texture\n";
	}
}

Texture::Texture(Texture&& tex) noexcept :
	m_Width(std::exchange(tex.m_Width, 0)),
	m_Height(std::exchange(tex.m_Height, 0)),
	m_BPP(std::exchange(tex.m_BPP, 0)),
	m_Data(std::exchange(tex.m_Data, nullptr)),
	m_TextureID(std::exchange(tex.m_TextureID, 0))
{
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextureID);
	if (m_Data) stbi_image_free(m_Data);
}

void Texture::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

//CubeMap definitions

CubeMap::CubeMap(const std::vector<std::string>& files, float fScalingFactor)
	:m_Width(0), m_Height(0), m_TextureID(0), m_BPP(0), m_Data(nullptr)
{
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

	stbi_set_flip_vertically_on_load(true);
	for (int i = 0; i < files.size(); i++)
	{
		m_Data = stbi_load(files[i].c_str(), &m_Width, &m_Height, &m_BPP, 3);


		if (m_Data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8,
				m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_Data);

			
		}
		else
		{
			std::cout << "Failed to load cubemap texture " << files[i] << std::endl;
		}

		stbi_image_free(m_Data);
		m_Data = nullptr;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Cubemap buffer data
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	for (int i = 0; i < 108; i++)
	{
		skyboxVertices[i] *= fScalingFactor;
	}

	Layout layout;
	layout.PushAttribute({ 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0 });
	m_VertexManager.SendDataToOpenGLArray(skyboxVertices, sizeof(skyboxVertices), layout);
}

CubeMap::~CubeMap()
{
	stbi_image_free(m_Data);
	glDeleteTextures(1, &m_TextureID);
}

CubeMap::CubeMap(CubeMap&& right) noexcept
	:m_VertexManager(std::move(right.m_VertexManager)),
	m_Width(std::exchange(right.m_Width, 0)),
	m_Height(std::exchange(right.m_Height, 0)),
	m_BPP(std::exchange(right.m_BPP, 0)),
	m_Data(std::exchange(right.m_Data, nullptr)),
	m_TextureID(std::exchange(right.m_TextureID, 0))
{
}

void CubeMap::BindTexture(uint32_t slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
}
