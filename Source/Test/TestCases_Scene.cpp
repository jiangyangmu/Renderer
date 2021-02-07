#include "TestCases.h"
#include "../Core/Renderer.h"
#include "../Native/Win32App.h"

#include <cstdlib>

using namespace Graphics;

struct SceneTestCase
{
	const char * pName;
	Ptr<Graphics::IScene> (*pScene)();
};

static SceneTestCase	tcScene[] =
{
	{"effects",	TestScene_Effects},
	{"minecraft",	TestScene_Minecraft},
	{"mirror",	TestScene_Mirror},
	{"water",	TestScene_Water},
};

const wchar_t *		GetTitle(const char * pName)
{
	static char	bufWndTitle[ 256 ];
	static wchar_t	bufWndTitleW[ 256 ];
	size_t		nSize;

	sprintf_s(bufWndTitle, "Scene %s", pName);
	mbstowcs_s(&nSize, bufWndTitleW, bufWndTitle, 256);

	return bufWndTitleW;
}

bool	TestScene(int argc, char * argv[])
{
	ULONG_PTR hGdiPlus;
	NativeWindow * pWindow;
	SceneTestCase * pCase;
	Ptr<Graphics::IScene> scene;

	hGdiPlus = win32::InitializeGdiplus();
	NativeInitialize();

	pWindow = NativeCreateWindow(GetTitle(TestCaseName()), 800, 600);
	pCase = nullptr;

	if (pWindow)
	{
		RenderWindow window(pWindow);
		SceneRenderer renderer(window);
		
		pCase = TestFindCase(tcScene, TestCaseName());

		if (pCase)
		{
			scene = TestScene_Water();
		
			renderer.SwitchScene(*scene);
			RenderMainLoop(pWindow, &renderer);
		}
	}

	NativeDestroyWindow(pWindow);

	NativeTerminate();
	win32::UninitializeGdiplus(hGdiPlus);

	return pCase != nullptr;
}
