#pragma once

#include "Graphics.h"
#include "Resource.h"

namespace Graphics
{
	class Renderable1
	{
	public:
		Renderable1()
			: m_refRenderContext1(nullptr)
			, m_refRenderInput(nullptr)
		{
		}
		virtual			~Renderable1() = default;

		// Operations

		virtual void		Initialize(RenderContext1 & renderContext, RenderInput & renderInput)
		{
			m_refRenderContext1 = &renderContext;
			m_refRenderInput = &renderInput;
		}
		virtual void		Update(double milliSeconds) = 0;

		// Properties

		RenderContext1 &	GetRenderContext1()
		{
			return *m_refRenderContext1;
		}
		RenderInput &		GetRenderInput()
		{
			return *m_refRenderInput;
		}

		// Events

	public: _RECV_EVENT_DECL1(Renderable1, OnWndResize);

	private:
		RenderContext1 *	m_refRenderContext1;
		RenderInput *		m_refRenderInput;
	};

	class Renderable
	{
	public:
		virtual			~Renderable() = default;
		virtual void		Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer) {}
		virtual void		Update(double ms) {}
		virtual void		Draw() {}
	};

	class ROTriangle : public Renderable
	{
	public:
		ROTriangle(Vec3 pos0, Vec3 rgb0, Vec3 pos1, Vec3 rgb1, Vec3 pos2, Vec3 rgb2);
		virtual			~ROTriangle();

		virtual void		Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer) override;
		virtual void		Draw() override;

	private:
		struct Vertex
		{
			Vec3 pos;
			Vec3 rgb;
		};
		Vertex			m_vertex[ 3 ];
		VertexRange		m_vertexRange;
		VertexBuffer *		m_refVertexBuffer;
		RenderContext *		m_refContext;
	};

	class IRenderer
	{
	public:
		virtual			~IRenderer() = default;
		virtual void		Present() = 0;
		virtual void		Clear() = 0;
		virtual void		Update(double milliSeconds) = 0;
		virtual void		Draw() = 0;
	};

	class Renderer1 : public IRenderer
	{
	public:

		Renderer1()
			: m_refRenderContext1(nullptr), m_refRenderTarget1(nullptr)
		{
		}

		// Operations

		void			AddRenderable(Ref<Renderable1> renderable)
		{
			m_renderables.emplace_back(renderable);
		}

		void			Initialize(Ptr<RenderContext1> renderContext, Ptr<RenderTarget1> renderTarget);

		virtual void		Present() override;
		virtual void		Clear() override;
		virtual void		Update(double milliSeconds) override;
		virtual void		Draw() override;

		// Properties

		RenderContext1 &	GetRenderContext1()
		{
			return *m_refRenderContext1;
		}
		Ref<Renderable1>	&	GetRenderable(Integer i)
		{
			return m_renderables.at(i);
		}

	private:
		Ptr<RenderInput>		m_refRenderInput;
		Ptr<RenderContext1>		m_refRenderContext1;
		Ptr<RenderTarget1>		m_refRenderTarget1;
		std::vector<Ref<Renderable1>>	m_renderables;
	};
}