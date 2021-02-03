#include "Includes/RendererApi.h"
#include "Platform/Win32App.h"
#include "Test/TestScene.h"

#include <Windows.h>

int main(int argc, char * argv[])
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	ENSURE_NOT_NULL(hInstance);

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