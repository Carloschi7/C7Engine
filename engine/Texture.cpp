#include "Texture.h"
#include "MainIncl.h"
#include <utility>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "VertexManager.h"

void my_func(String s) {}

namespace gfx
{
	TextureData texture_create(const String& filepath)
	{
		return texture_create(filepath, texture_default_args());
	}

	TextureData texture_create(const String& filepath, const TextureArgs& args)
	{
		TextureData texture_data = {};

		const GLint texture_type = args.texture_type;
		assert(texture_type != GL_TEXTURE_CUBE_MAP, "this code initializes an individual texture not a group of them, in order to initialize a cube_map, texture_cubemap_create is a valid alternative");
		texture_data.texture_type = texture_type;

		glGenTextures(1, &texture_data.id);
		glActiveTexture(GL_TEXTURE0 + args.default_binding);
		glBindTexture(GL_TEXTURE_2D, texture_data.id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		assert(args.texture_filter == GL_LINEAR || args.texture_filter == GL_NEAREST,
			"no other options are valid at the moment");

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, args.texture_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, args.texture_filter);


		stbi_set_flip_vertically_on_load(args.flip_axis_on_load);
		u8*& raw_buffer = texture_data.raw_buffers[0];
		raw_buffer = stbi_load(filepath.c_str(), &texture_data.width, &texture_data.height,
			&texture_data.bytes_per_pixel, 4);

		if (raw_buffer)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture_data.width, texture_data.height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, raw_buffer);

			texture_data.initialized = true;
		} else {
			log_message("texture at path \"{}\" not loaded", filepath.c_str());
			texture_data.initialized = false;
		}

		if(!args.store_raw_buffer) {
			stbi_image_free(raw_buffer);
			raw_buffer = nullptr;
		}

		return texture_data;
	}

	TextureData texture_cubemap_create(const std::string* locations, u32 count)
	{
		return texture_cubemap_create(locations, count, texture_cubemap_default_args());
	}
	TextureData texture_cubemap_create(const std::string* locations, u32 count, const TextureArgs& args)
	{
		TextureData texture_data = {};

		const GLint texture_type = args.texture_type;
		assert(texture_type == GL_TEXTURE_CUBE_MAP, "this function is only used for GL_TEXTURE_CUBE_MAP initializations");
		texture_data.texture_type = GL_TEXTURE_CUBE_MAP;

		glGenTextures(1, &texture_data.id);
		glActiveTexture(GL_TEXTURE0 + args.default_binding);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_data.id);

		for(u32 i = 0; i < count; i++) {
			u8*& raw_buffer = texture_data.raw_buffers[i];

			//INFO: at the moment width, height and bytes_per_pixel are overridden each call,
			//so only the information of the last texture loaded is preserved (should not be a
			//concern since texture that are part of a cubemap usually share this values anyways)
			raw_buffer = stbi_load(locations[i].c_str(), &texture_data.width, &texture_data.height,
				&texture_data.bytes_per_pixel, 3);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, texture_data.width,
				texture_data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_buffer);

			if(!args.store_raw_buffer) {
				stbi_image_free(raw_buffer);
				raw_buffer = nullptr;
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		LayoutElement elem = { 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), 0 };
		//TODO @C7 transfer this in the temp memory when allocators are available
		const u32 vtx_cube_data_count = sizeof(vtx_cube_data) / sizeof(f32);
		f32* buf = new f32[vtx_cube_data_count];
		for(u32 i = 0; i < vtx_cube_data_count; i++) {
			buf[i] = vtx_cube_data[i] * args.cubemap_scaling;
		}
		texture_data.cubemap_mesh = new VertexMesh;
		*texture_data.cubemap_mesh = create_mesh_and_push_attributes(buf, vtx_cube_data_count * sizeof(f32),
			&elem, sizeof(LayoutElement));

		return texture_data;
	}

	glm::ivec2 texture_get_width_and_height(const TextureData& data)
	{
		//Return those if they are already cached
		if(data.width != 0 && data.height != 0)
			return glm::ivec2{ data.width, data.height };

		glm::ivec2 ret = {};

		texture_bind(data);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ret.x);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &ret.y);
		return ret;
	}

	void texture_bind(const TextureData& data, u8 slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(data.texture_type, data.id);
	}

	void texture_cleanup(TextureData* data)
	{
		for(u32 i = 0; i < max_texture_buffers_in_unit; i++) {
			if(data->raw_buffers[i])
				stbi_image_free(data->raw_buffers[i]);
		}

		if(data->cubemap_mesh) {
			cleanup_mesh(data->cubemap_mesh);
			delete data->cubemap_mesh;
		}

		glDeleteTextures(1, &data->id);
		data->initialized = false;
	}
}

Texture::Texture() : is_loaded(false)
{
}

Texture::Texture(const char* filepath, bool flipaxis, TextureFilter fmt, u8 binding)
{
	Load(filepath, flipaxis, fmt, binding);
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
	if (!is_loaded)
		return;

	glDeleteTextures(1, &m_TextureID);
	if (m_Data) stbi_image_free(m_Data);
}

void Texture::Load(const char* filepath, bool flipaxis, TextureFilter fmt, u8 binding)
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

		is_loaded = true;
	}
	else
	{
		std::cout << GLError << "Failed to load texture\n";
		is_loaded = false;
	}
}

glm::ivec2 Texture::GetWidthAndHeight()
{
	glm::ivec2 ret{0};
	u32 mip_level = 0;

	Bind();
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ret.x);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &ret.y);
	return ret;
}

void Texture::Bind(unsigned int slot) const
{
	assert(is_loaded, "the texture needs to be loaded when calling this method");
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

//CubeMap definitions
CubeMap::CubeMap(const std::vector<std::string>& files, f32 fScalingFactor)
	:m_Width(0), m_Height(0), m_TextureID(0), m_BPP(0), m_Data(nullptr)
{
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

	for (int i = 0; i < files.size(); i++)
	{
		m_Data = stbi_load(files[i].c_str(), &m_Width, &m_Height, &m_BPP, 3);

		if (m_Data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
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
	extern std::atomic<u32> current_vao_binding;
	u32 vao = m_VertexManager.GetVertexArray();
	if (current_vao_binding == vao)
		return;

	glBindVertexArray(m_VertexManager.GetVertexArray());
	current_vao_binding = vao;
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
