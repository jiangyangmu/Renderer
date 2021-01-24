#include "Buffer.h"

#include <Windows.h>
#include <malloc.h>
#include <memory.h>

namespace Graphics
{
	Buffer::Buffer()
		: m_width(0)
		, m_height(0)
		, m_alignment(0)
		, m_elementSize(0)
		, m_sizeInBytes(0)
		, m_rowSizeInBytes(0)
		, m_data(nullptr)
	{
	}
	Buffer::Buffer(Integer width, Integer height, Integer elementSize, Integer alignment, Integer rowPadding)
		: m_width(width)
		, m_height(height)
		, m_alignment(alignment)
		, m_elementSize(elementSize)
		, m_sizeInBytes(0)
		, m_rowSizeInBytes(0)
		, m_data(nullptr)
	{
		m_data = ( Byte * ) _aligned_malloc(height * (width * elementSize + rowPadding), alignment);
		if ( m_data )
		{
			m_rowSizeInBytes = width * elementSize + rowPadding;
			m_sizeInBytes = height * m_rowSizeInBytes;
			memset(m_data, 0, m_sizeInBytes);
		}
	}
	Buffer::Buffer(Buffer && other)
		: m_width(other.m_width)
		, m_height(other.m_height)
		, m_alignment(other.m_alignment)
		, m_elementSize(other.m_elementSize)
		, m_sizeInBytes(other.m_sizeInBytes)
		, m_rowSizeInBytes(other.m_rowSizeInBytes)
		, m_data(other.m_data)
	{
		new ( &other ) Buffer();
	}
	Buffer::~Buffer()
	{
		if ( m_data )
		{
			_aligned_free(m_data);
			m_data = nullptr;
		}
	}

	Buffer &	Buffer::operator=(Buffer && other)
	{
		this->~Buffer();
		new ( this ) Buffer(std::move(other));
		return *this;
	}

	bool		Buffer::QueryInterface(Integer guid, void ** ppInterface)
	{
		if ( _INTERFACE_IID(Buffer) == guid )
		{
			*ppInterface = this;
			return true;
		}
		else
		{
			return false;
		}
	}
	void		Buffer::SetAll(Byte value)
	{
		if ( m_data )
		{
			memset(m_data, value, m_sizeInBytes);
		}
	}
	Integer		Buffer::Width() const
	{
		return m_width;
	}
	Integer		Buffer::Height() const
	{
		return m_height;
	}
	Integer		Buffer::Alignment() const
	{
		return m_alignment;
	}
	Integer		Buffer::SizeInBytes() const
	{
		return m_sizeInBytes;
	}
	Integer		Buffer::RowSizeInBytes() const
	{
		return m_rowSizeInBytes;
	}
	Integer		Buffer::ElementCount() const
	{
		return m_width * m_height;
	}
	Integer		Buffer::ElementSize() const
	{
		return m_elementSize;
	}
	void *		Buffer::Data()
	{
		return m_data;
	}
	const void *	Buffer::Data() const
	{
		return m_data;
	}
}