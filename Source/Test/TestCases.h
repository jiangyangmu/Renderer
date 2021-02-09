#pragma once

#include "../Includes/RendererApi.h"

// Entry point
void			TestMain(int argc, char * argv[]);

// Test cases
void			TestNative_Callbacks();
void			TestNative_Blit();
void			TestNative_MultipleWindow();
void			TestScene(int argc, char * argv[]);
void			TestGraphics_Buffer(int argc, char * argv[]);

// Test scenes
Ptr<Graphics::IScene>	TestScene_Effects();
Ptr<Graphics::IScene>	TestScene_Minecraft();
Ptr<Graphics::IScene>	TestScene_Mirror();
Ptr<Graphics::IScene>	TestScene_Water();

// Helper
template <typename T>
T *	_TestFindCase(T * pCases, int nCases, const char * pName)
{
	for (int i = 0; i < nCases; ++i)
	{
		if (strcmp(pCases[i].pName, pName) == 0)
			return pCases + i;
	}
	return nullptr;
}
template <typename T>
void	_TestListCase(T * pCases, int nCases, const char * pDesc)
{
	printf("Test %s:\n", pDesc);
	while (nCases > 0)
	{
		printf("\t%s\n", pCases->pName);
		++pCases;
		--nCases;
	}
}
#define TestCaseName() \
	((argc >= 1 && argv[0]) ? argv[0] : "")
#define TestFindCase(caseArray, name) \
	_TestFindCase((caseArray), sizeof(caseArray) / sizeof((caseArray)[0]), (name))
#define TestListCase(caseArray, desc) \
	_TestListCase((caseArray), sizeof(caseArray) / sizeof((caseArray)[0]), (desc))