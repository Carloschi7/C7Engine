#pragma once
#include <iostream>
#include "utils/types.h"

enum class FrameBufferType
{
	COLOR_ATTACHMENT = 0, DEPTH_ATTACHMENT, DEPTH_CUBEMAP_ATTACHMENT
};

class FrameBuffer
{
public:
	FrameBuffer(u32 width, u32 height, FrameBufferType type);
	FrameBuffer(FrameBuffer&& right) noexcept;
	~FrameBuffer();

	void Bind();
	void BindFrameTexture(u32 slot = 0);
	static void BindDefault();

private:
	void CheckFrameBufferStatus() const;

private:
	u32 m_FrameBufferID;
	u32 m_RenderBufferID;
	u32 m_FrameBufferTextureID;
	FrameBufferType m_Type;
};