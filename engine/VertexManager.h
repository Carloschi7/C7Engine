#pragma once
#include <iostream>
#include <vector>
#include <map>
#include "GL/glew.h"
#include "utils/types.h"

struct LayoutElement
{
	i32 count;
	GLenum type;
	GLboolean bNormalized;
	size_t stride;
	size_t offset;
};

//Example of an attribute structure => { 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 3, 0 }
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
	VertexManager(const f32* verts, size_t verts_size, const Layout& l);
	VertexManager(const f32* verts, size_t verts_size, const u32* indices, size_t indices_size,
		const Layout& l);
	VertexManager(VertexManager&& vm) noexcept;
	~VertexManager();
	void ReleaseResources();

	//Recommended for pushing initial static data
	void SendDataToOpenGLArray(const f32* verts, size_t verts_size, const Layout& l);
	void SendDataToOpenGLElements(const f32* verts, size_t verts_size, const u32* indices, size_t indices_size,
		const Layout& l);
	//Pushes a new attribute that will go alongside the static data. Can tweak the divisor to make one
	//attribute the same for the current drawcall(divisor_index = number of drawcalls that will us the attribute,
	//starting from the first one obviously)
	//Recommended for perframe-modified data
	u32 PushInstancedAttribute(const void* verts, size_t verts_size, u32 attr_index, const LayoutElement& el);
	//Very similar to PushInstanceAttribute but optimized for mat4 dynamic allocations
	u32 PushInstancedMatrixBuffer(const void* verts, size_t verts_size, u32 attr_index);
	//May require to wait until the gpu is done processing the info
	void EditInstance(u32 vb_local_index, const void* verts, size_t verts_size, size_t offset);
	void ClearBuffers();

	void BindVertexArray() const;
	bool IsLoaded() const { return m_SuccesfullyLoaded; };
	bool HasIndices() const { return m_HasIndices; }
	bool CheckStrideValidity(const Layout& l);

	f32* GetRawBuffer() const;
	f32* GetRawAttribute(u32 begin, u32 end) const;
	//Returns a mapped pointer of an instanced attribute
	void* InstancedAttributePointer(u32 buf_index);
	void UnmapAttributePointer(u32 buf_index);

public:
	u32 GetVertexArray() const { return m_VAO; }
	//For array instantiation it counts the number of vertices, while in the element one it marks the number of indices
	//(used for drawcalls mainly)
	u32 GetIndicesCount() const { return m_IndicesCount; }
	u32 GetAttribCount() const { return m_AttribCount; }
	u32 GetValuesCount() const { return m_ValuesCount; }
	size_t GetStrideLenght() const { return m_StrideLength; }
private:
	bool IsIntegerType(GLenum type) const;
	void VertexAttribPointer(u32 attr_index, const LayoutElement& el);
private:
	u32 m_VAO, m_VBO, m_EBO;
	u32 m_IndicesCount;
	u32 m_AttribCount;
	u32 m_ValuesCount;
	size_t m_StrideLength;
	bool m_SuccesfullyLoaded;
	bool m_HasIndices;

	//For extra instanced data
	std::vector<u32> m_AdditionalBuffers;
	std::map<u32, void*> m_BufferPointers;

	static u32 s_VaoBinding;
};