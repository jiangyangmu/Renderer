#pragma once

#include <memory>

namespace win32
{
	class Bitmap;
}

namespace Rendering
{
	typedef unsigned char Byte;

	class Buffer
	{
	public:
		Buffer() = default;
		Buffer(unsigned int width, unsigned int height, unsigned int elementSize, unsigned int alignment = 1);
		~Buffer();

		Buffer(const Buffer &) = delete;
		Buffer(Buffer && other);
		Buffer & operator = (const Buffer &) = delete;
		Buffer & operator = (Buffer && other);

		// Operations

		void		SetAll(Byte value);
		void		Reshape(unsigned int width, unsigned int height);

		// Properties

		unsigned int	Width() const;
		unsigned int	Height() const;
		unsigned int	SizeInBytes() const;
		unsigned int	ElementCount() const;
		unsigned int	ElementSize() const;
		void *		Data();
		void *		At(unsigned int row, unsigned int col);

	private:
		unsigned int	m_width;
		unsigned int	m_height;
		unsigned int	m_elementSize;
		unsigned int	m_sizeInBytes;
		Byte *		m_data;
	};

	class Texture2D
	{
	public:
		// Constructors

		static Texture2D FromBitmap(const win32::Bitmap * bitmap);

		// Operations

		void Sample(float u, float v, float * rgb) const;

	private:
		const win32::Bitmap *	m_bitmap;
	};

	class HardcodedRenderer
	{
	public:
		static std::unique_ptr<HardcodedRenderer> Create();

		~HardcodedRenderer() = default;

		HardcodedRenderer(const HardcodedRenderer &) = delete;
		HardcodedRenderer(HardcodedRenderer && other) = default;
		HardcodedRenderer & operator = (const HardcodedRenderer & other) = delete;
		HardcodedRenderer & operator = (HardcodedRenderer && other) = default;

		// Operations

		void		Draw(float milliSeconds);
		void		SwapBuffer();
		void		SetDebugPixel(int pixelX, int pixelY);

		// Properties

		unsigned int	Width();
		unsigned int	Height();
		const void *	GetFrontBuffer();
		const void *	GetBackBuffer();
		const void *	GetDepthBuffer();

	private:
		HardcodedRenderer(unsigned int width, unsigned int height);
		Buffer &	FrontBuffer();
		Buffer &	BackBuffer();
		Buffer &	DepthBuffer();

		int		m_frontId;
		int		m_backId;
		Buffer		m_swapBuffer[2];

		Buffer		m_depthBuffer;
	};
}