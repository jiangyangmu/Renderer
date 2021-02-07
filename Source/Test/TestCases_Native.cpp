#include "TestCases.h"
#include "../Core/Native.h"

#define WINDOW_WIDTH	(800)
#define WINDOW_HEIGHT	(600)

static NativeKeyboardCallbacks cbKeyboard;
static NativeWindow * pMain = nullptr;
static NativeWindow * pWindowGroup[4];
static wchar_t bufWindowTitle[256];

struct BGRA
{
	Graphics::u32 pixel;
};
struct BGR
{
	Graphics::u8 b, g, r;
};
struct DEPTH
{
	Graphics::f32 depth;
};
struct STENCIL
{
	Graphics::u8 stencil;
};

static BGRA	image0[ WINDOW_WIDTH * WINDOW_HEIGHT ];
static BGR	image1[ WINDOW_WIDTH * WINDOW_HEIGHT ];
static DEPTH	image2[ WINDOW_WIDTH * WINDOW_HEIGHT ];
static STENCIL	image3[ WINDOW_WIDTH * WINDOW_HEIGHT ];

static void	Keyboard_Down(int keycode)
{
	bool bSuccess = false;

	switch ( keycode )
	{
		case '1': bSuccess = NativeWindowBilt(pMain, image0, NATIVE_BLIT_BGRA); break;
		case '2': bSuccess = NativeWindowBilt(pMain, image1, NATIVE_BLIT_BGR); break;
		case '3': bSuccess = NativeWindowBilt(pMain, image2, NATIVE_BLIT_F32); break;
		case '4': bSuccess = NativeWindowBilt(pMain, image3, NATIVE_BLIT_U8); break;
		default: break;
	}

	printf("Key Down:   %d(%c) set color: %s\n",
	       keycode,
	       isprint(keycode) ? (char)keycode : '?',
	       bSuccess ? "ok" : "bad");
}
static void	Keyboard_Up(int keycode)
{
	printf("Key Up:     %d(%c)\n",
	       keycode,
	       isprint(keycode) ? (char)keycode : '?');
}

void		TestNative_Callbacks()
{
	if ( NativeInitialize() )
	{
		pMain = NativeCreateWindow(L"Callbacks Test", WINDOW_WIDTH, WINDOW_HEIGHT);
		if ( pMain )
		{
			NativeRegisterWindowCallbacks(pMain, NativeDebugGetWindowCallbacks());
			NativeRegisterKeyboardCallbacks(pMain, NativeDebugGetKeyboardCallbacks());
			NativeRegisterMouseCallbacks(pMain, NativeDebugGetMouseCallbacks());

			while ( NativeGetWindowCount() > 0 )
			{
				NativeInputPoll();
				Sleep(10);
			}

			NativeDestroyWindow(pMain);
		}
		NativeTerminate();
	}
}
void		TestNative_Blit()
{
	cbKeyboard.down = Keyboard_Down;
	cbKeyboard.up = Keyboard_Up;

	BGRA * p0 = image0;
	BGR * p1 = image1;
	DEPTH * p2 = image2;
	STENCIL * p3 =  image3;
	for ( int r = 0; r < WINDOW_WIDTH; ++r )
	{
		for ( int c = 0; c < WINDOW_HEIGHT; ++c )
		{
			( p0++ )->pixel = 0xffff0000;
			( p1 )->b = 0x00; ( p1 )->g = 0xff; ( p1 )->r = 0x00; ++p1;
			( p2++ )->depth = 0.3f;
			( p3++ )->stencil = 200;
		}
	}

	if ( NativeInitialize() )
	{
		pMain = NativeCreateWindow(L"Blit Test (Press 1/2/3/4)", WINDOW_WIDTH, WINDOW_HEIGHT);

		if ( pMain )
		{
			NativeRegisterKeyboardCallbacks(pMain, &cbKeyboard);

			while ( NativeGetWindowCount() > 0 )
			{
				NativeInputPoll();
				Sleep(10);
			}
			NativeDestroyWindow(pMain);
		}

		NativeTerminate();
	}
}
void		TestNative_MultipleWindow()
{
	memset(pWindowGroup, 0, sizeof(pWindowGroup));

	if ( NativeInitialize() )
	{
		wsprintf(bufWindowTitle, L"Window %d", 0);
		pWindowGroup[ 0 ] = NativeCreateWindow(bufWindowTitle, 300, 200, 200, 200);
		wsprintf(bufWindowTitle, L"Window %d", 1);
		pWindowGroup[ 1 ] = NativeCreateWindow(bufWindowTitle, 300, 200, 600, 200);
		wsprintf(bufWindowTitle, L"Window %d", 2);
		pWindowGroup[ 2 ] = NativeCreateWindow(bufWindowTitle, 300, 200, 200, 500);
		wsprintf(bufWindowTitle, L"Window %d", 3);
		pWindowGroup[ 3 ] = NativeCreateWindow(bufWindowTitle, 300, 200, 600, 500);

		while ( NativeGetWindowCount() > 0 )
		{
			NativeInputPoll();
			Sleep(10);
		}

		for (NativeWindow * p : pWindowGroup)
		{
			NativeDestroyWindow(p);
		}

		NativeTerminate();
	}
}