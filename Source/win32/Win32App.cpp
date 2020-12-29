#include "Win32App.h"
#include "Common.h"

#include <WindowsX.h>
#include <Gdiplus.h>

#include <strsafe.h>

namespace win32
{

	// --------------------------------------------------------------------------
	// Window
	// --------------------------------------------------------------------------

	HWND CreateDesktopWindow(
		HINSTANCE hInstance,
		LPCWSTR lpWndTitle,
		int width,
		int height,
		WNDPROC lpfnWndProc,
		LPVOID lpParam)
	{
		static LPCWSTR      wndClassName = L"Win32 Window Class";
		static UINT         wndStyle = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		static const int    x = CW_USEDEFAULT;
		static const int    y = CW_USEDEFAULT;

		// Register window class

		WNDCLASS wndClass;
		wndClass.style = wndStyle;
		wndClass.lpfnWndProc = lpfnWndProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInstance;
		wndClass.hIcon = LoadIconW(hInstance, L"IDI_ICON");
		wndClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wndClass.hbrBackground = NULL;
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = wndClassName;

		if ( !RegisterClassW(&wndClass) )
		{
			return NULL;
		}

		// Create window

		return CreateWindow(
			wndClassName,
			lpWndTitle,
			WS_OVERLAPPEDWINDOW,
			x,
			y,
			width,
			height,
			NULL, // hWndParent
			NULL, // hMenu
			hInstance,
			lpParam
		);
	}

	// --------------------------------------------------------------------------
	// GDI+
	// --------------------------------------------------------------------------

	ULONG_PTR InitializeGdiplus()
	{
		Gdiplus::GdiplusStartupInput input;
		ULONG_PTR token = NULL;
		ENSURE_GDIPLUS_OK(
			Gdiplus::GdiplusStartup(&token, &input, NULL));
		return token;
	}

	void UninitializeGdiplus(ULONG_PTR token)
	{
		Gdiplus::GdiplusShutdown(token);
	}

	void LoadBMP(LPCWSTR lpFilePath, LONG * lpWidth, LONG * lpHeight, LPVOID * lpPixels)
	{
		Gdiplus::Bitmap * gdiplusBitmap = Gdiplus::Bitmap::FromFile(lpFilePath, false);
		ENSURE_NOT_NULL(gdiplusBitmap);

		const UINT w = gdiplusBitmap->GetWidth();
		const UINT h = gdiplusBitmap->GetHeight();

		DWORD * pixels = new DWORD[ w * h ];
		DWORD * pixel = pixels;
		ENSURE_NOT_NULL(pixels);

		Gdiplus::Color color;
		for ( int y = 0; y < h; ++y )
		{
			for ( int x = 0; x < w; ++x )
			{
				gdiplusBitmap->GetPixel(x, h - y - 1, &color);
				*( pixel++ ) = color.GetValue();
			}
		}

		delete gdiplusBitmap;

		*lpWidth = static_cast<LONG>(w);
		*lpHeight = static_cast<LONG>(h);
		*lpPixels = pixels;
	}

	static inline HINSTANCE GetCurrentInstance()
	{
		return GetModuleHandle(NULL);
	}

	// --------------------------------------------------------------------------
	// Threads
	// --------------------------------------------------------------------------

	ParallelTaskRunner::ParallelTaskRunner(DWORD nThread)
	{
		InitializeThreadpoolEnvironment(&m_callBackEnviron);

		m_pool = CreateThreadpool(NULL);
		ENSURE_NOT_NULL(m_pool);

		SetThreadpoolThreadMaximum(m_pool, nThread);
		ENSURE_TRUE(SetThreadpoolThreadMinimum(m_pool, nThread));

		m_cleanupGroup = CreateThreadpoolCleanupGroup();
		ENSURE_NOT_NULL(m_cleanupGroup);

		SetThreadpoolCallbackPool(&m_callBackEnviron, m_pool);
		SetThreadpoolCallbackCleanupGroup(&m_callBackEnviron,
						  m_cleanupGroup,
						  NULL);
	}

	ParallelTaskRunner::~ParallelTaskRunner()
	{
		if (!m_tasks.empty())
		{
			WaitForAllTasks();
		}

		CloseThreadpoolCleanupGroupMembers(m_cleanupGroup,
						   FALSE,
						   NULL);
		CloseThreadpoolCleanupGroup(m_cleanupGroup);
		CloseThreadpool(m_pool);
	}

	void ParallelTaskRunner::RunTask(PTP_WORK_CALLBACK callback, LPVOID lpData)
	{
		PTP_WORK work = CreateThreadpoolWork(callback,
						     lpData,
						     &m_callBackEnviron);
		ENSURE_NOT_NULL(work);
		SubmitThreadpoolWork(work);

		m_tasks.emplace_back(work);
	}

	void ParallelTaskRunner::WaitForAllTasks()
	{
		for (PTP_WORK & pWork : m_tasks)
		{
			WaitForThreadpoolWorkCallbacks(pWork, FALSE);
		}

		m_tasks.clear();
	}


	// --------------------------------------------------------------------------
	// Application
	// --------------------------------------------------------------------------

	Window::Window(LPCWSTR lpTitle, HINSTANCE hInstance)
	{
		m_hInstance = hInstance;
		m_hWnd = CreateDesktopWindow(m_hInstance,
					     lpTitle,
					     800,
					     600,
					     Window::WindowProc,
					     this);
		ENSURE_NOT_NULL(m_hWnd);
		// HACK: CreateWindow can't set window title
		SetWindowText(m_hWnd, lpTitle);

		RECT rect = {};

		ENSURE_TRUE(GetClientRect(m_hWnd, &rect));

		m_width = rect.right - rect.left;
		m_height = rect.bottom - rect.top;
	}

