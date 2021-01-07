#include "Renderer.h"
#include "RenderWindow.h"
#include "Resource.h"
#include "win32/Win32App.h"

namespace Graphics
{
	class RenderV2 : public IRenderer
	{
	public:
		RenderV2(Graphics::RenderWindow & window)
			: m_window(window)
			, m_device(v2::Device::Default())
		{
			using namespace v2;
			using v2::RenderContext;
			using v2::RenderTarget;

			struct VertexPosRgb
			{
				Vec3 pos, rgb;
			};

			RenderTarget target;
			Matrix4x4 viewTrans;
			Matrix4x4 projTrans;
			VertexShader vs;
			PixelShader ps;

			VertexPosRgb vertices[ 3 ] =
			{
				{{0.0f,   0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
				{{1.0f,   0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
				{{0.5f, 0.866f, 1.0f}, {0.0f, 0.0f, 1.0f}},
			};

			target			= m_device.CreateRenderTarget(window.GetHWND());
			
			m_context		= m_device.CreateRenderContext();
			m_swapChain		= m_device.CreateSwapChain(target.GetWidth(), target.GetHeight());
			m_depthStencilBuffer	= m_device.CreateDepthStencilBuffer(target.GetWidth(), target.GetHeight());

			m_vertexBuffer		= m_device.CreateVertexBuffer(VertexFormat::POS_RGB);
			memcpy(m_vertexBuffer.Data(), &vertices, sizeof(vertices));

			viewTrans		= Matrix4x4::Identity();
			projTrans		= Matrix4x4::PerspectiveFovLH(DegreeToRadian(90),
									      ( double(target.GetWidth()) ) / target.GetHeight(),
									      0.1f,
									      1000.0f);
			vs			= m_device.CreateVertexShader();
			ps			= m_device.CreatePixelShader();

			m_context.SetSwapChain(m_swapChain);
			m_context.SetViewTransform(viewTrans);
			m_context.SetProjectionTransform(projTrans);
			m_context.SetVertexShader(vs);
			m_context.SetPixelShader(ps);
			m_context.SetDepthStencilBuffer(m_depthStencilBuffer);
			m_context.SetOutputTarget(target);
		}

		virtual void Present() override
		{
			m_swapChain.Swap();
			m_window.Paint(m_window.GetWidth(), m_window.GetHeight(), m_swapChain.FrameBuffer());
		}
		virtual void Clear() override
		{
			m_depthStencilBuffer.Reset();
		}
		virtual void Update(double milliSeconds) override
		{
		}
		virtual void Draw() override
		{
			m_context.Draw(m_vertexBuffer, 0, 3);
		}

	private:
		RenderWindow &		m_window;
		v2::Device		m_device;
		v2::RenderContext	m_context;
		v2::SwapChain		m_swapChain;
		v2::DepthStencilBuffer	m_depthStencilBuffer;
		v2::VertexBuffer	m_vertexBuffer;
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
		Graphics::RenderV2 renderer(renderWindow);
		renderWindow.SetRenderer(renderer);
#else
		// Setup renderer

		Graphics::RenderTarget * renderTarget		= Graphics::RenderTarget::FromRenderWindow(renderWindow).release();
		Graphics::RenderContext * renderContext		= new Graphics::RenderContext(W, H);
		Graphics::SceneRenderable * renderable		= new Graphics::SceneRenderable();

		Graphics::Renderer renderer;
		
		renderer.AddRenderable(Ref<Graphics::Renderable>(renderable));
		renderer.Initialize(Ptr<Graphics::RenderContext>(renderContext),
				    Ptr<Graphics::RenderTarget>(renderTarget));
		
		// Setup event handling

		Graphics::SceneState & scene = renderable->GetSceneState();
		Graphics::Camera & camera = *scene.camera;
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
