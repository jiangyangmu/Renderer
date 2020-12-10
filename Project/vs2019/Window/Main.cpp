#include "../../../Source/Renderer.h"

#include "Win32App.h"

int	ShowBitmap(HWND hWnd, HDC hdcWindow)
{
	Renderer::RenderResult rr = Renderer::RenderResult::Create();

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

		void	OnWndPaint(HDC hdc) override
		{
			ShowBitmap(GetHWND(), hdc);
		}

	private:
		bool	m_draw = true;
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

	win32::RendererWindow wnd(L"My Renderer", hInstance);
	
	return win32::Application::Run(wnd);
}
