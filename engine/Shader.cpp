#include "Shader.h"

u64 simple_string_hash(const char* string)
{
	u64 hash = 5381;
	u32 get_c_string_length_no_null_terminating(const char*);

	u32 string_length = get_c_string_length_no_null_terminating(string);
	for(u32 i = 0; i < string_length; i++) {
		hash = ((hash) << 5) + hash + string[i];
	}

	return hash;
}

Shader::Shader() : is_loaded(false), m_programID(-1)
{
}
Shader::Shader(const std::string& filepath)
{
	Load(filepath.c_str());
}

Shader::Shader(Shader&& shd) noexcept :
	m_UniformBuffers(std::move(shd.m_UniformBuffers)),
	m_UniformCache(std::move(shd.m_UniformCache))
{
	this->is_loaded = shd.is_loaded;
	this->m_programID = shd.m_programID;
	shd.m_programID = 0;
}

Shader::~Shader()
{
	DeleteUniformBuffers();
	glDeleteProgram(m_programID);
}


void Shader::Load(const char* filepath)
{
	m_programID = glCreateProgram();

	u32 vs, gs, fs;
	auto shader_source = LoadShadersFromFile(filepath);
	is_loaded = shader_source.initialized;

	if(!is_loaded) return;

	bool geometry_shader_defined = !shader_source.geometry_shader_source.empty();

	vs = SetupShader(shader_source.vertex_shader_source, GL_VERTEX_SHADER);
	if (geometry_shader_defined) gs = SetupShader(shader_source.geometry_shader_source, GL_GEOMETRY_SHADER);
	fs = SetupShader(shader_source.fragment_shader_source, GL_FRAGMENT_SHADER);

	glAttachShader(m_programID, vs);
	if (geometry_shader_defined) glAttachShader(m_programID, gs);
	glAttachShader(m_programID, fs);

	glLinkProgram(m_programID);
	glValidateProgram(m_programID);

	glDeleteShader(vs);
	if (geometry_shader_defined) glDeleteShader(gs);
	glDeleteShader(fs);
}

void Shader::Use() const
{
	extern std::atomic<u32> currently_bound_program;
	assert(is_loaded);
	if (m_programID != currently_bound_program)
	{
		glUseProgram(m_programID);
		currently_bound_program = m_programID;
	}
}

