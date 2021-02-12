#include "../Core/Native.h"

#include <WindowsX.h>
#include <Windows.h>
#include <Gdiplus.h>
#include <crtdbg.h>

#include <stdio.h>
#include <assert.h>

// Constants

#define NUM_MAX_WINDOW (10)
#define NUM_MAX_EVENT_PER_POLL (10)
#define BYTES_PER_PIXEL (4)

static LPCWSTR		StrWndClassName = L"Win32 Window Class";
static UINT		DwWndClassStyle = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
static DWORD		DwWndStyle = WS_CAPTION;

// Structures

struct NativeWindow
{
	HWND		hWnd;

	int		nWidth;
	int		nHeight;

	// Surface
	HDC		hWndDC;
	HDC		hMemDC;
	HBITMAP		hBmp;
	void *		pPixels;

	NativeWindowCallbacks	cbWindow;
	NativeKeyboardCallbacks cbKeyboard;
	NativeMouseCallbacks	cbMouse;
};

struct NativeWin32
{
	HINSTANCE	hInstance;

	bool		bInitialized;

	// windows
	NativeWindow	sWindows[ NUM_MAX_WINDOW ];
	bool		bWindows[ NUM_MAX_WINDOW ];
};

// Globals

static NativeWin32 native =
{
	NULL,
	false,
	{},
	{},
};
ULONG_PTR tkGdiPlus = NULL;

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
	for ( int i = 0; i < NUM_MAX_WINDOW; ++i )
	{
		if ( !native.bWindows[ i ] ) ++nCount;
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
	assert(native.sWindows <= pWindow && pWindow < ( native.sWindows + NUM_MAX_WINDOW ));
	assert(pWindow->hWnd == NULL);

	native.bWindows[ pWindow - native.sWindows ] = false;
}

static void		_CreateSurface(NativeWindow * pWindow)
{
	BITMAPINFOHEADER bi;
	HDC hWndDC;
	HDC hMemDC;
	HBITMAP hBmp;
	void * pPixels;

	memset(&bi, 0, sizeof(BITMAPINFOHEADER));
	bi.biSize		= sizeof(BITMAPINFOHEADER);
	bi.biWidth		= pWindow->nWidth;
	bi.biHeight		= -pWindow->nHeight;
	bi.biPlanes		= 1;
	bi.biBitCount		= BYTES_PER_PIXEL * 8;
	bi.biCompression	= BI_RGB;

	hWndDC			= GetDC(pWindow->hWnd);
	hMemDC			= CreateCompatibleDC(hWndDC);

	hBmp			= CreateDIBSection(hMemDC,
						   ( const BITMAPINFO * ) &bi,
						   DIB_RGB_COLORS,
						   &pPixels,
						   NULL,
						   0);
	assert(hMemDC);
	assert(hBmp && pPixels);

	DeleteObject(SelectObject(hMemDC, hBmp));

	pWindow->hWndDC = hWndDC;
	pWindow->hMemDC = hMemDC;
	pWindow->hBmp = hBmp;
	pWindow->pPixels = pPixels;
}
static void		_DestroySurface(NativeWindow * pWindow)
{
	DeleteObject(pWindow->hBmp);
	DeleteDC(pWindow->hMemDC);
	ReleaseDC(pWindow->hWnd, pWindow->hWndDC);

	pWindow->hBmp = NULL;
	pWindow->hMemDC = NULL;
	pWindow->hWndDC = NULL;
	pWindow->pPixels = NULL;
}
static bool		_PresentSurface(HWND hWnd, HDC hMemDC, int nWidth, int nHeight)
{
	HDC hDC;
	bool bRet;

	hDC	= GetDC(hWnd);

	bRet	= BitBlt(hDC,
			 0,
			 0,
			 nWidth,
			 nHeight,
			 hMemDC,
			 0,
			 0,
			 SRCCOPY);

	ReleaseDC(hWnd, hDC);

	return bRet;
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

	if ( native.hInstance == NULL )
	{
		return false;
	}

	native.bInitialized = true;
	memset(native.sWindows, 0, sizeof(native.sWindows));
	memset(native.bWindows, 0, sizeof(native.bWindows));

	( void ) _RegisterWindowClass();

	// GDI+
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup(&tkGdiPlus, &input, NULL);

	return true;
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

	// GDI+
	Gdiplus::GdiplusShutdown(tkGdiPlus);
}