	void Window::Show()
	{
		if ( m_hWnd )
		{
			ShowWindow(m_hWnd, SW_SHOW);
		}
	}

	LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Window * pThis = nullptr;

		if ( WM_NCCREATE == uMsg )
		{
			CREATESTRUCT * pCreate;

			pCreate = ( CREATESTRUCT * ) lParam;

			// Bind pThis and hWnd
			pThis = ( Window * ) pCreate->lpCreateParams;
			pThis->m_hWnd = hWnd;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, ( LONG_PTR ) pThis);

			return TRUE;
		}
		else if ( WM_NCDESTROY == uMsg )
		{
			// Unbind pThis and hWnd
			pThis = ( Window * ) GetWindowLongPtr(hWnd, GWLP_USERDATA);
			pThis->m_hWnd = NULL;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);

			return 0;
		}
		else
		{
			// Get pThis associated with hWnd
			pThis = ( Window * ) GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}

		if ( pThis )
		{
			if ( WM_DESTROY == uMsg )
			{
				PostQuitMessage(0);
			}
			else if ( WM_CLOSE == uMsg )
			{
				DestroyWindow(hWnd);
			}
			return pThis->ProcessMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	LRESULT Window::ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Map win32 events to Window events

		WindowRect rect;
		WindowPaintArgs wndPaintArgs;
		MouseEventArgs mouseEventArgs;
		KeyboardEventArgs keyboardEventArgs;
		switch ( uMsg )
		{
			// Window events

			case WM_CREATE:
				// Sent when an application requests that a window be created,
				// after the window is created, but before the window becomes visible.
				return 0;

			case WM_DESTROY:
				// Sent when a window is being destroyed,
				// after the window is removed from the screen, before all child windows are destroyed.
				return 0;

			case WM_PAINT:
				{
					PAINTSTRUCT ps;

					wndPaintArgs.hdc = BeginPaint(m_hWnd, &ps);

					_DISPATCH_EVENT1(OnWndPaint, *this, wndPaintArgs);
					
					EndPaint(m_hWnd, &ps);
				}
				return 0;

				// case WM_MOVING:
			case WM_MOVE:
				// Sent after a window has been moved.
				{
					rect.x = LOWORD(lParam);
					rect.y = HIWORD(lParam);

					_DISPATCH_EVENT1(OnWndMove, *this, rect);
				}
				return 0;

				// case WM_SIZING:
			case WM_SIZE:
				// Sent to a window after its size has changed.
				{
					m_width = LOWORD(lParam);
					m_height = HIWORD(lParam);
					rect.width = m_width;
					rect.height = m_height;

					_DISPATCH_EVENT1(OnWndResize, *this, rect);
				}
				return 0;

			case WM_CLOSE:
				// Sent as a signal that a window or an application should terminate.
				return 0;

				// Keyboard events

			case WM_SYSKEYDOWN:
				// wParam
				break;

			case WM_SYSCHAR:
				// wParam
				break;

			case WM_SYSKEYUP:
				// wParam
				break;

			case WM_KEYDOWN:
				{
					keyboardEventArgs.virtualKeyCode = wParam;
					_DISPATCH_EVENT1(OnKeyDown, *this, keyboardEventArgs);
				}
				return 0;

			case WM_KEYUP:
				{
					keyboardEventArgs.virtualKeyCode = wParam;
					_DISPATCH_EVENT1(OnKeyUp, *this, keyboardEventArgs);
				}
				return 0;

			case WM_CHAR:
				// wParam
				break;

				// Mouse events

			case WM_LBUTTONDOWN:
				{
					mouseEventArgs.pixelX = GET_X_LPARAM(lParam);
					mouseEventArgs.pixelY = GET_Y_LPARAM(lParam);
					mouseEventArgs.flags = ( DWORD ) wParam;
					_DISPATCH_EVENT1(OnMouseLButtonDown, *this, mouseEventArgs);
				}
				return 0;

			case WM_LBUTTONUP:
				{
					mouseEventArgs.pixelX = GET_X_LPARAM(lParam);
					mouseEventArgs.pixelY = GET_Y_LPARAM(lParam);
					mouseEventArgs.flags = ( DWORD ) wParam;
					_DISPATCH_EVENT1(OnMouseLButtonUp, *this, mouseEventArgs);
				}
				return 0;

			case WM_LBUTTONDBLCLK:
				break;

			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
				break;

			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:
				break;

			case WM_MOUSEMOVE:
				{
					mouseEventArgs.pixelX = GET_X_LPARAM(lParam);
					mouseEventArgs.pixelY = GET_Y_LPARAM(lParam);
					mouseEventArgs.flags = ( DWORD ) wParam;
					_DISPATCH_EVENT1(OnMouseMove, *this, mouseEventArgs);
				}
				return 0;

			default:
				break;
		}

		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}

	Application::Application()
	{
		m_gdiplusToken = InitializeGdiplus();
	}

	Application::~Application()
	{
		UninitializeGdiplus(m_gdiplusToken);
	}

	int Application::Run(Window & mainWnd)
	{
		mainWnd.Show();

		MSG msg;

		msg.message = WM_NULL;
		PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
		while ( WM_QUIT != msg.message )
		{
			if ( PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				_DISPATCH_EVENT(OnWndIdle, mainWnd);
			}
		}

		return ( int ) msg.wParam;
	}
}
