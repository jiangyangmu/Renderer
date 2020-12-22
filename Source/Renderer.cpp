#include "Renderer.h"
#include "Common.h"
#include "Graphics.h"
#include "win32/Win32App.h"

#include <algorithm>
#include <malloc.h> // _aligned_malloc

using Graphics::Buffer;
using Graphics::Texture2D;
using Graphics::Camera;
using Graphics::RGB;

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

	std::unique_ptr<HardcodedRenderer> HardcodedRenderer::Create()
	{
		std::unique_ptr<HardcodedRenderer> output(new HardcodedRenderer(800, 600));

		output->FrontBuffer().SetAll(100);
		output->BackBuffer().SetAll(100);
		std::fill_n(reinterpret_cast< float * >( output->DepthBuffer().Data() ),
			    output->DepthBuffer().ElementCount(),
			    1.0f);

		return output;
	}

	void HardcodedRenderer::ClearSurface()
	{
		BackBuffer().SetAll(100);

		std::fill_n(reinterpret_cast< float * >( DepthBuffer().Data() ),
			    DepthBuffer().ElementCount(),
			    1.0f);
	}

	void HardcodedRenderer::SetDebugPixel(int pixelX, int pixelY)
	{
		using Graphics::DB::GlbDebugPixel;
		GlbDebugPixel[ 0 ] = pixelX;
		GlbDebugPixel[ 1 ] = pixelY;
	}

	void HardcodedRenderer::Update(float milliSeconds)
	{
	}

	void HardcodedRenderer::Draw()
	{
		// Output
		Graphics::Buffer & backBuffer = BackBuffer();
		Graphics::Buffer & depthBuffer = DepthBuffer();

		// Input
		const Camera & camera = Graphics::DB::DefaultCamera();
		const auto & triangles = Graphics::DB::Triangles::TextureTest();

		// Projection & Clipping
		Graphics::Transform transform(camera);

		// Rasterization
		{
			Graphics::RenderTarget renderTarget(Width(),
								    Height(),
								    backBuffer.Data());

			for ( auto & triangle : triangles )
			{
				Graphics::Rasterizer::Rasterize(
					renderTarget.Width(),
					renderTarget.Height(),
					camera,
					triangle,
					transform,
					[&] (int pixelX, int pixelY, const Vec3 & pos, const RGB & color)
					{
						// Depth test
						float * pOldDepth = reinterpret_cast< float * >( depthBuffer.At(pixelY, pixelX) );

						float oldDepth = *pOldDepth;
						float newDepth = pos.z;

						if ( oldDepth <= newDepth )
						{
							return;
						}
						*pOldDepth = newDepth;

						renderTarget.SetPixel(pixelX,
								      pixelY,
								      static_cast< unsigned char >( color.r * 255.0f ),
								      static_cast< unsigned char >( color.g * 255.0f ),
								      static_cast< unsigned char >( color.b * 255.0f ));
					});
			}
		}
	}
}