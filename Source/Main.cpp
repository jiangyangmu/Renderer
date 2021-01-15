#include "Renderer.h"
#include "RenderWindow.h"
#include "Resource.h"
#include "Scene.h"
#include "win32/Win32App.h"

namespace Graphics
{
	struct SimpleEffect
	{
		struct VS_IN
		{
			Vec3 posWld;
			Vec3 color;
		};
		struct VS_OUT
		{
			Vec3 posNDC;
			Vec3 color;
		};
		struct VS_DATA
		{
			Matrix4x4 view;
			Matrix4x4 proj;
		};

		typedef VS_OUT PS_IN;
		typedef VS_DATA PS_DATA;
		struct PS_OUT
		{
			Vec3 color;
		};

		static void VS(void * pVSOut, const void * pVSIn, const void * pContext)
		{
			const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
			const VS_DATA & context	= *static_cast< const VS_DATA * >( pContext );
			VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

			out.posNDC = Vec3::Transform(Vec3::Transform(in.posWld, context.view), context.proj);
			out.color = in.color;
		}
		static void PS(void * pPSOut, const void * pPSIn, const void * pContext)
		{
			const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
			const PS_DATA & context	= *static_cast< const PS_DATA * >( pContext );
			PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

			out.color = in.color;
		}
	};

	class MyScene
	{
	public:
		void			OnLoad(Device & device, RenderContext & context)
		{
			m_device		= &device;
			m_context		= &context;

			// Setup shader

			VertexFormat fmtVSIn	= m_device->CreateVertexFormat(VertexFieldType::POSITION, VertexFieldType::COLOR);
			VertexFormat fmtVSOut	= fmtVSIn;
			VertexFormat fmtPSIn	= fmtVSOut;
			VertexFormat fmtPSOut	= m_device->CreateVertexFormat(VertexFieldType::COLOR);

			VertexShader vertexShader;
			PixelShader pixelShader;

			m_vertexBuffer		= m_device->CreateVertexBuffer(fmtVSIn);
			vertexShader		= m_device->CreateVertexShader(SimpleEffect::VS, fmtVSIn, fmtVSOut);
			pixelShader		= m_device->CreatePixelShader(SimpleEffect::PS, fmtPSIn, fmtPSOut);

			m_context->SetVertexShader(vertexShader);
			m_context->SetPixelShader(pixelShader);

			// Setup display

			Rect rect		= context.GetOutputTarget().GetRect();
			m_rdtgFullRect		= context.GetOutputTarget();
			m_rdtgMapRect		= device.CreateRenderTarget(m_rdtgFullRect, Rect { rect.right - 300,rect.right, 0, 300 });

			// Setup scene

			Scene * scene		= SceneManager::Default().CreateScene();

			EntityGroup * group	= SceneManager::Default().CreateEntityGroup();
			Light * light		= SceneManager::Default().CreateLight();
			Player * player		= SceneManager::Default().CreatePlayer();
			Terrain * terrain	= SceneManager::Default().CreateTerrain();
			m_cameraPlayer		= SceneManager::Default().CreateCamera();
			m_cameraMiniMap		= SceneManager::Default().CreateCamera();
			Controller * controller	= SceneManager::Default().CreateController();

			scene->AddChild(m_cameraPlayer);
			scene->AddChild(m_cameraMiniMap);
			scene->AddChild(group);
			scene->AddChild(controller);

			group->AddChild(light);
			group->AddChild(terrain);
			group->AddChild(player);

			m_cameraPlayer->Observe(group);
			m_cameraMiniMap->Observe(group);
			m_cameraMiniMap->transform.f42 = 3.0f; // Y

			controller->ConnectTo(player, ConnectType::PLAYER);
			player->ConnectTo(m_cameraPlayer, ConnectType::THIRD_PERSON_VIEW);
			player->ConnectTo(m_cameraMiniMap, ConnectType::MINI_MAP_VIEW);

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
			m_context->SetOutputTarget(m_rdtgFullRect);
			m_cameraPlayer->Draw();
			m_context->SetOutputTarget(m_rdtgMapRect);
			m_cameraMiniMap->Draw();
		}

	private:
		Device *		m_device;
		RenderContext *		m_context;

		RenderTarget		m_rdtgFullRect;
		RenderTarget		m_rdtgMapRect;
		VertexBuffer		m_vertexBuffer; // referred by SceneObject

		Scene *			m_scene;
		Camera *		m_cameraPlayer;
		Camera *		m_cameraMiniMap;
	};

	class SceneRenderer : public IRenderer
	{
	public:
		SceneRenderer(RenderWindow & window)
			: m_window(window)
			, m_scene(nullptr)
		{
			RenderTarget target;
			Rect rect;

			m_device		= Device::Default();

			rect			= Rect { 0, m_window.GetWidth(), 0, m_window.GetHeight() };
			target			= m_device.CreateRenderTarget(&m_window, rect);

			m_context		= m_device.CreateRenderContext();
			m_swapChain		= m_device.CreateSwapChain(target.GetWidth(), target.GetHeight());
			m_depthStencilBuffer	= m_device.CreateDepthStencilBuffer(target.GetWidth(), target.GetHeight());

			m_context.SetSwapChain(m_swapChain);
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
			m_swapChain.ResetBackBuffer();
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

		// TODO: handle window resize

	private:
		RenderWindow &		m_window;
		
		Device			m_device;
		SwapChain		m_swapChain;
		RenderContext		m_context;
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
		Integer W = 800;
		Integer H = 600;

		Graphics::RenderWindow renderWindow(L"My Renderer", hInstance, W, H);
		W = renderWindow.GetWidth();
		H = renderWindow.GetHeight();

		Graphics::SceneRenderer renderer(renderWindow);
		renderWindow.SetRenderer(renderer);

		Graphics::MyScene scene;
		renderer.SwitchScene(scene);

		ret = app.Run(renderWindow);
	}

	return ret;
}
