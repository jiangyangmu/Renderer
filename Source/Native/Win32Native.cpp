#include "../Core/Native.h"

#include <WindowsX.h>
#include <Windows.h>

#include <stdio.h>
#include <assert.h>

// Constants

#define NUM_MAX_WINDOW 10
#define NUM_MAX_EVENT_PER_POLL 10

static LPCWSTR		StrWndClassName = L"Win32 Window Class";
static UINT		DwWndClassStyle = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
static DWORD		DwWndStyle = WS_CAPTION;

// Structures

struct NativeWindow
{
	HWND		hWnd;
	int		nWidth;
	int		nHeight;
	
	NativeWindowCallbacks	cbWindow;
	NativeKeyboardCallbacks cbKeyboard;
	NativeMouseCallbacks	cbMouse;
};

struct NativeWin32
{
	HINSTANCE	hInstance;
	
	bool		bInitialized;
	
	// windows
	NativeWindow	sWindows[NUM_MAX_WINDOW];
	bool		bWindows[NUM_MAX_WINDOW];
};

// Globals

static NativeWin32 native =
{
	NULL,
	false,
	{},
	{},
};

// Methods

static LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static bool		_RegisterWindowClass()
{
	WNDCLASS wndClass;
	wndClass.style		= DwWndClassStyle;
	wndClass.lpfnWndProc	= _WindowProc;
	wndClass.cbClsExtra	= 0;
	wndClass.cbWndExtra	= 0;
	wndClass.hInstance	= native.hInstance;
	wndClass.hIcon		= NULL;
	wndClass.hCursor	= NULL;
	wndClass.hbrBackground	= NULL;
	wndClass.lpszMenuName	= NULL;
	wndClass.lpszClassName	= StrWndClassName;

	return RegisterClass(&wndClass) != NULL;
}
static int		_CountFreeWindow()
{
	int nCount = 0;
	for (int i = 0; i < NUM_MAX_WINDOW; ++i)
	{
		if (!native.bWindows[i]) ++nCount;
	}
	return nCount;
}
static NativeWindow *	_AllocWindow()
{
	NativeWindow * pWindow = NULL;

	for ( int i = 0; i < NUM_MAX_WINDOW; ++i )
	{
		if ( !native.bWindows[ i ] )
		{
			native.bWindows[ i ] = true;
			pWindow = native.sWindows + i;
			break;
		}
	}

	return pWindow;
}
static void		_ReleaseWindow(NativeWindow * pWindow)
{
	assert(native.sWindows <= pWindow && pWindow < (native.sWindows + NUM_MAX_WINDOW));
	assert(pWindow->hWnd == NULL);

	native.bWindows[pWindow - native.sWindows] = false;
}

