#include "Includes/RendererApi.h"
#include "Native/Win32App.h"
#include "Test/TestScene.h"
#include "Core/Native.h"

#include <iostream>

int main(int argc, char * argv[])
{
	NativeWindow * pMain;
	NativeWindow * pTool;

	if ( NativeInitialize() )
	{
		pMain = NativeCreateWindow(L"Renderer", 800, 600);

		if ( pMain )
		{
			NativeRegisterWindowCallbacks(pMain, NativeGetDebugWindowCallbacks());
			NativeRegisterKeyboardCallbacks(pMain, NativeGetDebugKeyboardCallbacks());
			NativeRegisterMouseCallbacks(pMain, NativeGetDebugMouseCallbacks());

			while ( NativeGetWindowCount() > 0 )
			{
				NativeInputPoll();
				Sleep(10);
			}
			NativeDestroyWindow(pMain);
		}

		NativeTerminate();
	}

	return 0;
}