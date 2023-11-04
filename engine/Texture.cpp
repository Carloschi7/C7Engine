#include "Texture.h"
#include <utility>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

u32 Texture::s_CurrentlyBoundTex = 0;

Texture::Texture(const char* filepath, bool flipaxis, TextureFilter fmt, u8 binding)
{
	glGenTextures(1, &m_TextureID);
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	switch (fmt)
	{
	case TextureFilter::Linear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case TextureFilter::Nearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	}
	

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
	if (s_CurrentlyBoundTex == m_TextureID)
		return;

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	s_CurrentlyBoundTex = m_TextureID;
}

void Texture::ForceBind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, s_CurrentlyBoundTex);
}

//CubeMap definitions

CubeMap::CubeMap(const std::vector<std::string>& files, f32 fScalingFactor)
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
	f32 skyboxVertices[] = {
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
	layout.PushAttribute({ 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0 });
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

void CubeMap::BindTexture(u32 slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
}

void CubeMap::BindVertexArray() const
{
	glBindVertexArray(m_VertexManager.GetVertexArray());
}

void DumpTexture(const std::string& filepath, const void* data, u32 width, u32 height, TexFormat format)
{
	std::ofstream file(filepath, std::ios::out);

	std::string extension = filepath.substr(filepath.find_last_of('.'));
	//Only format implemented yet
	if (extension == ".ppm") {
		DumpTexture_PPM(file, data, width, height, format);
	}
	file.close();
}

void DumpTexture_PPM(std::ofstream& file, const void* data, u32 width, u32 height, TexFormat format)
{
	file << "P3\n";
	file << std::to_string(width) << " " << std::to_string(height) << "\n";
	//64 level of colors per channel
	file << "255\n";

	//Initializing pointers that will be used depending on the texture format
	const uint8_t* u8_data = static_cast<const uint8_t*>(data);
	const float* f32_data = static_cast<const float*>(data);

	switch (format)
	{
	case TexFormat::Rgb8:

		for (uint32_t i = 0; i < height; i++)
		{
			for (uint32_t j = 0; j < width; j++)
			{
				std::string vals[3];
				for (int k = 0; k < 3; k++)
				{
					uint32_t index = i * (width * 3) + (j * 3) + k;
					vals[k] = std::to_string(static_cast<uint32_t>(u8_data[index]));
				}

				file << vals[0] << " " << vals[1] << " " << vals[2] << "\n";
			}
		}

		break;
	case TexFormat::Rgba8:
		//Alpha channel gets ignored
		for (uint32_t i = 0; i < height; i++)
		{
			for (uint32_t j = 0; j < width; j++)
			{
				std::string vals[3];
				for (int k = 0; k < 3; k++)
				{
					uint32_t index = i * (width * 4) + (j * 4) + k;
					vals[k] = std::to_string(static_cast<uint32_t>(u8_data[index]));
				}

				file << vals[0] << " " << vals[1] << " " << vals[2] << "\n";
			}
		}

		break;
	case TexFormat::DepthComponentf32:

		for (uint32_t i = 0; i < height; i++)
		{
			for (uint32_t j = 0; j < width; j++)
			{
				uint32_t val = static_cast<uint32_t>(f32_data[i * width + j] * 255.0f);
				std::string str = std::to_string(val);
				file << str << " " << str << " " << str << "  ";
			}
			file << "\n";
		}

		break;
	};
}
