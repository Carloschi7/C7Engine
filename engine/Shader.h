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
	void SetUniformBindingPoint(const std::string& UniformBlockName, uint32_t binding_point);

	//Beta
	static void GenUniformBuffer(uint32_t size, uint32_t binding_point);
	static void DeleteUniformBuffer();
	static void SendDataToUniformBuffer(uint32_t size, uint32_t offset, const void* data);
private:
	void LoadShadersFromFile(const std::string& File, std::string& vs, std::string& gs, std::string& fs);
	int32_t GetUniformLocation(const std::string& UniformName) const;
	int SetupShader(std::string& source, GLenum ShaderType);
	void CheckShaderCompileStatus(uint32_t shader, GLenum ShaderType);
private:
	uint32_t m_programID;
	static uint32_t s_UniformBuffer;
	//Uniform cache
	mutable std::unordered_map<std::string, int32_t> m_UniformCache;
	//Static var used to make Use function way faster
	static std::atomic<uint32_t> s_CurrentlyBoundProgram;
};