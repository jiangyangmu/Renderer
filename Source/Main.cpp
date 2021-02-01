#include "Includes/RendererApi.h"
#include "Platform/Win32App.h"
#include "Test/TestScene.h"

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
		Integer W = 1080;
		Integer H = 768;

		Graphics::RenderWindow renderWindow(L"My Renderer", hInstance, W, H);
		W = renderWindow.GetWidth();
		H = renderWindow.GetHeight();

		Graphics::SceneRenderer renderer(renderWindow);
		renderWindow.SetRenderer(renderer);

		Ptr<Graphics::IScene> scene = Graphics::CreateTestScene_Water();
		renderer.SwitchScene(*scene);

		ret = app.Run(renderWindow);
	}

	return ret;
}