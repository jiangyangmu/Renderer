#pragma once

#include "Graphics.h"
#include "Scene.h"

namespace Graphics
{
	class Renderable
	{
	public:
		Renderable()
			: m_refRenderContext(nullptr)
			, m_refRenderInput(nullptr)
		{
		}

		// Operations

		virtual void		Initialize(RenderContext & renderContext, RenderInput & renderInput)
		{
			m_refRenderContext = &renderContext;
			m_refRenderInput = &renderInput;
		}
		virtual void		Update(double milliSeconds) = 0;

		// Properties

		RenderContext &		GetRenderContext()
		{
			return *m_refRenderContext;
		}
		RenderInput &		GetRenderInput()
		{
			return *m_refRenderInput;
		}

		// Events

	public: _RECV_EVENT_DECL1(Renderable, OnWndResize);

	private:
		RenderContext *		m_refRenderContext;
		RenderInput *		m_refRenderInput;
	};

	class SceneRenderable : public Renderable
	{
	public:
		// Operations

		virtual void		Initialize(RenderContext & renderContext, RenderInput & renderInput) override;
		virtual void		Update(double milliSeconds) override;

		// Properties

		SceneState &		GetSceneState()
		{
			return *m_refSceneState;
		}

	private:
		Ptr<SceneState>		m_refSceneState;
	};

	class Renderer
	{
	public:

		Renderer()
			: m_refRenderContext(nullptr), m_refRenderTarget(nullptr)
		{
		}

		// Operations

		void			AddRenderable(Ref<Renderable> renderable)
		{
			m_renderables.emplace_back(renderable);
		}

		void			Initialize(Ptr<RenderContext> renderContext, Ptr<RenderTarget> renderTarget);

		void			Present();
		void			Clear();
		void			Update(double milliSeconds);
		void			Draw();

		// Properties

		RenderContext &		GetRenderContext()
		{
			return *m_refRenderContext;
		}
		Ref<Renderable>	&	GetRenderable(Integer i)
		{
			return m_renderables.at(i);
		}

	private:
		Ptr<RenderInput>		m_refRenderInput;
		Ptr<RenderContext>		m_refRenderContext;
		Ptr<RenderTarget>		m_refRenderTarget;
		std::vector<Ref<Renderable>>	m_renderables;
	};
}