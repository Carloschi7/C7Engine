#pragma once
#include <iostream>

enum class FrameBufferType
{
	COLOR_ATTACHMENT = 0, DEPTH_ATTACHMENT, DEPTH_CUBEMAP_ATTACHMENT
};

class FrameBuffer
{
public:
	FrameBuffer(uint32_t width, uint32_t height, FrameBufferType type);
	FrameBuffer(FrameBuffer&& right) noexcept;
	~FrameBuffer();

	void Bind();
	void BindFrameTexture(uint32_t slot = 0);
	static void BindDefault();

private:
	void CheckFrameBufferStatus() const;

private:
	uint32_t m_FrameBufferID;
	uint32_t m_RenderBufferID;
	uint32_t m_FrameBufferTextureID;
	FrameBufferType m_Type;
};