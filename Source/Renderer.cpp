#include "Renderer.h"

#include "Scene.h"
#include "Common.h"

namespace Graphics
{

	void Renderer1::Initialize(Ptr<RenderContext1> renderContext, Ptr<RenderTarget1> renderTarget)
	{
		m_refRenderContext1	= std::move(renderContext);
		m_refRenderTarget1	= std::move(renderTarget);
		m_refRenderInput	= Ptr<RenderInput>(new RenderInput());

		ResetBackBuffer(m_refRenderContext1->GetBackBuffer());
		ResetDepthBuffer(m_refRenderContext1->GetDepthBuffer());
		ResetStencilBuffer(GetRenderContext1().GetStencilBuffer());

		for (Ref<Renderable1> & renderable : m_renderables)
		{
			renderable->Initialize(*m_refRenderContext1, *m_refRenderInput);
		}
	}

	void Renderer1::Present()
	{
		m_refRenderContext1->SwapBuffer();

		Buffer & frameBuffer = m_refRenderContext1->GetFrontBuffer();

		m_refRenderTarget1->CopyPixelData((Byte *)frameBuffer.Data(),
						  frameBuffer.SizeInBytes());
	}

	void Renderer1::Clear()
	{
		ResetBackBuffer(m_refRenderContext1->GetBackBuffer());
		ResetDepthBuffer(m_refRenderContext1->GetDepthBuffer());
	}

	void Renderer1::Update(double milliSeconds)
	{
		for (Ref<Renderable1> & renderable : m_renderables)
		{
			renderable->Update(milliSeconds);
		}
	}

	void Renderer1::Draw()
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
			m_refRenderContext1->SetVertexShader(vss[ index ]);
			m_refRenderContext1->SetPixelShader(pss[ index ]);

			auto & vertices = triangles[ index ];
			Rasterize(*m_refRenderContext1, vertices.data(), vertices.size(), formats[ index ]);
		}
	}

	_RECV_EVENT_IMPL(Renderable1, OnWndResize) ( void * sender, const win32::WindowRect & args )
	{
		ResetBackBuffer(m_refRenderContext1->GetBackBuffer());
		ResetDepthBuffer(m_refRenderContext1->GetDepthBuffer());
		ResetStencilBuffer(GetRenderContext1().GetStencilBuffer());
	}

	void SceneRenderable::Initialize(RenderContext1 & renderContext, RenderInput & renderInput)
	{
		Renderable1::Initialize(renderContext, renderInput);

		m_refSceneState.reset(new SceneState(SceneLoader::Default(
			renderContext.GetWidth(),
			renderContext.GetHeight()
		)));
		
		GetRenderContext1().LoadTexture(m_refSceneState->textureURL);
		GetRenderContext1().GetConstants().Texture = GetRenderContext1().GetTexture(0);
		GetRenderContext1().GetConstants().Light = m_refSceneState->light;
		GetRenderContext1().GetConstants().Material = m_refSceneState->material;

		GetRenderInput().m_vertices = m_refSceneState->vertices;
	}

	void SceneRenderable::Update(double milliSeconds)
	{
		Camera1 & camera = *m_refSceneState->camera;
		
		// Update scene
		camera.GetController().Update(milliSeconds);

		// Update render context
		GetRenderContext1().GetConstants().WorldToCamera = camera.GetViewMatrix();
		GetRenderContext1().GetConstants().CameraToNDC = camera.GetProjMatrix();
		GetRenderContext1().GetConstants().CameraPosition = camera.GetPos();

		// Update render input
	}

	ROTriangle::ROTriangle(Vec3 pos0, Vec3 rgb0, Vec3 pos1, Vec3 rgb1, Vec3 pos2, Vec3 rgb2)
		: m_vertex { {pos0, rgb0}, {pos1, rgb1}, {pos2, rgb2} }
		, m_vertexRange()
		, m_refVertexBuffer(nullptr)
		, m_refContext(nullptr)
	{
	}
	ROTriangle::~ROTriangle()
	{
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}
	}
	void		ROTriangle::Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer)
	{
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refContext		= &renderContext;
		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(3);

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(Vertex) * 3);
	}
	void		ROTriangle::Draw()
	{
		m_refContext->Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}

}