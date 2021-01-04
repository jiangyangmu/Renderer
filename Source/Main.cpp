#include "Renderer.h"
#include "RenderWindow.h"
#include "win32/Win32App.h"

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
		ret = app.Run(renderWindow);
	}

	return ret;
}
