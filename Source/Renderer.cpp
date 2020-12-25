#include "Renderer.h"
#include "Common.h"
#include "Graphics.h"
#include "win32/Win32App.h"

#include <algorithm>

namespace Rendering
{
	HardcodedRenderer::HardcodedRenderer(Integer width, Integer height)
		: m_context(width, height)
	{
	}

	std::unique_ptr<HardcodedRenderer> HardcodedRenderer::Create()
	{
		std::unique_ptr<HardcodedRenderer> output(new HardcodedRenderer(800, 600));

		output->m_context.GetFrontBuffer().SetAll(100);
		output->m_context.GetBackBuffer().SetAll(100);
		std::fill_n(static_cast< float * >( output->m_context.GetDepthBuffer().Data() ),
			    output->m_context.GetDepthBuffer().ElementCount(),
			    1.0f);

		return output;
	}

	void HardcodedRenderer::ClearSurface()
	{
		m_context.GetBackBuffer().SetAll(0);

		std::fill_n(static_cast< float * >( m_context.GetDepthBuffer().Data() ),
			    m_context.GetDepthBuffer().ElementCount(),
			    1.0f);
	}

	void HardcodedRenderer::Update(float milliSeconds)
	{
	}

	void HardcodedRenderer::Draw()
	{
		using namespace Graphics;

		// Input
		const Camera & camera = DB::DefaultCamera();
		const auto & triangles = DB::Triangles::Perspective();

		m_context.GetConstants().WorldToCamera = Matrix4x4::Identity();
		m_context.GetConstants().CameraToNDC = Matrix4x4::PerspectiveFovLH(camera.fov,
										   camera.aspectRatio,
										   camera.zNear,
										   camera.zFar);
		m_context.GetConstants().DebugPixel[ 0 ] = -1;
		m_context.GetConstants().DebugPixel[ 1 ] = -1;


		// Rasterization
		auto verticesRGB = triangles[ 0 ];
		auto verticesTex = triangles[ 1 ];
		Rasterize(m_context, verticesRGB.data(), verticesRGB.size(), Pipeline::VertexFormat::POSITION_RGB);
		Rasterize(m_context, verticesTex.data(), verticesTex.size(), Pipeline::VertexFormat::POSITION_TEXCOORD);
	}
}