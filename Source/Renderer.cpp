#include "Renderer.h"

#include <algorithm>
#include <cassert>
#include <malloc.h> // _aligned_malloc

#include <Windows.h>
#include <Gdiplus.h> // Gdiplus::Bitmap

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

	Texture2D::Texture2D()
		: m_bitmap(nullptr)
	{
	}

	Texture2D::~Texture2D()
	{
		if (m_bitmap)
		{
			delete m_bitmap;
			m_bitmap = nullptr;
		}
	}

	void Texture2D::LoadFromFile(std::wstring filePath)
	{
		if (!m_bitmap)
		{
			m_bitmap = Gdiplus::Bitmap::FromFile(filePath.c_str(), false);
			assert(m_bitmap);
		}
	}

	void Texture2D::Sample(float u, float v, float * r, float * g, float * b)
	{
		assert(0.0f <= u && u <= 1.0f);
		assert(0.0f <= v && v <= 1.0f);
		assert(u + v <= 2.0f);

		LONG width = m_bitmap->GetWidth();
		LONG height = m_bitmap->GetHeight();

		LONG col = static_cast< LONG >( width * u );
		LONG row = static_cast< LONG >( height * (1.0f - v) );

		Gdiplus::Color color;
		
		if (m_bitmap->GetPixel(col, row, &color) == Gdiplus::Ok)
		{
			*r = static_cast< float >( color.GetR() ) / 255.f;
			*g = static_cast< float >( color.GetG() ) / 255.f;
			*b = static_cast< float >( color.GetB() ) / 255.f;
		}
	}

}