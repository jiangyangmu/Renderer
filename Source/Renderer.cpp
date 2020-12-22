#include "Renderer.h"
#include "Common.h"
#include "win32/Win32App.h"

#include <algorithm>
#include <malloc.h> // _aligned_malloc

namespace Rendering
{
	HardcodedRenderer::HardcodedRenderer(unsigned int width, unsigned int height)
		: m_frontId(0)
		, m_backId(1)
		, m_depthBuffer(width, height, 4, 4)
	{
		m_swapBuffer[ 0 ] = Buffer(width, height, 3);
		m_swapBuffer[ 1 ] = Buffer(width, height, 3);
	}

	void HardcodedRenderer::SwapBuffer()
	{
		std::swap(m_frontId, m_backId);
	}

	unsigned int HardcodedRenderer::Width()
	{
		return m_depthBuffer.Width();
	}

	unsigned int HardcodedRenderer::Height()
	{
		return m_depthBuffer.Height();
	}

	const void * HardcodedRenderer::GetFrontBuffer()
	{
		return FrontBuffer().Data();
	}

	const void * HardcodedRenderer::GetBackBuffer()
	{
		return BackBuffer().Data();
	}

	const void * HardcodedRenderer::GetDepthBuffer()
	{
		return DepthBuffer().Data();
	}

	Buffer & HardcodedRenderer::FrontBuffer()
	{
		return m_swapBuffer[m_frontId];
	}

	Buffer & HardcodedRenderer::BackBuffer()
	{
		return m_swapBuffer[m_backId];
	}

	Buffer & HardcodedRenderer::DepthBuffer()
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
		ASSERT(m_width * m_height <= width * height);
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
		ASSERT(m_data);
		return m_data + (row * m_width + col) * m_elementSize;
	}

	Texture2D Texture2D::FromBitmap(const win32::Bitmap * bitmap)
	{
		Texture2D texture2D;
		texture2D.m_bitmap = bitmap;
		return texture2D;
	}

	void Texture2D::Sample(float u, float v, float * rgb) const
	{
		ASSERT(0.0f <= u && u <= 1.0f);
		ASSERT(0.0f <= v && v <= 1.0f);
		ASSERT(u + v <= 2.0f);

		LONG width = m_bitmap->GetWidth();
		LONG height = m_bitmap->GetHeight();

		LONG col = static_cast< LONG >( width * u );
		LONG row = static_cast< LONG >( height * (1.0f - v) );

		DWORD bgra = m_bitmap->GetPixel(col, row);

		BYTE * p = (BYTE *)&bgra;
		rgb[0] = static_cast< float >( p[2] ) / 255.f;
		rgb[1] = static_cast< float >( p[1] ) / 255.f;
		rgb[2] = static_cast< float >( p[0] ) / 255.f;
	}

}