NativeWindow *		NativeCreateWindow(const wchar_t * pWindowTitle, int nWidth, int nHeight)
{
	int nScreenWidth;
	int nScreenHeight;

	assert(pWindowTitle);
	assert(nWidth > 0 && nHeight > 0);

	nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	return NativeCreateWindow(pWindowTitle, nWidth, nHeight, ( nScreenWidth - nWidth ) / 2, ( nScreenHeight - nHeight ) / 2);
}
NativeWindow *		NativeCreateWindow(const wchar_t * pWindowTitle, int nWidth, int nHeight, int nLeft, int nTop)
{
	NativeWindow * pWindow;
	RECT rect;

	assert(pWindowTitle);
	assert(nWidth > 0 && nHeight > 0);

	rect.left = nLeft;
	rect.right = rect.left + nWidth;
	rect.top = nTop;
	rect.bottom = rect.top + nHeight;

	AdjustWindowRect(&rect, DwWndStyle, false);

	pWindow = _AllocWindow();
	memset(pWindow, 0, sizeof(NativeWindow));
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
	if ( !pWindow->hWnd )
	{
		return NULL;
	}

	pWindow->nWidth = nWidth;
	pWindow->nHeight = nHeight;

	_CreateSurface(pWindow);

	SetWindowText(pWindow->hWnd, pWindowTitle);
	ShowWindow(pWindow->hWnd, SW_SHOW);

	return pWindow;
}
void			NativeDestroyWindow(NativeWindow * pWindow)
{
	if ( pWindow->hWnd )
	{
		_DestroySurface(pWindow);

		DestroyWindow(pWindow->hWnd);
		pWindow->hWnd = NULL;
	}

	_ReleaseWindow(pWindow);
}
void			NativeDestroyAllWindows()
{
	for (int i = 0; i < NUM_MAX_WINDOW; ++i)
	{
		if (native.bWindows[i])
		{
			NativeDestroyWindow(native.sWindows + i);
		}
	}
}

int			NativeGetWindowCount()
{
	return NUM_MAX_WINDOW - _CountFreeWindow();
}

int			NativeWindowGetWidth(NativeWindow * pWindow)
{
	return pWindow->nWidth;
}
int			NativeWindowGetHeight(NativeWindow * pWindow)
{
	return pWindow->nHeight;
}

