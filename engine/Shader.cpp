#include "Shader.h"

std::atomic<uint32_t> Shader::s_CurrentlyBoundProgram = 0;
std::atomic<uint32_t> Shader::s_CurrentlyBoundUniformBuffer = 0;

Shader::Shader(const std::string& filepath)
{
	m_programID = glCreateProgram();

	unsigned int vs, gs, fs;
	std::string VertShader, GeomShader, FragShader;
	LoadShadersFromFile(filepath, VertShader, GeomShader, FragShader);
	bool IsGeomShaderDefined = !GeomShader.empty();

	vs = SetupShader(VertShader, GL_VERTEX_SHADER);
	if (IsGeomShaderDefined) gs = SetupShader(GeomShader, GL_GEOMETRY_SHADER);
	fs = SetupShader(FragShader, GL_FRAGMENT_SHADER);

	glAttachShader(m_programID, vs);
	if (IsGeomShaderDefined) glAttachShader(m_programID, gs);
	glAttachShader(m_programID, fs);

	glLinkProgram(m_programID);
	glValidateProgram(m_programID);

	glDeleteShader(vs);
	if (IsGeomShaderDefined) glDeleteShader(gs);
	glDeleteShader(fs);
}

Shader::Shader(Shader&& shd) noexcept
{
	this->m_programID = shd.m_programID;
	shd.m_programID = 0;
}

Shader::~Shader()
{
	DeleteUniformBuffers();
	glDeleteProgram(m_programID);
}

void Shader::Use() const
{
	if (m_programID != s_CurrentlyBoundProgram)
	{
		glUseProgram(m_programID);
		s_CurrentlyBoundProgram = m_programID;
	}
}

void Shader::UniformMat4f(const glm::mat4& mat, const std::string& UniformName)
{
	Use();
	glUniformMatrix4fv(GetUniformLocation(UniformName), 1, GL_FALSE, &mat[0][0]);
}

void Shader::UniformVec3f(const glm::vec3& v, const std::string& UniformName)
{
	Use();
	glUniform3f(GetUniformLocation(UniformName), v.x, v.y, v.z);
}

void Shader::UniformVec4f(const glm::vec4& v, const std::string& UniformName)
{
	Use();
	glUniform4f(GetUniformLocation(UniformName), v.x, v.y, v.z, v.a);
}

void Shader::Uniform1i(int i, const std::string& UniformName)
{
	Use();
	glUniform1i(GetUniformLocation(UniformName), i);
}

void Shader::Uniform1f(float i, const std::string& UniformName)
{
	Use();
	glUniform1f(GetUniformLocation(UniformName), i);
}

bool Shader::IsUniformDefined(const std::string& UniformName) const
{
	return (glGetUniformLocation(m_programID, UniformName.c_str()) != -1);
}

uint32_t Shader::GenUniformBuffer(const std::string& block_name, uint32_t size, uint32_t binding_point)
{
	Use();
	uint32_t& new_buffer = m_UniformBuffers.emplace_back();

	//Generate a buffer and allocate the desired space
	glGenBuffers(1, &new_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, new_buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

	//Bind the uniform block to the requested binding point...
	uint32_t block_index = glGetUniformBlockIndex(m_programID, block_name.c_str());
	glUniformBlockBinding(m_programID, block_index, binding_point);

	//And link the binding point to the GPU buffer
	glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, new_buffer, 0, size);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return m_UniformBuffers.size() - 1;
}

void Shader::BindUniformBuffer(uint32_t ub_local_index)
{
	uint32_t buf = m_UniformBuffers[ub_local_index];
	if (buf != s_CurrentlyBoundUniformBuffer)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBuffers[ub_local_index]);
		s_CurrentlyBoundUniformBuffer = buf;
	}
}

void Shader::DeleteUniformBuffers()
{
	for (uint32_t i = 0; i < m_UniformBuffers.size(); i++)
		glDeleteBuffers(1, &m_UniformBuffers[i]);

	m_UniformBuffers.clear();
}

void Shader::SendDataToUniformBuffer(uint32_t ub_local_index, uint32_t size, uint32_t offset, const void* data)
{
	BindUniformBuffer(ub_local_index);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void Shader::SetUniformBufferRange(uint32_t ub_local_index, uint32_t binding, uint32_t size, uint32_t offset)
{
	BindUniformBuffer(ub_local_index);
	glBindBufferRange(GL_UNIFORM_BUFFER, binding, m_UniformBuffers[ub_local_index], offset, size);
}

void Shader::ClearUniformCache()
{
	m_UniformCache.clear();
}

void Shader::LoadShadersFromFile(const std::string& File, std::string& vs, std::string& gs, std::string& fs)
{
	std::ifstream str(File);
	if (!str.is_open())
	{
		std::cout << "could not open the file; (function):" << __FUNCTION__
			<< "(line):" << __LINE__ << std::endl;
	}

	std::stringstream input[3];
	int index = -1;
	std::string line;
	while (std::getline(str, line))
	{
		if (line.find("#shader vertex") != std::string::npos)
		{
			index = 0;
			continue;
		}

		if (line.find("#shader geometry") != std::string::npos)
		{
			index = 1;
			continue;
		}

		if (line.find("#shader fragment") != std::string::npos)
		{
			index = 2;
			continue;
		}

		input[index] << line << "\n";

	}

	vs = input[0].str();
	gs = input[1].str();
	fs = input[2].str();
}

int32_t Shader::GetUniformLocation(const std::string& UniformName) const
{
	//Checking if the uniform is already stored in the cache
	if (m_UniformCache.find(UniformName) != m_UniformCache.end())
		return m_UniformCache[UniformName];

	int uniform = glGetUniformLocation(m_programID, UniformName.c_str());
	if (uniform == -1)
	{
		std::cout << GLError << "Uniform <" << UniformName <<"> was not found, please check string input\n";
		return -1;
	}

	m_UniformCache[UniformName] = uniform;
	return uniform;
}

int Shader::SetupShader(std::string& source, GLenum ShaderType)
{
	int s = glCreateShader(ShaderType);
	const char* strvs = source.c_str();
	glShaderSource(s, 1, &strvs, nullptr);
	glCompileShader(s);
	CheckShaderCompileStatus(s, ShaderType);

	return s;
}

void Shader::CheckShaderCompileStatus(uint32_t shader, GLenum ShaderType)
{
	int is_ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &is_ok);
	if (!is_ok)
	{
		char ErrorMsg[400];
		glGetShaderInfoLog(shader, 400, nullptr, ErrorMsg);
		
		std::string ShaderName;
		switch (ShaderType)
		{
		case GL_VERTEX_SHADER:
			ShaderName = "Vertex";
			break;
		case GL_GEOMETRY_SHADER:
			ShaderName = "Geometry";
			break;
		case GL_FRAGMENT_SHADER:
			ShaderName = "Fragment";
			break;
		}

		std::cout << "[OpenGL]: Error in " << ShaderName
			<< " Shader:\n" << ErrorMsg << std::endl;
	}
}
