#pragma once

#include "../Core/RenderWindow.h"
#include "../Core/Renderer.h"
#include "../Core/Scene.h"
#include "../Core/VisualEffects.h"

struct TestCase
{
	const char * pName;
	void ( *pFunc )( int, char *[] );
};

struct TestSuit
{
	const char * pName;
	void ( *pEntry )( int, char *[] );
};

// Helper
template <typename T>
T *			_TestFindCase(T * pCases, int nCases, const char * pName)
{
	for ( int i = 0; i < nCases; ++i )
	{
		if ( strcmp(pCases[ i ].pName, pName) == 0 )
			return pCases + i;
	}
	return nullptr;
}
template <typename T>
void			_TestListCase(T * pCases, int nCases, const char * pDesc)
{
	printf("Test %s:\n", pDesc);
	while ( nCases > 0 )
	{
		printf("\t%s\n", pCases->pName);
		++pCases;
		--nCases;
	}
}
#define			TestCaseName() \
			((argc >= 1 && argv[0]) ? argv[0] : "")
#define			TestFindCase(caseArray, name) \
			_TestFindCase((caseArray), sizeof(caseArray) / sizeof((caseArray)[0]), (name))
#define			TestListCase(caseArray, desc) \
			_TestListCase((caseArray), sizeof(caseArray) / sizeof((caseArray)[0]), (desc))
#define			TestSuitEntry(suit) \
void			TestSuit_##suit(int argc, char * argv[]) \
{ \
	const char * pName; \
	TestCase * pCase; \
	pName = TestCaseName(); \
	pCase = nullptr; \
	if ( pCase = TestFindCase(cases, pName) ) \
	{ \
		pCase->pFunc(argc - 1, argv + 1); \
		return; \
	} \
	printf("No test case matches name '%s' in suit '" #suit "'.\n", pName); \
	TestListCase(cases, "cases"); \
}
// Entry point
void			TestMain(int argc, char * argv[]);
void			TestSuit_Native(int argc, char * argv[]);
void			TestSuit_Scene(int argc, char * argv[]);
void			TestSuit_Graphics(int argc, char * argv[]);
void			TestSuit_glTF(int argc, char * argv[]);