static bool		_NativeWindowBilt(NativeWindow * pWindow, const void * pSrc, NativeBlitMode mode)
{
	const int nWidth = pWindow->nWidth;
	const int nHeight = pWindow->nHeight;

	const BYTE * pbSrc;
	const float * pfSrc;
	DWORD * pDst;
	bool bBadMode;

	assert(( mode & NATIVE_BLIT_COLOR_MASK ) == mode);

	pDst = ( DWORD * ) pWindow->pPixels;
	bBadMode = false;
	switch ( mode )
	{
		case NATIVE_BLIT_BGRA:
			memcpy(pDst,
			       pSrc,
			       ( size_t ) nWidth * nHeight * BYTES_PER_PIXEL);
			break;
		case NATIVE_BLIT_BGR:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					*pDst = ( 0xff000000 ) |
						( pbSrc[ 2 ] << 16 ) |
						( pbSrc[ 1 ] << 8 ) |
						( pbSrc[ 0 ] );

					++pDst;
					pbSrc += 3;
				}
			}
			break;
		case NATIVE_BLIT_F32:
			pfSrc	= ( const float * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = ( BYTE ) ( *pfSrc * 255.0f );

					*pDst = ( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pDst;
					++pfSrc;
				}
			}
			break;
		case NATIVE_BLIT_U8:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = *pbSrc;

					*pDst = ( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pDst;
					++pbSrc;
				}
			}
			break;
		default:
			bBadMode = true;
			break;
	}

	return !bBadMode;
}
static bool		_NativeWindowBiltFlipH(NativeWindow * pWindow, const void * pSrc, NativeBlitMode mode)
{
	const int nWidth = pWindow->nWidth;
	const int nHeight = pWindow->nHeight;

	const DWORD * pdSrc;
	const BYTE * pbSrc;
	const float * pfSrc;
	DWORD * pDst;
	bool bBadMode;

	assert(( mode & NATIVE_BLIT_COLOR_MASK ) == mode);

	pDst = ( DWORD * ) pWindow->pPixels;
	bBadMode = false;
	switch ( mode )
	{
		case NATIVE_BLIT_BGRA:
			pdSrc = ( const DWORD * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					pDst[ r * nWidth + nWidth - 1 - c ] = pdSrc[ r * nWidth + c ];
				}
			}
			break;
		case NATIVE_BLIT_BGR:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					pDst[ r * nWidth + nWidth - 1 - c ] =
						( 0xff000000 ) |
						( pbSrc[ 2 ] << 16 ) |
						( pbSrc[ 1 ] << 8 ) |
						( pbSrc[ 0 ] );

					pbSrc += 3;
				}
			}
			break;
		case NATIVE_BLIT_F32:
			pfSrc	= ( const float * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = ( BYTE ) ( *pfSrc * 255.0f );

					pDst[ r * nWidth + nWidth - 1 - c ] =
						( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pfSrc;
				}
			}
			break;
		case NATIVE_BLIT_U8:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = *pbSrc;

					pDst[ r * nWidth + nWidth - 1 - c ] =
						( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pbSrc;
				}
			}
			break;
		default:
			bBadMode = true;
			break;
	}

	return !bBadMode;
}
static bool		_NativeWindowBiltFlipV(NativeWindow * pWindow, const void * pSrc, NativeBlitMode mode)
{
	const int nWidth = pWindow->nWidth;
	const int nHeight = pWindow->nHeight;

	const DWORD * pdSrc;
	const BYTE * pbSrc;
	const float * pfSrc;
	DWORD * pDst;
	bool bBadMode;

	assert(( mode & NATIVE_BLIT_COLOR_MASK ) == mode);

	pDst = ( DWORD * ) pWindow->pPixels;
	bBadMode = false;
	switch ( mode )
	{
		case NATIVE_BLIT_BGRA:
			pdSrc = ( const DWORD * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					pDst[ ( nHeight - 1 - r ) * nWidth + c ] = pdSrc[ r * nWidth + c ];
				}
			}
			break;
		case NATIVE_BLIT_BGR:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					pDst[ ( nHeight - 1 - r ) * nWidth + c ] =
						( 0xff000000 ) |
						( pbSrc[ 2 ] << 16 ) |
						( pbSrc[ 1 ] << 8 ) |
						( pbSrc[ 0 ] );

					pbSrc += 3;
				}
			}
			break;
		case NATIVE_BLIT_F32:
			pfSrc	= ( const float * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = ( BYTE ) ( *pfSrc * 255.0f );

					pDst[ ( nHeight - 1 - r ) * nWidth + c ] =
						( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pfSrc;
				}
			}
			break;
		case NATIVE_BLIT_U8:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = *pbSrc;

					pDst[ ( nHeight - 1 - r ) * nWidth + c ] =
						( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pbSrc;
				}
			}
			break;
		default:
			bBadMode = true;
			break;
	}

	return !bBadMode;
}
static bool		_NativeWindowBiltFlipHV(NativeWindow * pWindow, const void * pSrc, NativeBlitMode mode)
{
	const int nWidth = pWindow->nWidth;
	const int nHeight = pWindow->nHeight;

	const DWORD * pdSrc;
	const BYTE * pbSrc;
	const float * pfSrc;
	DWORD * pDst;
	bool bBadMode;

	assert(( mode & NATIVE_BLIT_COLOR_MASK ) == mode);

	pDst = ( DWORD * ) pWindow->pPixels;
	bBadMode = false;
	switch ( mode )
	{
		case NATIVE_BLIT_BGRA:
			pdSrc = ( const DWORD * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					pDst[ ( nHeight - 1 - r ) * nWidth + nWidth - 1 - c ] = pdSrc[ r * nWidth + c ];
				}
			}
			break;
		case NATIVE_BLIT_BGR:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					pDst[ ( nHeight - 1 - r ) * nWidth + nWidth - 1 - c ] =
						( 0xff000000 ) |
						( pbSrc[ 2 ] << 16 ) |
						( pbSrc[ 1 ] << 8 ) |
						( pbSrc[ 0 ] );

					pbSrc += 3;
				}
			}
			break;
		case NATIVE_BLIT_F32:
			pfSrc	= ( const float * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = ( BYTE ) ( *pfSrc * 255.0f );

					pDst[ ( nHeight - 1 - r ) * nWidth + nWidth - 1 - c ] =
						( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pfSrc;
				}
			}
			break;
		case NATIVE_BLIT_U8:
			pbSrc	= ( const BYTE * ) pSrc;
			for ( int r = 0; r < nHeight; ++r )
			{
				for ( int c = 0; c < nWidth; ++c )
				{
					BYTE grey = *pbSrc;

					pDst[ ( nHeight - 1 - r ) * nWidth + nWidth - 1 - c ] =
						( 0xff000000 ) |
						( grey << 16 ) |
						( grey << 8 ) |
						( grey );

					++pbSrc;
				}
			}
			break;
		default:
			bBadMode = true;
			break;
	}

	return !bBadMode;
}
bool			NativeWindowBilt(NativeWindow * pWindow, const void * pSrc, int mode)
{
	NativeBlitMode mColor = (NativeBlitMode)(mode & NATIVE_BLIT_COLOR_MASK);
	bool bSuccess;

	switch ((mode >> 30) & 3)
	{
		case 0: bSuccess = _NativeWindowBilt(pWindow, pSrc, mColor); break;
		case 1: bSuccess = _NativeWindowBiltFlipH(pWindow, pSrc, mColor); break;
		case 2: bSuccess = _NativeWindowBiltFlipV(pWindow, pSrc, mColor); break;
		case 3: bSuccess = _NativeWindowBiltFlipHV(pWindow, pSrc, mColor); break;
	}

	return bSuccess && _PresentSurface(pWindow->hWnd, pWindow->hMemDC, pWindow->nWidth, pWindow->nHeight);
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

int64_t			NativeGetTick()
{
	static LARGE_INTEGER liFrequence = { 0 };
	static LARGE_INTEGER liBegin = { 0 };
	LARGE_INTEGER liEnd;
	
	if (liFrequence.QuadPart == 0)
	{
		QueryPerformanceFrequency(&liFrequence);
		QueryPerformanceCounter(&liBegin);
		return 0;
	}
	else
	{
		QueryPerformanceCounter(&liEnd);
		return (liEnd.QuadPart - liBegin.QuadPart) * 1000000 / liFrequence.QuadPart;
	}
}

void *			AlignedMalloc(size_t nSize, size_t nAlign)
{
#ifdef _DEBUG
	return _aligned_malloc_dbg(nSize, nAlign, __FILE__, __LINE__);
#else
	return _aligned_malloc(nSize, nAlign);
#endif
}
void			AlignedFree(void * p)
{
#ifdef _DEBUG
	return _aligned_free_dbg(p);
#else
	return _aligned_free(p);
#endif
}

// Image

void			NativeLoadBmp(const wchar_t * pBmpFile, int * pWidth, int * pHeight, void ** ppPixels)
{
	Gdiplus::Bitmap * pBmp;
	Gdiplus::Color color;
	UINT w;
	UINT h;
	DWORD * pixels;
	DWORD * pixel;

	pBmp	= Gdiplus::Bitmap::FromFile(pBmpFile, false);
	assert(pBmp);

	w	= pBmp->GetWidth();
	h	= pBmp->GetHeight();

	pixels	= new DWORD[ w * h ];
	pixel	= pixels;
	assert(pixels);

	for ( int y = 0; y < h; ++y )
	{
		for ( int x = 0; x < w; ++x )
		{
			pBmp->GetPixel(x, h - y - 1, &color);
			*( pixel++ ) = color.GetValue();
		}
	}

	delete pBmp;

	*pWidth	= static_cast< int >( w );
	*pHeight = static_cast< int >( h );
	*ppPixels = pixels;
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
	printf("Key Down:   %d(%c)\n", keycode, isprint(keycode) ? (char)keycode : '?');
}
static void		DebugKeyboard_Up(int keycode)
{
	printf("Key Up:     %d(%c)\n", keycode, isprint(keycode) ? (char)keycode : '?');
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

const NativeWindowCallbacks *		NativeDebugGetWindowCallbacks()
{
	static NativeWindowCallbacks cbs;
	static bool init = false;
	if ( !init )
	{
		cbs.move =   &DebugWindow_Move;
		cbs.resize = &DebugWindow_Resize;
		cbs.close =  &DebugWindow_Close;
		init = true;
	}
	return &cbs;
}
const NativeKeyboardCallbacks *		NativeDebugGetKeyboardCallbacks()
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
const NativeMouseCallbacks *		NativeDebugGetMouseCallbacks()
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