#include "Win32App.h"

#include <strsafe.h>
#include <cassert>

#include <WindowsX.h>

#define ENSURE_NOT_NULL(e) assert((e) != NULL)
#define ENSURE_TRUE(e) assert((e))

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

	static inline HINSTANCE GetCurrentInstance()
	{
		return GetModuleHandle(NULL);
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

		RECT rect;

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
					HDC hdc = BeginPaint(m_hWnd, &ps);

					OnWndPaint(hdc);
					
					EndPaint(m_hWnd, &ps);
				}
				return 0;

				// case WM_MOVING:
			case WM_MOVE:
				// Sent after a window has been moved.
				{
					OnWndMove(LOWORD(lParam), HIWORD(lParam), 0, 0);
				}
				return 0;

				// case WM_SIZING:
			case WM_SIZE:
				// Sent to a window after its size has changed.
				{
					OnWndResize(0, 0, LOWORD(lParam), HIWORD(lParam));
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
					OnKeyDown(wParam);
				}
				return 0;

			case WM_KEYUP:
				{
					OnKeyUp(wParam);
				}
				return 0;

			case WM_CHAR:
				// wParam
				break;

				// Mouse events

			case WM_LBUTTONDOWN:
				{
				}
				return 0;

			case WM_LBUTTONUP:
				{
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
					OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
				}
				return 0;

			default:
				break;
		}

		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
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
				mainWnd.OnWndIdle();
			}
		}

		return ( int ) msg.wParam;
	}

}
