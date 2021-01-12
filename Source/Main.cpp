#include "Renderer.h"
#include "RenderWindow.h"
#include "Resource.h"
#include "Scene.h"
#include "win32/Win32App.h"

namespace Graphics
{
	class MyScene
	{
	public:
		void			OnLoad(Device & device, RenderContext & context)
		{
			m_device		= &device;
			m_context		= &context;
			m_vertexBuffer		= m_device->CreateVertexBuffer(VertexFormat::POS_RGB);

			Scene * scene		= SceneManager::Default().CreateScene();
			Light * light		= SceneManager::Default().CreateLight();
			Player * player		= SceneManager::Default().CreatePlayer();
			Terrain * terrain	= SceneManager::Default().CreateTerrain();
			Camera * camera		= SceneManager::Default().CreateCamera();
			Controller * controller	= SceneManager::Default().CreateController();

			scene->AddChild(light);
			scene->AddChild(player);
			scene->AddChild(terrain);
			player->AddChild(camera, Connector::THIRD_PERSON_VIEW);
			player->AddChild(controller);

			m_scene			= scene;

			SceneObject::InitializeAll(m_scene, *m_context, m_vertexBuffer);
		}
		void			OnUnload()
		{
		}
		void			OnUpdate(double ms)
		{
			SceneObject::UpdateAll(m_scene, ms);
		}
		void			OnDraw()
		{
			m_context->Draw(m_vertexBuffer, 0, m_vertexBuffer.Count());
		}

	private:
		Device *		m_device;
		RenderContext *		m_context;
		VertexBuffer		m_vertexBuffer;
		Scene *			m_scene;
	};

	class SceneRenderer : public IRenderer
	{
	public:
		SceneRenderer(RenderWindow & window)
			: m_window(window)
			, m_scene(nullptr)
		{
			RenderTarget target;
			VertexShader vs;
			PixelShader ps;

			m_device		= Device::Default();

			target			= m_device.CreateRenderTarget(&m_window);

			m_context		= m_device.CreateRenderContext();
			m_swapChain		= m_device.CreateSwapChain(target.GetWidth(), target.GetHeight());
			m_depthStencilBuffer	= m_device.CreateDepthStencilBuffer(target.GetWidth(), target.GetHeight());

			vs			= m_device.CreateVertexShader();
			ps			= m_device.CreatePixelShader();

			m_context.SetSwapChain(m_swapChain);
			m_context.SetViewTransform(Matrix4x4::Identity());
			m_context.SetProjectionTransform(Matrix4x4::Identity());
			m_context.SetVertexShader(vs);
			m_context.SetPixelShader(ps);
			m_context.SetDepthStencilBuffer(m_depthStencilBuffer);
			m_context.SetOutputTarget(target);
		}

		void			SwitchScene(MyScene & scene)
		{
			if (m_scene)
			{
				m_scene->OnUnload();
			}
			m_scene = &scene;
			m_scene->OnLoad(m_device, m_context);
		}

		virtual void		Present() override
		{
			m_swapChain.Swap();
			m_window.Paint(m_window.GetWidth(), m_window.GetHeight(), m_swapChain.FrameBuffer());
		}
		virtual void		Clear() override
		{
			m_depthStencilBuffer.Reset();
		}
		virtual void		Update(double ms) override
		{
			if ( m_scene )
			{
				m_scene->OnUpdate(ms);
			}
		}
		virtual void		Draw() override
		{
			if ( m_scene )
			{
				m_scene->OnDraw();
			}
		}

	private:
		RenderWindow &		m_window;
		Device			m_device;
		RenderContext		m_context;
		SwapChain		m_swapChain;
		DepthStencilBuffer	m_depthStencilBuffer;
		MyScene *		m_scene;
	};
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
		      _In_opt_ HINSTANCE hPrevInstance,
		      _In_ LPWSTR lpCmdLine,
		      _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	win32::Application app;

	int ret;
	{
		// Setup main window

		Integer W = 800;
		Integer H = 600;

		Graphics::RenderWindow renderWindow(L"My Renderer", hInstance, W, H);
		W = renderWindow.GetWidth();
		H = renderWindow.GetHeight();

#define USE_V2
#ifdef USE_V2
		Graphics::SceneRenderer renderer(renderWindow);
		renderWindow.SetRenderer(renderer);

		Graphics::MyScene scene;
		renderer.SwitchScene(scene);
#else
		// Setup renderer

		Graphics::RenderTarget1 * renderTarget		= Graphics::RenderTarget1::FromRenderWindow(renderWindow).release();
		Graphics::RenderContext1 * renderContext	= new Graphics::RenderContext1(W, H);
		Graphics::SceneRenderable * renderable		= new Graphics::SceneRenderable();

		Graphics::Renderer1 renderer;
		
		renderer.AddRenderable(Ref<Graphics::Renderable1>(renderable));
		renderer.Initialize(Ptr<Graphics::RenderContext1>(renderContext),
				    Ptr<Graphics::RenderTarget1>(renderTarget));
		
		// Setup event handling

		Graphics::SceneState & scene = renderable->GetSceneState();
		Graphics::Camera1 & camera = *scene.camera;
		// keyboard, mouse -> camera
		_BIND_EVENT(OnMouseMove, renderWindow, camera.GetController());
		_BIND_EVENT(OnKeyDown, renderWindow, camera.GetController());
		_BIND_EVENT(OnKeyUp, renderWindow, camera.GetController());
		// resize -> render target, render context, renderables
		_BIND_EVENT(OnWndResize, renderWindow, *renderTarget);
		_BIND_EVENT(OnWndResize, renderWindow, *renderContext);
		_BIND_EVENT(OnWndResize, renderWindow, *renderable);
		_BIND_EVENT(OnAspectRatioChange, renderWindow, camera);

		// Start render loop

		renderWindow.SetRenderer(renderer);
#endif

		ret = app.Run(renderWindow);
	}

	return ret;
}
