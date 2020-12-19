#pragma once

#include <memory>

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