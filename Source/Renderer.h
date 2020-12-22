#pragma once

#include "Graphics.h"

namespace Rendering
{
	using Graphics::Buffer;

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

		void		ClearSurface();
		void		Update(float milliSeconds);
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