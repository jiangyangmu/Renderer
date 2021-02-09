#include "TestCases.h"

#include <vector>

struct TestCase
{
	const char * pName;
	void ( *pFunc )( );
};

struct TestSuit
{
	const char * pName;
	void ( *pEntry )( int, char *[] );
};

static TestCase	cases[] =
{
	{"callback",	TestNative_Callbacks},
	{"blit",	TestNative_Blit},
	{"window",	TestNative_MultipleWindow},
};

static TestSuit suits[] =
{
	{"scene",	TestScene},
	{"buffer",	TestGraphics_Buffer},
};

void		TestMain(int argc, char * argv[])
{
	const char * pName;
	TestCase * pCase;
	TestSuit * pSuit;
	
	pName = TestCaseName();
	pCase = nullptr;
	pSuit = nullptr;

	if (pCase = TestFindCase(cases, pName))
	{
		pCase->pFunc();
		return;
	}
	if (pSuit = TestFindCase(suits, pName))
	{
		pSuit->pEntry(argc - 1, argv + 1);
		return;
	}

	printf("No test case or suit matches name '%s'.\n", pName);

	TestListCase(cases, "cases");
	TestListCase(suits, "suits");
}