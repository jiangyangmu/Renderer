#include "Renderer.h"
#include "Common.h"
#include "Graphics.h"
#include "win32/Win32App.h"

#include <algorithm>

namespace Rendering
{
	HardcodedRenderer::HardcodedRenderer(Integer width, Integer height)
		: m_context(width, height)
		, m_camera(0.1f,			// z near
			   1000.0f,			// z far (use smaller value for better depth test.)
			   DegreeToRadian(90.0f),	// field of view
			   ((float)width) / height,	// aspect ratio
			   Vec3::Zero())		// position
	{
	}

	void HardcodedRenderer::Resize(Integer width, Integer height)
	{
		m_context.Resize(width, height);
		m_camera.SetAspectRatio(((float)width) / height);
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
		GetCamera().GetController().Update(milliSeconds);
	}

	void HardcodedRenderer::Draw()
	{
		using namespace Graphics;

		// Input
		const auto & triangles = DB::Triangles::LightingTest();
		m_context.GetConstants().WorldToCamera = GetCamera().GetViewMatrix();
		m_context.GetConstants().CameraToNDC = GetCamera().GetProjMatrix();

		// Rasterization
		Pipeline::VertexFormat formats[] = {
			Pipeline::VertexFormat::POSITION_RGB,
			Pipeline::VertexFormat::POSITION_TEXCOORD,
			Pipeline::VertexFormat::POSITION_NORM_MATERIAL,
		};
		for (int index = 0; index < triangles.size() && index < (sizeof(formats)/sizeof(Pipeline::VertexFormat)); ++index)
		{
			auto & vertices = triangles[index];
			Rasterize(m_context, vertices.data(), vertices.size(), formats[index]);
		}
	}
}