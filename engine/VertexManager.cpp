#include "VertexManager.h"
#include <cstring>

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

VertexManager::VertexManager(const float* verts, size_t verts_size, const uint32_t* indices, size_t indices_size,
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
	m_StrideLength(std::exchange(vm.m_StrideLength, 0))
{
}

VertexManager::~VertexManager()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
	glDeleteBuffers(1, &m_VAO);
}

void VertexManager::SendDataToOpenGLArray(const float* verts, size_t verts_size, const Layout& l)
{
	glBindVertexArray(m_VAO);

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
		glVertexAttribPointer(i, attr.count, attr.type, attr.bNormalized, attr.stride,
			(void*)attr.offset);
	}

	m_IndicesCount = verts_size / l.GetAttributes()[0].stride;
	m_AttribCount = l.GetAttributes().size();
	m_HasIndices = false;
	m_SuccesfullyLoaded = true;
	m_ValuesCount = verts_size / sizeof(float);
	m_StrideLength = l.GetAttributes()[0].stride / sizeof(float);
}

void VertexManager::SendDataToOpenGLElements(const float* verts, size_t verts_size, const uint32_t* indices, size_t indices_size,
	const Layout& l)
{
	glBindVertexArray(m_VAO);

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
		glVertexAttribPointer(i, attr.count, attr.type, attr.bNormalized, attr.stride,
			(void*)attr.offset);
	}

	m_IndicesCount = indices_size / sizeof(uint32_t);
	m_AttribCount = l.GetAttributes().size();
	m_HasIndices = true;
	m_SuccesfullyLoaded = true;
	m_ValuesCount = verts_size / sizeof(float);
	m_StrideLength = l.GetAttributes()[0].stride / sizeof(float);
}

void VertexManager::ClearBuffers()
{
	glInvalidateBufferData(m_VBO);
	glInvalidateBufferData(m_EBO);
}

void VertexManager::BindVertexArray() const
{
	glBindVertexArray(m_VAO);
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

//Must be deallocated manually
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
float* VertexManager::GetRawAttribute(uint32_t begin, uint32_t end) const
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


