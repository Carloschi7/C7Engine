#include "FrameBuffer.h"
#include "GL/glew.h"

FrameBuffer::FrameBuffer() : is_loaded{ false }, m_Type{ FrameBufferType::COLOR_ATTACHMENT },
	m_FrameBufferID{ 0 }, m_RenderBufferID{ 0 }, m_FrameBufferTextureID{ 0 }
{
}

FrameBuffer::FrameBuffer(u32 width, u32 height, FrameBufferType type) : FrameBuffer()
{
	Load(width, height, type);
}

void FrameBuffer::Load(u32 width, u32 height, FrameBufferType type)
{
	glGenFramebuffers(1, &m_FrameBufferID);
	glGenTextures(1, &m_FrameBufferTextureID);
	m_Type = type;

	switch (m_Type)
	{
	case FrameBufferType::COLOR_ATTACHMENT:
		
		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);

		//Setting up local framebuffer texture
		glBindTexture(GL_TEXTURE_2D, m_FrameBufferTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FrameBufferTextureID, 0);

		//Setting the renderbuffer for stencil/depth operations
		glGenRenderbuffers(1, &m_RenderBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferID);

		CheckFrameBufferStatus();

		glBindTexture(GL_TEXTURE_2D, 0);
		break;

	case FrameBufferType::DEPTH_ATTACHMENT:
		//Setting up the texture which will hold the depth values
		glBindTexture(GL_TEXTURE_2D, m_FrameBufferTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_FrameBufferTextureID, 0);
		//Manually disabling the option to draw colors
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		CheckFrameBufferStatus();

		glBindTexture(GL_TEXTURE_2D, 0);
		m_RenderBufferID = 0;
		break;

	case FrameBufferType::DEPTH_CUBEMAP_ATTACHMENT:
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_FrameBufferTextureID);
		
		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0,
				GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_FrameBufferTextureID, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		CheckFrameBufferStatus();

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		m_RenderBufferID = 0;
		break;
	}

	//Setting the drawing functions back to the main FrameBuffer (the screen by default)
	FrameBuffer::BindDefault();
	is_loaded = true;
}

FrameBuffer::FrameBuffer(FrameBuffer&& right) noexcept
{
	m_FrameBufferID = right.m_FrameBufferID;
	m_FrameBufferTextureID = right.m_FrameBufferTextureID;
	m_RenderBufferID = right.m_FrameBufferID;
	m_Type = right.m_Type;

	right.m_FrameBufferID = 0;
	right.m_FrameBufferTextureID = 0;
	right.m_RenderBufferID = 0;
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &m_FrameBufferID);
	if (m_RenderBufferID != 0) glDeleteRenderbuffers(1, &m_RenderBufferID);
	glDeleteTextures(1, &m_FrameBufferTextureID);
}

void FrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
}

void FrameBuffer::BindFrameTexture(u32 slot)
{
	GLenum texture_type = (m_Type == FrameBufferType::DEPTH_CUBEMAP_ATTACHMENT) ? 
		GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(texture_type, m_FrameBufferTextureID);
	
}

void FrameBuffer::BindDefault()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::CheckFrameBufferStatus() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID);
	GLenum n = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (n != GL_FRAMEBUFFER_COMPLETE) std::cout <<"Framebuffer setup error: " << n << std::endl;
}