void Shader::UniformMat4f(const glm::mat4& mat, const char* uniform_name)
{
	Use();
	glUniformMatrix4fv(GetUniformLocation(uniform_name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::UniformVec2f(const glm::vec2& v, const char* uniform_name)
{
	Use();
	glUniform2f(GetUniformLocation(uniform_name), v.x, v.y);
}

void Shader::UniformVec3f(const glm::vec3& v, const char* uniform_name)
{
	Use();
	glUniform3f(GetUniformLocation(uniform_name), v.x, v.y, v.z);
}

void Shader::UniformVec4f(const glm::vec4& v, const char* uniform_name)
{
	Use();
	glUniform4f(GetUniformLocation(uniform_name), v.x, v.y, v.z, v.a);
}

void Shader::Uniform1i(int i, const char* uniform_name)
{
	Use();
	glUniform1i(GetUniformLocation(uniform_name), i);
}

void Shader::Uniform1f(f32 i, const char* uniform_name)
{
	Use();
	glUniform1f(GetUniformLocation(uniform_name), i);
}

bool Shader::IsUniformDefined(const char* uniform_name) const
{
	return (glGetUniformLocation(m_programID, uniform_name) != -1);
}

u32 Shader::GenUniformBuffer(const char* block_name, u32 size, u32 binding_point)
{
	Use();
	u32& new_buffer = m_UniformBuffers.emplace_back();

	//Generate a buffer and mem_allocate the desired space
	glGenBuffers(1, &new_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, new_buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

	//Bind the uniform block to the requested binding point...
	u32 block_index = glGetUniformBlockIndex(m_programID, block_name);
	glUniformBlockBinding(m_programID, block_index, binding_point);

	//And link the binding point to the GPU buffer
	glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, new_buffer, 0, size);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return m_UniformBuffers.size() - 1;
}

s32 Shader::GetAttributeLocation(const char* attr_name)
{
	Use();
	return glGetAttribLocation(m_programID, attr_name);
}

void Shader::BindUniformBuffer(u32 ub_local_index)
{
	extern std::atomic<u32> currently_bound_uniform_buffer;
	u32 buf = m_UniformBuffers[ub_local_index];
	if (buf != currently_bound_uniform_buffer)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBuffers[ub_local_index]);
		currently_bound_uniform_buffer = buf;
	}
}

void Shader::DeleteUniformBuffers()
{
	for (u32 i = 0; i < m_UniformBuffers.size(); i++)
		glDeleteBuffers(1, &m_UniformBuffers[i]);

	m_UniformBuffers.clear();
}

void Shader::SendDataToUniformBuffer(u32 ub_local_index, u32 size, u32 offset, const void* data)
{
	BindUniformBuffer(ub_local_index);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void Shader::SetUniformBufferRange(u32 ub_local_index, u32 binding, u32 size, u32 offset)
{
	BindUniformBuffer(ub_local_index);
	glBindBufferRange(GL_UNIFORM_BUFFER, binding, m_UniformBuffers[ub_local_index], offset, size);
}

void Shader::ClearUniformCache()
{
	m_UniformCache.clear();
}

ShaderSource Shader::LoadShadersFromFile(const char* file)
{
    ShaderSource shader_source = {};
    FILE* file_handle = nullptr;
    fopen_s(&file_handle, file, "r");

    if(!file_handle){
        shader_source.initialized = false;
        return shader_source;
    }

    const u32 max_chars_on_the_same_line = 512;

    auto get_next_line = [](FILE* file, char* buffer) ->FileParseResult {
        char ch;
        FileParseResult res = {};
        while((ch = fgetc(file)) != '\n') {
            if(ch == EOF) {
                res.is_eof = true;
                break;
            }

            assert(res.index < max_chars_on_the_same_line, "shader lines are too long");
            buffer[res.index++] = ch;
        }

        return res;
    };

    auto strings_match = [](const char* str1, const char* str2, u32 size) -> bool {
        for(u32 i = 0; i < size; i++) {
            if(str1[i] != str2[i]) {
                return false;
            }
        }

        return true;
    };

    char current_line[max_chars_on_the_same_line] = {};
    std::string* current_shader_source = nullptr;
    FileParseResult parse_result = {};

    char vertex_shader_signature[]   = "#shader vertex";
    char geometry_shader_signature[] = "#shader geometry";
    char fragment_shader_signature[] = "#shader fragment";

    while(!parse_result.is_eof) {
        parse_result = get_next_line(file_handle, current_line);

        if(strings_match(current_line, vertex_shader_signature, sizeof(vertex_shader_signature) - 1)) {
            current_shader_source = &shader_source.vertex_shader_source;
            continue;
        }
        if(strings_match(current_line, geometry_shader_signature, sizeof(geometry_shader_signature) - 1)) {
            current_shader_source = &shader_source.geometry_shader_source;
            continue;
        }
        if(strings_match(current_line, fragment_shader_signature, sizeof(fragment_shader_signature) - 1)) {
            current_shader_source = &shader_source.fragment_shader_source;
            continue;
        }

		//If the pointer is still not defined, no shader implementation has started just yet, probably some
		//comments or blank lines have been left, so keep iterating forward
		//TODO @C7 test to see if this does not cause issues
		if(!current_shader_source)
			continue;

        if(parse_result.index != 0) {
            current_shader_source->append(current_line, parse_result.index);
            //We dont wanna write the \0 aswell otherwise OpenGL will load the shader improperly
            current_shader_source->append("\n", 1);
        }
    }

    shader_source.initialized = true;
    return shader_source;
}

s32 Shader::GetUniformLocation(const char* uniform_name) const
{
	//Checking if the uniform is already stored in the cache

	u64 uniform_hash = simple_string_hash(uniform_name);
	if (m_UniformCache.find(uniform_hash) != m_UniformCache.end())
		return m_UniformCache[uniform_hash];

	int uniform = glGetUniformLocation(m_programID, uniform_name);
	if (uniform == -1)
	{
		std::cout << GLError << "Uniform <" << uniform_name <<"> was not found, please check string input\n";
		return -1;
	}

	m_UniformCache[uniform_hash] = uniform;
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

void Shader::CheckShaderCompileStatus(u32 shader, GLenum ShaderType)
{
	int is_ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &is_ok);
	if (!is_ok)
	{
		char ErrorMsg[400];
		glGetShaderInfoLog(shader, 400, nullptr, ErrorMsg);

		u32 shader_name_index = 0;
		const char* shader_names[3] = {
			"vertex",
			"geometry",
			"fragment"
		};

		switch (ShaderType)
		{
		case GL_VERTEX_SHADER:
			shader_name_index = 0;
			break;
		case GL_GEOMETRY_SHADER:
			shader_name_index = 1;
			break;
		case GL_FRAGMENT_SHADER:
			shader_name_index = 2;
			break;
		}

		std::cout << "[OpenGL]: Error in " << shader_names[shader_name_index]
			<< " Shader:\n" << ErrorMsg << std::endl;
	}
}
