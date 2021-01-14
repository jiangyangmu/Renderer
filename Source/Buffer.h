#pragma once

#include "Common.h"
#include "Event.h"

namespace Graphics
{
	class Buffer
	{
	public:
		Buffer();
		Buffer(Integer width, Integer height, Integer elementSize, Integer alignment = 1, Integer rowPadding = 0);
		~Buffer();

		Buffer(const Buffer &) = delete;
		Buffer(Buffer && other);
		Buffer & operator = (const Buffer &) = delete;
		Buffer & operator = (Buffer && other);

		// Operations

		void		SetAll(Byte value);
		template <typename T>
		void		SetAllAs(T value)
		{
			ASSERT(sizeof(T) == m_elementSize);
			Byte * pRow = m_data;
			T * pData;
			for (Integer r = 0; r < m_height; ++r)
			{
				pData = reinterpret_cast<T *>(pRow);
				for (Integer c = 0; c < m_width; ++c)
				{
					*pData++ = value;
				}
				pRow += m_rowSizeInBytes;
			}
		}
		// TODO: resize (keep properties)

		// Properties

		Integer		Width() const;
		Integer		Height() const;
		Integer		Alignment() const;
		Integer		SizeInBytes() const;
		Integer		RowSizeInBytes() const;
		Integer		ElementCount() const;
		Integer		ElementSize() const;
		const void *	Data() const;
		void *		Data();
		const void *	At(Integer row, Integer col) const
		{
			ASSERT(m_data);
			return m_data + row * m_rowSizeInBytes + col * m_elementSize;
		}
		void *		At(Integer row, Integer col)
		{
			ASSERT(m_data);
			return m_data + row * m_rowSizeInBytes + col * m_elementSize;
		}


	private:
		Integer		m_width;
		Integer		m_height;
		Integer		m_alignment;
		Integer		m_elementSize;
		Integer		m_sizeInBytes;
		Integer		m_rowSizeInBytes;
		Byte *		m_data;
	};
}
