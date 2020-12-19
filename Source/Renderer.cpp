#include "Renderer.h"

#include <algorithm>
#include <cassert>
#include <malloc.h> // _aligned_malloc

namespace Renderer
{
	RenderResult::RenderResult(unsigned int width, unsigned int height)
		: m_frontId(0)
		, m_backId(1)
		, m_depthBuffer(width, height, 4, 4)
	{
		m_swapBuffer[ 0 ] = Buffer(width, height, 3);
		m_swapBuffer[ 1 ] = Buffer(width, height, 3);
	}

	void RenderResult::SwapBuffer()
	{
		std::swap(m_frontId, m_backId);
	}

	unsigned int RenderResult::Width()
	{
		return m_depthBuffer.Width();
	}

	unsigned int RenderResult::Height()
	{
		return m_depthBuffer.Height();
	}

	const void * RenderResult::GetFrontBuffer()
	{
		return FrontBuffer().Data();
	}

	const void * RenderResult::GetBackBuffer()
	{
		return BackBuffer().Data();
	}

	const void * RenderResult::GetDepthBuffer()
	{
		return DepthBuffer().Data();
	}

	Buffer & RenderResult::FrontBuffer()
	{
		return m_swapBuffer[m_frontId];
	}

	Buffer & RenderResult::BackBuffer()
	{
		return m_swapBuffer[m_backId];
	}

	Buffer & RenderResult::DepthBuffer()
	{
		return m_depthBuffer;
	}

	Buffer::Buffer(unsigned int width, unsigned int height, unsigned int elementSize, unsigned int alignment)
		: m_width(width)
		, m_height(height)
		, m_elementSize(elementSize)
		, m_sizeInBytes(0)
		, m_data(nullptr)
	{
		m_data = ( Byte * ) _aligned_malloc(width * height * elementSize, alignment);
		if ( m_data )
		{
			m_sizeInBytes = width * height * elementSize;
		}
	}

	Buffer::~Buffer()
	{
		if ( m_data )
		{
			_aligned_free(m_data);
		}
		m_data = nullptr;
	}

	Buffer::Buffer(Buffer && other)
		: m_width(other.m_width)
		, m_height(other.m_height)
		, m_elementSize(other.m_elementSize)
		, m_sizeInBytes(other.m_sizeInBytes)
		, m_data(other.m_data)
	{
		other.m_width = 0;
		other.m_height = 0;
		other.m_sizeInBytes = 0;
		other.m_data = nullptr;
	}

	Buffer & Buffer::operator=(Buffer && other)
	{
		this->~Buffer();
		new (this) Buffer(std::move(other));
		return *this;
	}

	void Buffer::SetAll(Byte value)
	{
		if ( m_data )
		{
			memset(m_data, value, m_sizeInBytes);
		}
	}

	void Buffer::Reshape(unsigned int width, unsigned int height)
	{
		assert(m_width * m_height <= width * height);
		m_width = width;
		m_height = height;
		m_sizeInBytes = m_width * m_height * m_elementSize;
	}

	unsigned int Buffer::Width() const
	{
		return m_width;
	}

	unsigned int Buffer::Height() const
	{
		return m_height;
	}

	unsigned int Buffer::SizeInBytes() const
	{
		return m_sizeInBytes;
	}

	unsigned int Buffer::ElementCount() const
	{
		return m_width * m_height;
	}

	unsigned int Buffer::ElementSize() const
	{
		return m_elementSize;
	}

	void * Buffer::Data()
	{
		return m_data;
	}

	void * Buffer::At(unsigned int row, unsigned int col)
	{
		assert(m_data);
		return m_data + (row * m_width + col) * m_elementSize;
	}

}