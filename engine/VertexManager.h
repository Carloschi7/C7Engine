#pragma once
#include <iostream>
#include <vector>
#include "GL/glew.h"

struct LayoutElement
{
	int32_t count;
	GLenum type;
	GLboolean bNormalized;
	size_t stride;
	size_t offset;
};

class Layout
{
public:
	Layout() {}
	Layout(const std::vector<LayoutElement>& el) { vec = el; }

	void PushAttribute(const LayoutElement& attr) { vec.push_back(attr); }
	void PopAttribute() { vec.pop_back(); }
	const std::vector<LayoutElement>& GetAttributes() const { return vec; }
private:
	std::vector<LayoutElement> vec;
};

class VertexManager
{
public:
	VertexManager();
	VertexManager(const float* verts, size_t verts_size, const Layout& l);
	VertexManager(const float* verts, size_t verts_size, const uint32_t* indices, size_t indices_size,
		const Layout& l);
	VertexManager(VertexManager&& vm) noexcept;
	~VertexManager();
	void ReleaseResources();

	//Recommended for pushing initial static data
	void SendDataToOpenGLArray(const float* verts, size_t verts_size, const Layout& l);
	void SendDataToOpenGLElements(const float* verts, size_t verts_size, const uint32_t* indices, size_t indices_size,
		const Layout& l);
	//Pushes a new attribute that will go alongside the static data. Can tweak the divisor to make one
	//attribute the same for the current drawcall(divisor_index = number of drawcalls that will us the attribute,
	//starting from the first one obviously)
	//Recommended for perframe-modified data
	uint32_t PushInstancedAttribute(const void* verts, size_t verts_size, uint32_t attr_index, const LayoutElement& el);
	//Very similar to PushInstanceAttribute but optimized for mat4 dynamic allocations
	uint32_t PushInstancedMatrixBuffer(const void* verts, size_t verts_size, uint32_t attr_index);
	void EditInstance(uint32_t vb_local_index, const void* verts, size_t verts_size, size_t offset);
	void ClearBuffers();

	void BindVertexArray() const;
	bool IsLoaded() const { return m_SuccesfullyLoaded; };
	bool HasIndices() const { return m_HasIndices; }
	bool CheckStrideValidity(const Layout& l);

	float* GetRawBuffer() const;
	float* GetRawAttribute(uint32_t begin, uint32_t end) const;

public:
	uint32_t GetVertexArray() const { return m_VAO; }
	//For array instantiation it counts the number of vertices, while in the element one it marks the number of indices
	//(used for drawcalls mainly)
	uint32_t GetIndicesCount() const { return m_IndicesCount; }
	uint32_t GetAttribCount() const { return m_AttribCount; }
	uint32_t GetValuesCount() const { return m_ValuesCount; }
	size_t GetStrideLenght() const { return m_StrideLength; }
private:
	bool IsIntegerType(GLenum type) const;
	void VertexAttribPointer(uint32_t attr_index, const LayoutElement& el);
private:
	uint32_t m_VAO, m_VBO, m_EBO;
	uint32_t m_IndicesCount;
	uint32_t m_AttribCount;
	uint32_t m_ValuesCount;
	size_t m_StrideLength;
	bool m_SuccesfullyLoaded;
	bool m_HasIndices;

	//For extra instanced data
	std::vector<uint32_t> m_AdditionalBuffers;

	static uint32_t s_VaoBinding;
};