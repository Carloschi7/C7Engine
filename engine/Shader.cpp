#include "Shader.h"

uint32_t Shader::s_UniformBuffer = 0;

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
	glDeleteProgram(m_programID);
}

void Shader::Use() const
{
	int curr_prog;
	glGetIntegerv(GL_CURRENT_PROGRAM, &curr_prog);
	
	if(curr_prog != m_programID)
		glUseProgram(m_programID);
}

void Shader::UniformMat4f(const glm::mat4& mat, const std::string& UniformName)
{
	glUniformMatrix4fv(GetUniformLocation(UniformName), 1, GL_FALSE, &mat[0][0]);
}

void Shader::UniformVec3f(const glm::vec3& v, const std::string& UniformName)
{
	glUniform3f(GetUniformLocation(UniformName), v.x, v.y, v.z);
}

void Shader::UniformVec4f(const glm::vec4& v, const std::string& UniformName)
{
	glUniform4f(GetUniformLocation(UniformName), v.x, v.y, v.z, v.a);
}

void Shader::Uniform1i(int i, const std::string& UniformName)
{
	glUniform1i(GetUniformLocation(UniformName), i);
}

void Shader::Uniform1f(float i, const std::string& UniformName)
{
	glUniform1f(GetUniformLocation(UniformName), i);
}

bool Shader::IsUniformDefined(const std::string& UniformName) const
{
	return (glGetUniformLocation(m_programID, UniformName.c_str()) != -1);
}

void Shader::GenUniformBuffer(uint32_t size, uint32_t binding_point)
{
	if (s_UniformBuffer != 0)
		DeleteUniformBuffer();

	glGenBuffers(1, &s_UniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, s_UniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, s_UniformBuffer, 0, size);
}

void Shader::DeleteUniformBuffer()
{
	glDeleteBuffers(1, &s_UniformBuffer);
	s_UniformBuffer = 0;
}

void Shader::SendDataToUniformBuffer(uint32_t size, uint32_t offset, const void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, s_UniformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//Sets a binding point for an uniform block defined in the shader
void Shader::SetUniformBindingPoint(const std::string& UniformBlockName, uint32_t binding_point)
{
	Use();
	uint32_t block_adress = glGetUniformBlockIndex(m_programID, UniformBlockName.c_str());
	glUniformBlockBinding(m_programID, block_adress, binding_point);
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

int Shader::GetUniformLocation(const std::string& UniformName) const
{
	Use();
	int uniform = glGetUniformLocation(m_programID, UniformName.c_str());
	if (uniform == -1)
	{
		std::cout << GLError << "Uniform <" << UniformName <<"> was not found, please check string input\n";
		return -1;
	}

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
