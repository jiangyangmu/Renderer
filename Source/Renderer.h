#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Gdiplus
{
	class Bitmap;
}

namespace Renderer
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
		Texture2D();
		~Texture2D();

		Texture2D(const Texture2D &) = delete;
		Texture2D(Texture2D && other) = delete;
		Texture2D & operator = (const Texture2D &) = delete;
		Texture2D & operator = (Texture2D && other) = delete;

		void LoadFromFile(std::wstring filePath);

		void Sample(float u, float v, float * r, float * g, float * b);

	private:
		Gdiplus::Bitmap * m_bitmap;
	};

	class RenderResult
	{
	public:
		static std::unique_ptr<RenderResult> Create();

		~RenderResult() = default;

		RenderResult(const RenderResult &) = delete;
		RenderResult(RenderResult && other) = default;
		RenderResult & operator = (const RenderResult & other) = delete;
		RenderResult & operator = (RenderResult && other) = default;

		// Operations

		void		Draw();
		void		SwapBuffer();
		void		SetDebugPixel(int pixelX, int pixelY);

		// Properties

		unsigned int	Width();
		unsigned int	Height();
		const void *	GetFrontBuffer();
		const void *	GetBackBuffer();
		const void *	GetDepthBuffer();

	private:
		RenderResult(unsigned int width, unsigned int height);
		Buffer &	FrontBuffer();
		Buffer &	BackBuffer();
		Buffer &	DepthBuffer();

		int		m_frontId;
		int		m_backId;
		Buffer		m_swapBuffer[2];

		Buffer		m_depthBuffer;
	};
}