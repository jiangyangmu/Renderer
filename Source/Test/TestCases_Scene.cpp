#include "TestCases.h"
#include "../Core/Renderer.h"

#include <cstdlib>

using namespace Graphics;

extern Ptr<IScene>	TestScene_Effects(int argc, char * argv[]);
extern Ptr<IScene>	TestScene_Minecraft(int argc, char * argv[]);
extern Ptr<IScene>	TestScene_Mirror(int argc, char * argv[]);
extern Ptr<IScene>	TestScene_Water(int argc, char * argv[]);

struct SceneTestCase
{
	const char * pName;
	Ptr<Graphics::IScene>(*pScene)(int argc, char * argv[]);
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

void	TestSuit_Scene(int argc, char * argv[])
{
	NativeWindow * pWindow;
	SceneTestCase * pCase;
	Ptr<Graphics::IScene> scene;

	pCase = TestFindCase(tcScene, TestCaseName());
	if (!pCase)
	{
		printf("No test scene matches name '%s'.\n", TestCaseName());
		TestListCase(tcScene, "scenes");
		return;
	}

	NativeInitialize();
	pWindow = NativeCreateWindow(GetTitle(TestCaseName()), 800, 600);

	if (pWindow)
	{
		RenderWindow window(pWindow);
		SceneRenderer renderer(window);
		
		scene = pCase->pScene(argc - 1, argv + 1);
		
		renderer.SwitchScene(*scene);
		RenderMainLoop(pWindow, &renderer);
	}

	NativeDestroyWindow(pWindow);
	NativeTerminate();
}
