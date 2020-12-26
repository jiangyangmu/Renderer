#include "Renderer.h"
#include "Common.h"
#include "win32/Win32App.h"

#include <algorithm>
#include <strsafe.h>

#ifdef max // conflict with std::max
#undef max
#endif

class RendererWindow : public win32::Window
{
public:
	RendererWindow(LPCWSTR lpTitle, HINSTANCE hInstance)
		: Window(lpTitle, hInstance)
		, m_render(GetWidth(), GetHeight())
	{
		ENSURE_TRUE(QueryPerformanceFrequency(&m_timerFrequence));

		m_timerBegin.QuadPart = m_timerEnd.QuadPart = 0;
		m_drawnFrame = m_presentedFrame = 0;
		m_lastFrameCostMS = m_minFrameCostMS = 1000.0 / 220.0; // 220 max FPS
		m_debugMode = false;

		_BIND_EVENT(OnWndIdle, *this, *this);
		_BIND_EVENT(OnWndPaint, *this, *this);
		_BIND_EVENT(OnWndResize, *this, *this);
		
		_BIND_EVENT(OnMouseMove, *this, *this);
		_BIND_EVENT(OnMouseLButtonDown, *this, *this);
		_BIND_EVENT(OnMouseLButtonUp, *this, *this);

		_BIND_EVENT(OnMouseMove, *this, m_render.GetCamera().GetController());
		_BIND_EVENT(OnKeyDown, *this, m_render.GetCamera().GetController());
		_BIND_EVENT(OnKeyUp, *this, m_render.GetCamera().GetController());
	}

public: _RECV_EVENT_DECL(RendererWindow, OnWndIdle);
public: _RECV_EVENT_DECL1(RendererWindow, OnWndPaint);
public: _RECV_EVENT_DECL1(RendererWindow, OnWndResize);
public: _RECV_EVENT_DECL1(RendererWindow, OnMouseMove);
public: _RECV_EVENT_DECL1(RendererWindow, OnMouseLButtonDown);
public: _RECV_EVENT_DECL1(RendererWindow, OnMouseLButtonUp);

private:
	int Present(HWND hWnd, HDC hdcWindow, LONG width, LONG height, LPVOID data);

	// Rendering
	Rendering::HardcodedRenderer	m_render;

	// FPS control
	LARGE_INTEGER			m_timerFrequence;
	LARGE_INTEGER			m_timerBegin;
	LARGE_INTEGER			m_timerEnd;
	double				m_minFrameCostMS; // each frame spend at least this milliseconds
	double				m_lastFrameCostMS;

	// Statistics
	LONG				m_drawnFrame;
	LONG				m_presentedFrame;

	bool				m_debugMode;
};

_RECV_EVENT_IMPL(RendererWindow, OnWndIdle) ( void * sender )
{
	UNREFERENCED_PARAMETER(sender);

	bool isFirstFrame = ( m_timerBegin.QuadPart == 0 );
	if ( !isFirstFrame )
	{
		// Count frame N cost - end
		ENSURE_TRUE(QueryPerformanceCounter(&m_timerEnd));

		m_lastFrameCostMS = static_cast< double >( ( m_timerEnd.QuadPart - m_timerBegin.QuadPart ) * 1000 / m_timerFrequence.QuadPart );

		// Throttle frame rate
		if ( m_lastFrameCostMS < m_minFrameCostMS )
		{
			Sleep(static_cast< DWORD >( m_minFrameCostMS - m_lastFrameCostMS ));
		}

		// Present frame N
		m_render.GetContext().SwapBuffer();
		ENSURE_TRUE(InvalidateRect(GetHWND(), NULL, TRUE));

		// Show frame N debug info
		{
			static double totalMS = 0.0;
			static TCHAR strBuf[ 1024 ];
			if ( ( totalMS += std::max(m_minFrameCostMS, m_lastFrameCostMS) ) > 500.0 ) // update once every 0.5 second
			{
				auto pos = m_render.GetCamera().GetPos();

				StringCchPrintf(strBuf,
						1024,
						TEXT("FPS=%.2f Cost=%.2f(ms) Frame=%d/%d Pos=(%.2f, %.2f, %.2f) Debug%s=(%d, %d)"),
						1000.0 / std::max(m_minFrameCostMS, m_lastFrameCostMS),
						m_lastFrameCostMS,
						m_presentedFrame,
						m_drawnFrame,
						pos.x,
						pos.y,
						pos.z,
						( m_debugMode ? TEXT("On") : TEXT("Off") ),
						m_render.GetContext().GetConstants().DebugPixel[ 0 ],
						m_render.GetContext().GetConstants().DebugPixel[ 1 ]);

				SetWindowText(GetHWND(), strBuf);

				totalMS = 0.0;
			}
		}
	}

	// Count frame N+1 cost - begin
	ENSURE_TRUE(QueryPerformanceCounter(&m_timerBegin));

	// Reset drawing surface
	m_render.ClearSurface();

	// Update and draw frame N+1
	m_render.Update(m_minFrameCostMS);
	m_render.Draw();
	++m_drawnFrame;
}
_RECV_EVENT_IMPL(RendererWindow, OnWndPaint) ( void * sender, const win32::WindowPaintArgs & args )
{
	auto & context = m_render.GetContext();

	Present(GetHWND(),
		args.hdc,
		context.GetWidth(),
		context.GetHeight(),
		context.GetFrontBuffer().Data());
	++m_presentedFrame;
}
_RECV_EVENT_IMPL(RendererWindow, OnWndResize) ( void * sender, const win32::WindowRect & args )
{
	m_render.Resize(args.width, args.height);
}
_RECV_EVENT_IMPL(RendererWindow, OnMouseMove) ( void * sender, const win32::MouseEventArgs & args )
{
	if ( m_debugMode )
	{
		m_render.GetContext().GetConstants().DebugPixel[ 0 ] = args.pixelX;
		m_render.GetContext().GetConstants().DebugPixel[ 1 ] = args.pixelY;
	}
}
_RECV_EVENT_IMPL(RendererWindow, OnMouseLButtonDown) ( void * sender, const win32::MouseEventArgs & args )
{
	m_render.GetContext().GetConstants().DebugPixel[ 0 ] = args.pixelX;
	m_render.GetContext().GetConstants().DebugPixel[ 1 ] = args.pixelY;
	m_debugMode = true;
}
_RECV_EVENT_IMPL(RendererWindow, OnMouseLButtonUp) ( void * sender, const win32::MouseEventArgs & args )
{
	m_debugMode = false;
	m_render.GetContext().GetConstants().DebugPixel[ 0 ] = -1;
	m_render.GetContext().GetConstants().DebugPixel[ 1 ] = -1;
}
int RendererWindow::Present(HWND hWnd, HDC hdcWindow, LONG width, LONG height, LPVOID data)
{
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;
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
				width,
				height,
				0,
				0,
				0,
				height,
				data,
				( BITMAPINFO * ) &bi,
				DIB_RGB_COLORS) )
	{
		MessageBox(hWnd, L"SetDIBitsToDevice has failed", L"Failed", MB_OK);
	}

	return 0;
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
