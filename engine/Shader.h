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

class Shader
{
public:
	Shader(const std::string& filepath);
	Shader(const Shader&) = delete;
	Shader(Shader&& shd) noexcept;
	~Shader();
	
	void Use() const;
	void UniformMat4f(const glm::mat4& mat, const std::string& UniformName);
	void UniformVec3f(const glm::vec3& v, const std::string& UniformName);
	void UniformVec4f(const glm::vec4& v, const std::string& UniformName);
	void Uniform1i(int i, const std::string& UniformName);
	void Uniform1f(float i, const std::string& UniformName);
	bool IsUniformDefined(const std::string& UniformName) const;
	void ClearUniformCache();

	//Uniform buffer utilities(Beta)

	//@returns the index of the generated buffer in the vector
	u32 GenUniformBuffer(const std::string& block_name, u32 size, u32 binding_point);
	void DeleteUniformBuffers();
	/*
	*	@param retval of GenUniformBuffer
	*	@param size of the data
	*	@param offset of the data
	*	@param actual data
	*/
	void SendDataToUniformBuffer(u32 ub_local_index, u32 size, u32 offset, const void* data);
	void SetUniformBufferRange(u32 ub_local_index, u32 binding, u32 size, u32 offset);

	//Attribute utilities
	i32 GetAttributeLocation(const std::string& attr_name);
private:
	void BindUniformBuffer(u32 ub_local_index);
	void LoadShadersFromFile(const std::string& File, std::string& vs, std::string& gs, std::string& fs);
	i32 GetUniformLocation(const std::string& UniformName) const;
	int SetupShader(std::string& source, GLenum ShaderType);
	void CheckShaderCompileStatus(u32 shader, GLenum ShaderType);
private:
	u32 m_programID;
	std::vector<u32> m_UniformBuffers;
	//Uniform cache
	mutable std::unordered_map<std::string, i32> m_UniformCache;
	//Static var used to make Use function way faster
	static std::atomic<u32> s_CurrentlyBoundProgram;
	static std::atomic<u32> s_CurrentlyBoundUniformBuffer;
};