static LRESULT CALLBACK _WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NativeWindow * pWindow = NULL;
	LRESULT lpResult = NULL;
	bool bCallDefault = false;

	if ( WM_NCCREATE == uMsg )
	{
		// Bind pWindow and hWnd
		pWindow = ( NativeWindow * ) ( ( CREATESTRUCT * ) lParam )->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, ( LONG_PTR ) pWindow);

		return TRUE;
	}
	else if ( WM_NCDESTROY == uMsg )
	{
		// Unbind pWindow and hWnd
		pWindow = ( NativeWindow * ) GetWindowLongPtr(hWnd, GWLP_USERDATA);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);

		return 0;
	}
	else
	{
		pWindow = ( NativeWindow * ) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	switch ( uMsg )
	{
		case WM_KEYDOWN:	if ( pWindow->cbKeyboard.down ) pWindow->cbKeyboard.down(wParam); break;
		case WM_KEYUP:		if ( pWindow->cbKeyboard.up ) pWindow->cbKeyboard.up(wParam); break;
		case WM_LBUTTONDOWN:	if ( pWindow->cbMouse.leftdown ) pWindow->cbMouse.leftdown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
		case WM_LBUTTONUP:	if ( pWindow->cbMouse.leftup ) pWindow->cbMouse.leftup(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
		case WM_RBUTTONDOWN:	if ( pWindow->cbMouse.rightdown ) pWindow->cbMouse.rightdown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
		case WM_RBUTTONUP:	if ( pWindow->cbMouse.rightup ) pWindow->cbMouse.rightup(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
		case WM_MBUTTONDOWN:	if ( pWindow->cbMouse.middledown ) pWindow->cbMouse.middledown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
		case WM_MBUTTONUP:	if ( pWindow->cbMouse.middleup ) pWindow->cbMouse.middleup(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
		case WM_MOUSEMOVE:	if ( pWindow->cbMouse.move ) pWindow->cbMouse.move(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); break;
		case WM_MOVE:		if ( pWindow->cbWindow.move ) pWindow->cbWindow.move(LOWORD(lParam), HIWORD(lParam)); break;
		case WM_SIZE:		if ( pWindow->cbWindow.resize ) pWindow->cbWindow.resize(LOWORD(lParam), HIWORD(lParam)); break;
		case WM_CLOSE:		if ( pWindow->cbWindow.close ) pWindow->cbWindow.close(); bCallDefault = true; break;
		case WM_DESTROY:	pWindow->hWnd = NULL; _ReleaseWindow(pWindow); bCallDefault = true; break;
		default:		bCallDefault = true; break;
	}

	return bCallDefault ? DefWindowProc(hWnd, uMsg, wParam, lParam) : lpResult;
}

bool			NativeInitialize()
{
	if ( native.bInitialized )
	{
		return true;
	}

	native.hInstance = GetModuleHandle(NULL);

	if ( native.hInstance != NULL )
	{
		native.bInitialized = true;
		memset(native.sWindows, 0, sizeof(native.sWindows));
		memset(native.bWindows, 0, sizeof(native.bWindows));

		( void ) _RegisterWindowClass();
	}

	return native.bInitialized;
}
void			NativeTerminate()
{
	if ( !native.bInitialized )
	{
		return;
	}

	for ( int i = 0; i < NUM_MAX_WINDOW; ++i )
	{
		if ( native.bWindows[ i ] )
		{
			NativeDestroyWindow(native.sWindows + i);
			native.bWindows[ i ] = false;
		}
	}
}

NativeWindow *		NativeCreateWindow(const wchar_t * pWindowTitle, int nWidth, int nHeight)
{
	NativeWindow * pWindow;
	int nScreenWidth;
	int nScreenHeight;
	RECT rect;

	assert(pWindowTitle);
	assert(nWidth > 0 && nHeight > 0);

	nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	rect.left = (nScreenWidth - nWidth) / 2;
	rect.right = rect.left + nWidth;
	rect.top = (nScreenHeight - nHeight) / 2;
	rect.bottom = rect.top + nHeight;
	AdjustWindowRect(&rect, DwWndStyle, false);

	pWindow = _AllocWindow();
	pWindow->hWnd = CreateWindow(StrWndClassName,
				     pWindowTitle,
				     WS_OVERLAPPEDWINDOW,
				     rect.left,
				     rect.top,
				     rect.right - rect.left,
				     rect.bottom - rect.top,
				     NULL,
				     NULL,
				     native.hInstance,
				     ( LPVOID ) pWindow);
	pWindow->nWidth = nWidth;
	pWindow->nHeight = nHeight;

	if (pWindow->hWnd)
	{
		SetWindowText(pWindow->hWnd, pWindowTitle);
		ShowWindow(pWindow->hWnd, SW_SHOW);
	}

	return pWindow->hWnd ? pWindow : NULL;
}
void			NativeDestroyWindow(NativeWindow * pWindow)
{
	if ( pWindow->hWnd )
	{
		DestroyWindow(pWindow->hWnd);
		pWindow->hWnd = NULL;
	}

	_ReleaseWindow(pWindow);
}
int			NativeGetWindowCount()
{
	return NUM_MAX_WINDOW - _CountFreeWindow();
}

void			NativeRegisterWindowCallbacks(NativeWindow * pWindow, const NativeWindowCallbacks * pCallbacks)
{
	assert(pCallbacks);

	pWindow->cbWindow = *pCallbacks;
}
void			NativeRegisterKeyboardCallbacks(NativeWindow * pWindow, const NativeKeyboardCallbacks * pCallbacks)
{
	assert(pCallbacks);

	pWindow->cbKeyboard = *pCallbacks;
}
void			NativeRegisterMouseCallbacks(NativeWindow * pWindow, const NativeMouseCallbacks * pCallbacks)
{
	assert(pCallbacks);

	pWindow->cbMouse = *pCallbacks;
}

void			NativeInputPoll()
{
	MSG msg;

	do
	{
		msg.message = WM_NULL;
		PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);

		if ( msg.message == WM_NULL )
		{
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	while ( msg.message != WM_QUIT );
}

// Debug

static void		DebugWindow_Move(int x, int y)
{
	printf("Wnd Move:   %d, %d\n", x, y);
}
static void		DebugWindow_Resize(int width, int height)
{
	printf("Wnd Resize: %d, %d\n", width, height);
}
static void		DebugWindow_Close()
{
	printf("Wnd Close.\n");
}

static void		DebugKeyboard_Down(int keycode)
{
	printf("Key Down:   %d(%c)\n", keycode, (char)keycode);
}
static void		DebugKeyboard_Up(int keycode)
{
	printf("Key Up:     %d(%c)\n", keycode, (char)keycode);
}

static void		DebugMouse_Move(int x, int y)
{
	printf("Mse Move:   %d, %d\n", x, y);
}
static void		DebugMouse_LeftDown(int x, int y)
{
	printf("Mse L Down: %d, %d\n", x, y);
}
static void		DebugMouse_LeftUp(int x, int y)
{
	printf("Mse L Up:   %d, %d\n", x, y);
}
static void		DebugMouse_RightDown(int x, int y)
{
	printf("Mse R Down: %d, %d\n", x, y);
}
static void		DebugMouse_RightUp(int x, int y)
{
	printf("Mse R Up:   %d, %d\n", x, y);
}
static void		DebugMouse_MiddleDown(int x, int y)
{
	printf("Mse M Down: %d, %d\n", x, y);
}
static void		DebugMouse_MiddleUp(int x, int y)
{
	printf("Mse M Up:   %d, %d\n", x, y);
}

const NativeWindowCallbacks *		NativeGetDebugWindowCallbacks()
{
	static NativeWindowCallbacks cbs;
	static bool init = false;
	if (!init)
	{
		cbs.move =   &DebugWindow_Move;
		cbs.resize = &DebugWindow_Resize;
		cbs.close =  &DebugWindow_Close;
		init = true;
	}
	return &cbs;
}
const NativeKeyboardCallbacks *		NativeGetDebugKeyboardCallbacks()
{
	static NativeKeyboardCallbacks cbs;
	static bool init = false;
	if ( !init )
	{
		cbs.down = &DebugKeyboard_Down;
		cbs.up = &DebugKeyboard_Up;
		init = true;
	}
	return &cbs;
}
const NativeMouseCallbacks *		NativeGetDebugMouseCallbacks()
{
	static NativeMouseCallbacks cbs;
	static bool init = false;
	if ( !init )
	{
		cbs.move = &DebugMouse_Move;
		cbs.leftdown = &DebugMouse_LeftDown;
		cbs.leftup = &DebugMouse_LeftUp;
		cbs.rightdown = &DebugMouse_RightDown;
		cbs.rightup = &DebugMouse_RightUp;
		cbs.middledown = &DebugMouse_MiddleDown;
		cbs.middleup = &DebugMouse_MiddleUp;
		init = true;
	}
	return &cbs;
}