#include "TestCases.h"

static TestSuit suits[] =
{
	{"native",	TestSuit_Native},
	{"scene",	TestSuit_Scene},
	{"graphics",	TestSuit_Graphics},
	{"gltf",	TestSuit_glTF},
};

void		TestMain(int argc, char * argv[])
{
	const char * pName;
	TestSuit * pSuit;
	
	pName = TestCaseName();
	pSuit = nullptr;

	if (pSuit = TestFindCase(suits, pName))
	{
		pSuit->pEntry(argc - 1, argv + 1);
		return;
	}

	printf("No test suit matches name '%s'.\n", pName);

	TestListCase(suits, "suits");
}