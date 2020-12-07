#include "Renderer.h"

#include <algorithm>

namespace Renderer
{
	RenderResult::RenderResult(unsigned int width, unsigned int height)
		: m_width(width)
		, m_height(height)
		, m_frameBufferSize(0)
		, m_frameBuffer(nullptr)
	{
		m_frameBuffer = new unsigned char[width * height * 3];
		if (m_frameBuffer)
		{
			m_frameBufferSize = width * height * 3;
		}
	}

	RenderResult::RenderResult(RenderResult && other)
		: m_width(other.m_width)
		, m_height(other.m_height)
		, m_frameBufferSize(other.m_frameBufferSize)
		, m_frameBuffer(other.m_frameBuffer)
	{
		other.m_width = 0;
		other.m_height = 0;
		other.m_frameBufferSize = 0;
		other.m_frameBuffer = nullptr;
	}

	RenderResult & RenderResult::operator=(RenderResult && other)
	{
		new (this) RenderResult(std::move(other));
		return *this;
	}

	RenderResult::~RenderResult()
	{
		if (m_frameBuffer)
		{
			delete[] m_frameBuffer;
		}
	}

	unsigned int RenderResult::Width()
	{
		return m_width;
	}

	unsigned int RenderResult::Height()
	{
		return m_height;
	}

	unsigned int RenderResult::GetFrameBufferSize()
	{
		return m_frameBufferSize;
	}

	const void * RenderResult::GetFrameBuffer()
	{
		return m_frameBuffer;
	}

	void * RenderResult::FrameBuffer()
	{
		return m_frameBuffer;
	}
}