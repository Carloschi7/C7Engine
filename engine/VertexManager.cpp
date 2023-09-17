#include "VertexManager.h"
#include <utility>
#include <cstring>

u32 VertexManager::s_VaoBinding = 0;

VertexManager::VertexManager()
	:m_IndicesCount(0), m_AttribCount(0), m_SuccesfullyLoaded(false), m_HasIndices(false), m_ValuesCount(0),
	m_StrideLength(0)
{
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glGenVertexArrays(1, &m_VAO);
}

VertexManager::VertexManager(const float* verts, size_t verts_size, const Layout& l)
	:VertexManager()
{
	SendDataToOpenGLArray(verts, verts_size, l);
}

VertexManager::VertexManager(const float* verts, size_t verts_size, const u32* indices, size_t indices_size,
	const Layout& l)
	: VertexManager()
{
	SendDataToOpenGLElements(verts, verts_size, indices, indices_size, l);
}

VertexManager::VertexManager(VertexManager&& vm) noexcept :
	m_VAO(std::exchange(vm.m_VAO, 0)),
	m_VBO(std::exchange(vm.m_VBO, 0)),
	m_EBO(std::exchange(vm.m_EBO, 0)),
	m_IndicesCount(std::exchange(vm.m_IndicesCount, 0)),
	m_AttribCount(std::exchange(vm.m_AttribCount, 0)),
	m_HasIndices(std::exchange(vm.m_HasIndices, 0)),
	m_SuccesfullyLoaded(std::exchange(vm.m_SuccesfullyLoaded, 0)),
	m_ValuesCount(std::exchange(vm.m_ValuesCount, 0)),
	m_StrideLength(std::exchange(vm.m_StrideLength, 0)),
	m_AdditionalBuffers(std::move(vm.m_AdditionalBuffers))
{
}

VertexManager::~VertexManager()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VAO);

	for (u32 i : m_AdditionalBuffers)
		glDeleteBuffers(1, &i);
}

void VertexManager::ReleaseResources()
{
	std::memset(this, 0, sizeof(VertexManager));
}

void VertexManager::SendDataToOpenGLArray(const float* verts, size_t verts_size, const Layout& l)
{
	BindVertexArray();

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_STATIC_DRAW);

	if (!CheckStrideValidity(l))
	{
		std::cout << "Invalid layout strides" << std::endl;
		this->~VertexManager();
		return;
	}

	for (size_t i = 0; i < l.GetAttributes().size(); i++)
	{
		const auto& attr = l.GetAttributes()[i];
		glEnableVertexAttribArray(i);
		VertexAttribPointer(i, attr);
	}

	m_IndicesCount = verts_size / l.GetAttributes()[0].stride;
	m_AttribCount = l.GetAttributes().size();
	m_HasIndices = false;
	m_SuccesfullyLoaded = true;
	m_ValuesCount = verts_size / sizeof(float);
	m_StrideLength = l.GetAttributes()[0].stride / sizeof(float);
}

void VertexManager::SendDataToOpenGLElements(const float* verts, size_t verts_size, const u32* indices, size_t indices_size,
	const Layout& l)
{
	BindVertexArray();

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);

	if (!CheckStrideValidity(l))
	{
		std::cout << "Invalid layout strides" << std::endl;
		this->~VertexManager();
		return;
	}

	for (size_t i = 0; i < l.GetAttributes().size(); i++)
	{
		const auto& attr = l.GetAttributes()[i];
		glEnableVertexAttribArray(i);
		VertexAttribPointer(i, attr);
	}

	m_IndicesCount = indices_size / sizeof(u32);
	m_AttribCount = l.GetAttributes().size();
	m_HasIndices = true;
	m_SuccesfullyLoaded = true;
	m_ValuesCount = verts_size / sizeof(float);
	m_StrideLength = l.GetAttributes()[0].stride / sizeof(float);
}


u32 VertexManager::PushInstancedAttribute(const void* verts, size_t verts_size, u32 attr_index, const LayoutElement& el)
{
	if (attr_index == static_cast<u32>(-1))
		return static_cast<u32>(-1);

	BindVertexArray();

	u32& buffer = m_AdditionalBuffers.emplace_back();
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_DYNAMIC_DRAW);

	//Map the attribute
	glEnableVertexAttribArray(attr_index);
	VertexAttribPointer(attr_index, el);
	glVertexAttribDivisor(attr_index, 1);

	return buffer;
}

