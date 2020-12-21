#include <cassert>

#include <Windows.h>
#include <Gdiplus.h>

#include "Win32App.h"
#include "../../../Source/Renderer.h"

int	ShowBitmap(HWND hWnd, HDC hdcWindow, Renderer::RenderResult & rr)
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

namespace win32
{
	class RendererWindow : public Window
	{
	public:
		using Window::Window;

		void		OnWndPaint(HDC hdc) override
		{
			if ( !m_renderResult )
			{
				m_renderResult = Renderer::RenderResult::Create();
				m_renderResult->Draw();
			}
			ShowBitmap(GetHWND(), hdc, *m_renderResult);
		}
		virtual void    OnMouseLButtonDown(int pixelX, int pixelY, DWORD flags) override
		{
			if ( m_renderResult )
			{
				m_renderResult->SetDebugPixel(pixelX, pixelY);
				m_renderResult->Draw();
				//if ( !UpdateWindow(GetHWND()) )
				//{
				//	MessageBox(GetHWND(), L"UpdateWindow has failed", L"Failed", MB_OK);
				//}
			}
		}

	private:
		std::unique_ptr<Renderer::RenderResult>	m_renderResult;
	};
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
		      _In_opt_ HINSTANCE hPrevInstance,
		      _In_ LPWSTR lpCmdLine,
		      _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	auto status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	assert(status == Gdiplus::Ok);

	int ret;
	{
		win32::RendererWindow wnd(L"My Renderer", hInstance);
		ret = win32::Application::Run(wnd);
	}

	// Uninitialize GDI+
	Gdiplus::GdiplusShutdown(gdiplusToken);
	
	return ret;
}
