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
		const auto & triangles = DB::Scene::LightingTest();
		m_context.GetConstants().WorldToCamera = GetCamera().GetViewMatrix();
		m_context.GetConstants().CameraToNDC = GetCamera().GetProjMatrix();
		m_context.GetConstants().CameraPosition = GetCamera().GetPos();
		m_context.GetConstants().Light = Lights::Light
		{
			{ 1.0f, 1.0f, 1.0f }, // color
			{ 0.3f, 0.3f, 0.1f }, // position
			{ 0.0f, 0.0f, 1.0f } // attenuation
		};
		m_context.GetConstants().Material = Materials::BlinnPhong
		{
			Materials::Ambient{{1.0f, 1.0f, 1.0f}},
			Materials::Diffuse{{1.0f, 1.0f, 1.0f}},
			Materials::Specular{{1.0f, 1.0f, 1.0f}},
			0.0f,
			0.8f,
			0.2f,
		};

		// Rasterization
		Pipeline::VertexFormat formats[] =
		{
			Pipeline::VertexFormat::POSITION_RGB,
			Pipeline::VertexFormat::POSITION_TEXCOORD,
			Pipeline::VertexFormat::POSITION_NORM_MATERIAL,
		};
		Pipeline::Shader::VertexShader::Func vss[] =
		{
			Pipeline::Shader::VertexShader::VS_RGB,
			Pipeline::Shader::VertexShader::VS_TEX,
			Pipeline::Shader::VertexShader::VS_BlinnPhong,
		};
		Pipeline::Shader::PixelShader::Func pss[] =
		{
			Pipeline::Shader::PixelShader::PS_RGB,
			Pipeline::Shader::PixelShader::PS_TEX,
			Pipeline::Shader::PixelShader::PS_BlinnPhong,
		};
		for (int index = 0; index < triangles.size() && index < (sizeof(formats)/sizeof(Pipeline::VertexFormat)); ++index)
		{
			m_context.SetVertexShader(vss[index]);
			m_context.SetPixelShader(pss[index]);

			auto & vertices = triangles[index];
			Rasterize(m_context, vertices.data(), vertices.size(), formats[index]);
		}
	}
}