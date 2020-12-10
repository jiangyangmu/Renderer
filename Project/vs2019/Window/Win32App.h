#pragma once

#include <functional>

#include <Windows.h>

namespace win32
{
	// Window
	HWND                CreateDesktopWindow(HINSTANCE hInstance, LPCWSTR lpWndTitle, int width, int height, WNDPROC lpfnWndProc, LPVOID lpParam);

	class Window
	{
	public:
		Window(LPCWSTR lpTitle, HINSTANCE hInstance);

		virtual         ~Window() = default;

		// Operations
		void            Show();

		// Properties
		HWND            GetHWND() const
		{
			return m_hWnd;
		}
		int             GetWidth() const
		{
			return m_width;
		}
		int             GetHeight() const
		{
			return m_height;
		}
		float           GetAspectRatio() const
		{
			return static_cast< float >( m_width ) / static_cast< float >( m_height );
		}

		// Events
		virtual void    OnWndIdle()
		{
		}
		virtual void    OnWndPaint(HDC hdc)
		{
		}
		virtual void    OnWndMove(int x, int y, int width, int height)
		{
		}
		virtual void    OnWndResize(int x, int y, int width, int height)
		{
		}
		virtual void    OnMouseMove(int pixelX, int pixelY, DWORD flags)
		{
		}
		virtual void    OnKeyDown(WPARAM virtualKeyCode)
		{
		}
		virtual void    OnKeyUp(WPARAM virtualKeyCode)
		{
		}

	private:
		// Win32 interfaces
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT         ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

		HINSTANCE       m_hInstance;
		HWND            m_hWnd;
		int             m_width;
		int             m_height;
	};

	class Application
	{
	public:
		static int      Run(Window & mainWnd);
	};
}
