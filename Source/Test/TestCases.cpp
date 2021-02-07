#include "TestCases.h"

#include <vector>

struct TestCase
{
	const char * pName;
	void ( *pFunc )( );
};

static void Noop()
{
}

static TestCase	cases[] =
{
	{"scene",	Noop},

	{"callback",	TestNative_Callbacks},
	{"blit",	TestNative_Blit},
	{"window",	TestNative_MultipleWindow},
};

void		TestMain(int argc, char * argv[])
{
	TestCase * pCase;
	const char * pName;
	
	pName = TestCaseName();

	if ( strcmp(pName, "scene") == 0 )
	{
		if ( TestScene(argc - 1, argv + 1) )
			return;
	}
	else
	{
		pCase = TestFindCase(cases, pName);

		if ( pCase )
		{
			pCase->pFunc();
			return;
		}
		
		printf("No test case matches name '%s'.\n", pName);
	}

	TestListCase(cases);
}