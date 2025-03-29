#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <atomic>
#include <vector>
#include "GL/glew.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "utils/types.h"

#define GLError "[OpenGL]: Error in file:" << __FILE__ << ", line:" << __LINE__ << "\n"

struct ShaderSource
{
    std::string vertex_shader_source;
    std::string geometry_shader_source;
    std::string fragment_shader_source;
    bool initialized;
};

struct FileParseResult
{
    u32 index = 0;
    bool is_eof = false;
};

class Shader
{
public:
	Shader();
	//INFO @C7 i dont know why, but if this parameter is a const char* a bunch of errors get thrown,
	//so just load a std::string here
	Shader(const std::string& filepath);
	Shader(const Shader&) = delete;
	Shader(Shader&& shd) noexcept;
	~Shader();

	void Load(const char* filepath);
	void Use() const;
	void UniformMat4f(const glm::mat4& mat, const char* uniform_name);
	void UniformVec2f(const glm::vec2& v,   const char* uniform_name);
	void UniformVec3f(const glm::vec3& v,   const char* uniform_name);
	void UniformVec4f(const glm::vec4& v,   const char* uniform_name);
	void Uniform1i(int i, const char* uniform_name);
	void Uniform1f(f32 i, const char* uniform_name);
	bool IsUniformDefined(const char* uniform_name) const;
	void ClearUniformCache();

	//Uniform buffer utilities(Beta)

	//@returns the index of the generated buffer in the vector
	u32 GenUniformBuffer(const char* block_name, u32 size, u32 binding_point);
	void DeleteUniformBuffers();
	void SendDataToUniformBuffer(u32 ub_local_index, u32 size, u32 offset, const void* data);
	void SetUniformBufferRange(u32 ub_local_index, u32 binding, u32 size, u32 offset);

	//Attribute utilities
	s32 GetAttributeLocation(const char* attr_name);
private:
	void BindUniformBuffer(u32 ub_local_index);
	ShaderSource LoadShadersFromFile(const char* File);
	s32 GetUniformLocation(const char* uniform_name) const;
	int SetupShader(std::string& source, GLenum ShaderType);
	void CheckShaderCompileStatus(u32 shader, GLenum ShaderType);
private:
	bool is_loaded;
	u32 m_programID;
	std::vector<u32> m_UniformBuffers;
	//Uniform cache
	mutable std::unordered_map<u64, s32> m_UniformCache;
};
