#include "Renderer.h"
#include "Common.h"
#include "win32/Win32App.h"

#include <Windows.h>
#include <Gdiplus.h>

#include <algorithm>
#include <sstream>

#ifdef max // conflict with std::max
#undef max
#endif

int	ShowBitmap(HWND hWnd, HDC hdcWindow, Rendering::HardcodedRenderer & rr)
{
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = rr.Width();
	bi.biHeight = -( LONG ) rr.Height();
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	if ( !SetDIBitsToDevice(hdcWindow,
				0,
				0,
				rr.Width(),
				rr.Height(),
				0,
				0,
				0,
				rr.Height(),
				( void * ) rr.GetFrontBuffer(),
				( BITMAPINFO * ) &bi,
				DIB_RGB_COLORS) )
	{
		MessageBox(hWnd, L"SetDIBitsToDevice has failed", L"Failed", MB_OK);
	}

	return 0;
}

class RendererWindow : public win32::Window
{
public:
	RendererWindow(LPCWSTR lpTitle, HINSTANCE hInstance)
		: Window(lpTitle, hInstance)
		, m_render(Rendering::HardcodedRenderer::Create())
	{
		ENSURE_TRUE(
			QueryPerformanceFrequency(&m_timerFrequence));
		m_timerPreviousValue.QuadPart = 0;
		m_fps = 60.0;
		m_frame = 0;

		_BIND_EVENT(OnWndIdle, *this, *this);
		_BIND_EVENT(OnWndPaint, *this, *this);
		_BIND_EVENT(OnMouseLButtonUp, *this, *this);
	}

public: _RECV_EVENT_DECL(RendererWindow, OnWndIdle);
public: _RECV_EVENT_DECL1(RendererWindow, OnWndPaint);
public: _RECV_EVENT_DECL1(RendererWindow, OnMouseLButtonUp);

private:
	std::unique_ptr<Rendering::HardcodedRenderer>	m_render;

	LARGE_INTEGER					m_timerFrequence;
	LARGE_INTEGER					m_timerPreviousValue;
	LONG						m_frame;
	double						m_fps;
};

_RECV_EVENT_IMPL(RendererWindow, OnWndIdle) ( void * sender )
{
	UNREFERENCED_PARAMETER(sender);

	// Throttle FPS
	double elapsedMilliSeconds;
	if ( m_timerPreviousValue.QuadPart != 0 )
	{
		LARGE_INTEGER delta;

		delta = m_timerPreviousValue;

		ENSURE_TRUE(
			QueryPerformanceCounter(&m_timerPreviousValue));

		delta.QuadPart = m_timerPreviousValue.QuadPart - delta.QuadPart;
		delta.QuadPart *= 1000;
		delta.QuadPart /= m_timerFrequence.QuadPart;

		elapsedMilliSeconds = static_cast< double >( delta.QuadPart );
	}
	else
	{
		ENSURE_TRUE(
			QueryPerformanceCounter(&m_timerPreviousValue));

		elapsedMilliSeconds = 16.0;
	}

	m_fps = 0.5 * m_fps + 0.5 * 1000.0 / elapsedMilliSeconds;
	{
		std::wstringstream ss;
		ss << L"FPS: " << m_fps << " ms: " << elapsedMilliSeconds << " frame: " << m_frame;
		SetWindowText(GetHWND(), ss.str().c_str());
		Sleep(static_cast< DWORD >( std::max(0.0, 16.0 - elapsedMilliSeconds) ));
	}

	// Reset drawing surface
	m_render->ClearSurface();

	// Update and draw next frame
	m_render->Update(elapsedMilliSeconds);
	m_render->Draw();

	// Present next frame
	m_render->SwapBuffer();

	ENSURE_TRUE(InvalidateRect(GetHWND(), NULL, TRUE));
}
_RECV_EVENT_IMPL(RendererWindow, OnWndPaint) ( void * sender, const win32::WindowPaintArgs & args )
{
	if ( m_render )
	{
		ShowBitmap(GetHWND(), args.hdc, *m_render);
		++m_frame;
	}
}
_RECV_EVENT_IMPL(RendererWindow, OnMouseLButtonUp) ( void * sender, const win32::MouseEventArgs & args )
{
	if ( m_render )
	{
		m_render->SetDebugPixel(args.pixelX, args.pixelY);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
		      _In_opt_ HINSTANCE hPrevInstance,
		      _In_ LPWSTR lpCmdLine,
		      _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	win32::Application app;

	int ret;
	{
		RendererWindow wnd(L"My Renderer", hInstance);
		ret = app.Run(wnd);
	}
	
	return ret;
}
