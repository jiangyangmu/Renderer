#include "RenderWindow.h"
#include "Renderer.h"
#include "Buffer.h"

#include <strsafe.h>

#include <algorithm>

#ifdef max // conflict with std::max
#undef max
#endif

namespace Graphics
{
	RenderWindow::RenderWindow(LPCWSTR lpTitle, HINSTANCE hInstance, int nWidth, int nHeight)
		: Window(lpTitle, hInstance, nWidth, nHeight)
		, m_refRender(nullptr)
	{
		ENSURE_TRUE(QueryPerformanceFrequency(&m_timerFrequence));

		m_timerBegin.QuadPart = m_timerEnd.QuadPart = 0;
		m_drawnFrame = m_presentedFrame = 0;
		m_lastFrameCostMS = m_minFrameCostMS = 1000.0 / 220.0; // 220 max FPS
		m_debugMode = false;

		_BIND_EVENT(OnWndIdle, *this, *this);

		_BIND_EVENT(OnMouseMove, *this, *this);
		_BIND_EVENT(OnMouseLButtonDown, *this, *this);
		_BIND_EVENT(OnMouseLButtonUp, *this, *this);
	}

	void RenderWindow::Paint(LONG width, LONG height, LPVOID data)
	{
		HWND hWnd = GetHWND();
		HDC hdc = GetDC(hWnd);

		BITMAPINFOHEADER bi;
		bi.biSize		= sizeof(BITMAPINFOHEADER);
		bi.biWidth		= width;
		bi.biHeight		= -height;
		bi.biPlanes		= 1;
		bi.biBitCount		= 24;
		bi.biCompression	= BI_RGB;
		bi.biSizeImage		= 0;
		bi.biXPelsPerMeter	= 0;
		bi.biYPelsPerMeter	= 0;
		bi.biClrUsed		= 0;
		bi.biClrImportant	= 0;

		if ( !SetDIBitsToDevice(hdc,
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
			ReleaseDC(hWnd, hdc);
			ExitProcess(0);
		}

		ReleaseDC(hWnd, hdc);

		++m_presentedFrame;
	}

	bool RenderWindow::QueryInterface(Integer guid, void ** ppInterface)
	{
		if (_INTERFACE_IID(RenderWindow) == guid)
		{
			*ppInterface = this;
			return true;
		}
		else
		{
			return false;
		}
	}

	void RenderWindow::SetRenderer(IRenderer & renderer)
	{
		m_refRender = &renderer;
	}

	_RECV_EVENT_IMPL(RenderWindow, OnWndIdle) ( void * sender )
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
			m_refRender->Present();

			// Show frame N debug info
			{
				static double totalMS = 0.0;
				static TCHAR strBuf[ 1024 ];
				if ( ( totalMS += std::max(m_minFrameCostMS, m_lastFrameCostMS) ) > 500.0 ) // update once every 0.5 second
				{
					auto pos = V3Zero();//static_cast< SceneRenderable * >( m_refRender->GetRenderable(0).get() )->GetSceneState().camera->GetPos();
					float xDbg = 0.0f; //m_refRender->GetRenderContext().GetConstants().DebugPixel[ 0 ];
					float yDbg = 0.0f; //m_refRender->GetRenderContext().GetConstants().DebugPixel[ 1 ];

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
							xDbg,
							yDbg);

					SetWindowText(GetHWND(), strBuf);

					totalMS = 0.0;
				}
			}
		}

		// Count frame N+1 cost - begin
		ENSURE_TRUE(QueryPerformanceCounter(&m_timerBegin));

		// Reset drawing surface
		m_refRender->Clear();

		// Update and draw frame N+1
		m_refRender->Update(m_minFrameCostMS);
		m_refRender->Draw();
		++m_drawnFrame;
	}

	_RECV_EVENT_IMPL(RenderWindow, OnWndResize) ( void * sender, const win32::WindowRect & args )
	{
		float aspectRatio = ((float)args.width) / args.height;
		_DISPATCH_EVENT1(OnAspectRatioChange, *this, aspectRatio);
	}

	_RECV_EVENT_IMPL(RenderWindow, OnMouseMove) ( void * sender, const win32::MouseEventArgs & args )
	{
		if ( m_debugMode )
		{
			//m_refRender->GetRenderContext().GetConstants().DebugPixel[ 0 ] = args.pixelX;
			//m_refRender->GetRenderContext().GetConstants().DebugPixel[ 1 ] = args.pixelY;
		}
	}

	_RECV_EVENT_IMPL(RenderWindow, OnMouseLButtonDown) ( void * sender, const win32::MouseEventArgs & args )
	{
		//m_refRender->GetRenderContext().GetConstants().DebugPixel[ 0 ] = args.pixelX;
		//m_refRender->GetRenderContext().GetConstants().DebugPixel[ 1 ] = args.pixelY;
		m_debugMode = true;
	}

	_RECV_EVENT_IMPL(RenderWindow, OnMouseLButtonUp) ( void * sender, const win32::MouseEventArgs & args )
	{
		m_debugMode = false;
		//m_refRender->GetRenderContext().GetConstants().DebugPixel[ 0 ] = -1;
		//m_refRender->GetRenderContext().GetConstants().DebugPixel[ 1 ] = -1;
	}
}