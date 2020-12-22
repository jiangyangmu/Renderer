#include <cassert>

#include <Windows.h>
#include <Gdiplus.h>

#include "win32/Win32App.h"
#include "Renderer.h"
#include "Common.h"

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
	using Window::Window;

	void		OnWndPaint(HDC hdc) override
	{
		if ( !m_render )
		{
			m_render = Rendering::HardcodedRenderer::Create();
			m_render->Draw(0);
			m_render->SwapBuffer();
		}
		ShowBitmap(GetHWND(), hdc, *m_render);
	}
	virtual void    OnMouseLButtonDown(int pixelX, int pixelY, DWORD flags) override
	{
		if ( m_render )
		{
			m_render->SetDebugPixel(pixelX, pixelY);
			m_render->Draw(0);
			m_render->SwapBuffer();
			//if ( !UpdateWindow(GetHWND()) )
			//{
			//	MessageBox(GetHWND(), L"UpdateWindow has failed", L"Failed", MB_OK);
			//}
		}
	}

private:
	std::unique_ptr<Rendering::HardcodedRenderer>	m_render;
};

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
