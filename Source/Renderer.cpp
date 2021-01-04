#include "Renderer.h"
#include "Common.h"

namespace Graphics
{

	void Renderer::Initialize(Ptr<RenderContext> renderContext, Ptr<RenderTarget> renderTarget)
	{
		m_refRenderContext	= std::move(renderContext);
		m_refRenderTarget	= std::move(renderTarget);
		m_refRenderInput	= Ptr<RenderInput>(new RenderInput());

		ResetBackBuffer(m_refRenderContext->GetBackBuffer());
		ResetDepthBuffer(m_refRenderContext->GetDepthBuffer());
		ResetStencilBuffer(GetRenderContext().GetStencilBuffer());

		for (Ref<Renderable> & renderable : m_renderables)
		{
			renderable->Initialize(*m_refRenderContext, *m_refRenderInput);
		}
	}

	void Renderer::Present()
	{
		m_refRenderContext->SwapBuffer();

		Buffer & frameBuffer = m_refRenderContext->GetFrontBuffer();

		m_refRenderTarget->CopyPixelData((Byte *)frameBuffer.Data(),
						frameBuffer.SizeInBytes());
	}

	void Renderer::Clear()
	{
		ResetBackBuffer(m_refRenderContext->GetBackBuffer());
		ResetDepthBuffer(m_refRenderContext->GetDepthBuffer());
	}

	void Renderer::Update(double milliSeconds)
	{
		for (Ref<Renderable> & renderable : m_renderables)
		{
			renderable->Update(milliSeconds);
		}
	}

	void Renderer::Draw()
	{
		using namespace Graphics;

		// Input
		const auto & triangles = m_refRenderInput->m_vertices;

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
		for ( int index = 0; index < triangles.size() && index < ( sizeof(formats) / sizeof(Pipeline::VertexFormat) ); ++index )
		{
			m_refRenderContext->SetVertexShader(vss[ index ]);
			m_refRenderContext->SetPixelShader(pss[ index ]);

			auto & vertices = triangles[ index ];
			Rasterize(*m_refRenderContext, vertices.data(), vertices.size(), formats[ index ]);
		}
	}

	_RECV_EVENT_IMPL(Renderable, OnWndResize) ( void * sender, const win32::WindowRect & args )
	{
		ResetBackBuffer(m_refRenderContext->GetBackBuffer());
		ResetDepthBuffer(m_refRenderContext->GetDepthBuffer());
		ResetStencilBuffer(GetRenderContext().GetStencilBuffer());
	}

	void SceneRenderable::Initialize(RenderContext & renderContext, RenderInput & renderInput)
	{
		Renderable::Initialize(renderContext, renderInput);

		m_refSceneState.reset(new SceneState(SceneLoader::Default(
			renderContext.GetWidth(),
			renderContext.GetHeight()
		)));
		
		GetRenderContext().LoadTexture(m_refSceneState->textureURL);
		GetRenderContext().GetConstants().Texture = GetRenderContext().GetTexture(0);
		GetRenderContext().GetConstants().Light = m_refSceneState->light;
		GetRenderContext().GetConstants().Material = m_refSceneState->material;

		GetRenderInput().m_vertices = m_refSceneState->vertices;
	}

	void SceneRenderable::Update(double milliSeconds)
	{
		Camera & camera = *m_refSceneState->camera;
		
		// Update scene
		camera.GetController().Update(milliSeconds);

		// Update render context
		GetRenderContext().GetConstants().WorldToCamera = camera.GetViewMatrix();
		GetRenderContext().GetConstants().CameraToNDC = camera.GetProjMatrix();
		GetRenderContext().GetConstants().CameraPosition = camera.GetPos();

		// Update render input
	}
}