u32 VertexManager::PushInstancedMatrixBuffer(const void* verts, size_t verts_size, u32 attr_index)
{
	if (attr_index == static_cast<u32>(-1))
		return static_cast<u32>(-1);

	BindVertexArray();

	u32& buffer = m_AdditionalBuffers.emplace_back();
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_DYNAMIC_DRAW);

	//We cant uniform a matrix all at once, we need to split it in 4 vec4
	//A mat4 occupies 4 attribute indexes in the shader, each one containing a vec4
	for (u32 i = 0; i < 4; i++)
	{
		glEnableVertexAttribArray(attr_index + i);
		glVertexAttribPointer(attr_index + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 4 * i));
		//One matrix per drawn instance
		glVertexAttribDivisor(attr_index + i, 1);
	}

	return buffer;
}

void VertexManager::EditInstance(u32 vb_local_index, const void* verts, size_t verts_size, size_t offset)
{
	if (vb_local_index >= m_AdditionalBuffers.size())
		return;

	glBindBuffer(GL_ARRAY_BUFFER, m_AdditionalBuffers[vb_local_index]);
	void* my_buf = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	if (offset == 0)
	{
		std::memcpy(my_buf, verts, verts_size);
	}
	else
	{
		const void* buf = static_cast<const u8*>(verts) + offset;
		std::memcpy(my_buf, buf, verts_size);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void VertexManager::ClearBuffers()
{
	glInvalidateBufferData(m_VBO);
	glInvalidateBufferData(m_EBO);
}

void VertexManager::BindVertexArray() const
{
	if (s_VaoBinding != m_VAO)
	{
		glBindVertexArray(m_VAO);
		s_VaoBinding = m_VAO;
	}
}

bool VertexManager::CheckStrideValidity(const Layout& l)
{
	if (l.GetAttributes().empty()) return false;

	size_t val = l.GetAttributes()[0].stride;
	for (int i = 1; i < l.GetAttributes().size(); i++)
	{
		if (l.GetAttributes()[i].stride != val)
			return false;
	}

	return true;
}

//Returns a copy
float* VertexManager::GetRawBuffer() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	float* ret = (float*)::operator new(m_ValuesCount * sizeof(float));

	memcpy(ret, data, m_ValuesCount * sizeof(float));

	glUnmapBuffer(GL_ARRAY_BUFFER);
	return ret;
}

//Must be deallocated manually
float* VertexManager::GetRawAttribute(u32 begin, u32 end) const
{
	if (begin >= m_StrideLength || end >= m_StrideLength || end <= begin) return nullptr;

	float* ptr = GetRawBuffer();
	int elem_count = end - begin;
	float* res = (float*)::operator new((m_ValuesCount / m_StrideLength) * elem_count * sizeof(float));

	for (int i = begin, j = 0; i < m_ValuesCount - (m_StrideLength - end); i += m_StrideLength)
	{
		for (int k = i; k < i + elem_count; k++, j++)
		{
			//k = index of the raw buffer array, j = index simply incremented for the return pointer
			res[j] = ptr[k];
		}
	}

	::operator delete(ptr);
	return res;
}

void* VertexManager::InstancedAttributePointer(u32 buf_index)
{
	if (buf_index >= m_AdditionalBuffers.size())
		return nullptr;

	auto iter = m_BufferPointers.find(buf_index);
	if (iter != m_BufferPointers.end())
		return iter->second;

	glBindBuffer(GL_ARRAY_BUFFER, m_AdditionalBuffers[buf_index]);
	void* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	m_BufferPointers.insert({ buf_index, buf });
	return buf;
}

void VertexManager::UnmapAttributePointer(u32 buf_index)
{
	//Already unmapped
	auto iter = m_BufferPointers.find(buf_index);
	if (m_BufferPointers.find(buf_index) == m_BufferPointers.end())
		return;

	glBindBuffer(GL_ARRAY_BUFFER, m_AdditionalBuffers[buf_index]);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	m_BufferPointers.erase(iter);
}

bool VertexManager::IsIntegerType(GLenum type) const
{
	return type == GL_BYTE || type == GL_UNSIGNED_BYTE || type == GL_SHORT ||
		type == GL_UNSIGNED_SHORT || type == GL_INT || type == GL_UNSIGNED_INT;
}

void VertexManager::VertexAttribPointer(u32 attr_index, const LayoutElement& el)
{
	if (IsIntegerType(el.type))
		glVertexAttribIPointer(attr_index, el.count, el.type, el.stride, (void*)el.offset);
	else
		glVertexAttribPointer(attr_index, el.count, el.type, el.bNormalized, el.stride, (void*)el.offset);
}


