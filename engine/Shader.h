#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <atomic>
#include "GL/glew.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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
	uint32_t GenUniformBuffer(const std::string& block_name, uint32_t size, uint32_t binding_point);
	void DeleteUniformBuffers();
	/*
	*	@param retval of GenUniformBuffer
	*	@param size of the data
	*	@param offset of the data
	*	@param actual data
	*/
	void SendDataToUniformBuffer(uint32_t ub_local_index, uint32_t size, uint32_t offset, const void* data);
	void SetUniformBufferRange(uint32_t ub_local_index, uint32_t binding, uint32_t size, uint32_t offset);
private:
	void BindUniformBuffer(uint32_t ub_local_index);
	void LoadShadersFromFile(const std::string& File, std::string& vs, std::string& gs, std::string& fs);
	int32_t GetUniformLocation(const std::string& UniformName) const;
	int SetupShader(std::string& source, GLenum ShaderType);
	void CheckShaderCompileStatus(uint32_t shader, GLenum ShaderType);
private:
	uint32_t m_programID;
	std::vector<uint32_t> m_UniformBuffers;
	//Uniform cache
	mutable std::unordered_map<std::string, int32_t> m_UniformCache;
	//Static var used to make Use function way faster
	static std::atomic<uint32_t> s_CurrentlyBoundProgram;
	static std::atomic<uint32_t> s_CurrentlyBoundUniformBuffer;
};