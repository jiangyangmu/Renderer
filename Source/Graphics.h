#pragma once

#include "Common.h"
#include "Event.h"

#include <functional>
#include <memory>
#include <vector>

namespace Graphics
{
	class RenderWindow;

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

	class Texture2D1
	{
	public:
		// Constructors
		
		Texture2D1();
		Texture2D1(const Buffer & bitmap);

		Texture2D1(const Texture2D1 &) = default;
		Texture2D1(Texture2D1 &&) = default;
		Texture2D1 & operator = (const Texture2D1 &) = default;
		Texture2D1 & operator = (Texture2D1 &&) = default;

		// Operations

		inline void Sample(float u, float v, float * rgb) const
		{
			ASSERT(0.0f <= u && u <= 1.0001f);
			ASSERT(0.0f <= v && v <= 1.0001f);
			ASSERT(u + v <= 2.0f);

			LONG width = m_bitmap->Width();
			LONG height = m_bitmap->Height();

			LONG col = static_cast< LONG >( width * u );
			LONG row = static_cast< LONG >( height * v );

			const Byte * bgra = ( Byte * ) m_bitmap->At(row, col);
			rgb[ 0 ] = static_cast< float >( bgra[ 2 ] ) / 255.f;
			rgb[ 1 ] = static_cast< float >( bgra[ 1 ] ) / 255.f;
			rgb[ 2 ] = static_cast< float >( bgra[ 0 ] ) / 255.f;
		}

	private:
		const Buffer *	m_bitmap;
	};

	namespace Materials
	{
		struct Ambient
		{
			RGB color;
		};
		struct Diffuse
		{
			RGB color;
		};
		struct Specular
		{
			RGB color;
		};
		struct BlinnPhong
		{
			Ambient ambient;
			Diffuse diffuse;
			Specular specular;
			float ambientRatio;
			float diffuseRatio;
			float specularRatio;
		};
	}
	namespace Lights
	{
		struct Light
		{
			RGB color;
			Vec3 pos;
			Vec3 attenuation;
		};
		struct Sphere
		{
			Light light;
			float radius;
		};
		struct Plane
		{
			Light light;
		};
	}
}
