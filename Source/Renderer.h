#pragma once

namespace Renderer
{
	// TODO: split FrameBuffer to FrontBuffer and BackBuffer
	// TODO: manage buffers (front, back, depth): create, resize, destroy
	class RenderResult
	{
	public:
		static RenderResult Create();
		~RenderResult();

		RenderResult(const RenderResult &) = delete;
		RenderResult(RenderResult && other);
		RenderResult & operator = (const RenderResult & other) = delete;
		RenderResult & operator = (RenderResult && other);

		// Properties

		unsigned int	Width();
		unsigned int	Height();
		unsigned int	GetFrameBufferSize();
		const void *	GetFrameBuffer();

	private:
		RenderResult(unsigned int width, unsigned int height);
		void *		FrameBuffer();

		unsigned int	m_width;
		unsigned int	m_height;
		unsigned int	m_frameBufferSize;
		void *		m_frameBuffer;
	};

	// TODO: buffer classes: width/height, element size, element [i], reshape
}