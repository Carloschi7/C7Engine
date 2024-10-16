#pragma once
#include <iostream>
#include "utils/types.h"

//Trying to implement a more functional-style library, so that the user if he wants is not
//locked in the FrameBuffer class environment

struct DoubleTextureFramebuffer
{
	u32 handle;
	u32 color_texture;
	u32 depth_texture;
};

namespace gfx {
	DoubleTextureFramebuffer create_framebuffer_texture_color_texture_depth(u32 width, u32 height);
	void destroy_framebuffer(DoubleTextureFramebuffer* fb);
}

enum class FrameBufferType
{
	COLOR_ATTACHMENT = 0, DEPTH_ATTACHMENT, COLOR_DEPTH_TEXTURE_ATTACHMENT, DEPTH_CUBEMAP_ATTACHMENT
};

class FrameBuffer
{
public:
	FrameBuffer();
	FrameBuffer(u32 width, u32 height, FrameBufferType type);
	FrameBuffer(FrameBuffer&& right) noexcept;
	~FrameBuffer();

	void Load(u32 width, u32 height, FrameBufferType type);
	void Bind();
	void BindFrameTexture(u32 slot = 0);
	void BindFrameDepthTexture(u32 slot = 0);
	static void BindDefault();
	inline bool IsLoaded() const { return is_loaded; }

private:
	void CheckFrameBufferStatus() const;

private:
	bool is_loaded;
	u32 m_FrameBufferID = 0;
	u32 m_RenderBufferID = 0;
	u32 m_FrameBufferColorTextureID = 0;
	u32 m_FrameBufferDepthTextureID = 0;
	FrameBufferType m_Type;
};