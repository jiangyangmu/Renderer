#pragma once

#include "Event.h"

#include <memory>

namespace win32
{
	// Window
	HWND		CreateDesktopWindow(HINSTANCE hInstance, LPCWSTR lpWndTitle, int width, int height, WNDPROC lpfnWndProc, LPVOID lpParam);

	// GDI+
	ULONG_PTR	InitializeGdiplus();
	void		UninitializeGdiplus(ULONG_PTR token);
	void		LoadBMP(LPCWSTR lpFilePath, LONG * lpWidth, LONG * lpHeight, LPVOID * lpPixels);

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
	public: _SEND_EVENT(OnWndIdle);
	public: _SEND_EVENT(OnWndMove);
	public: _SEND_EVENT(OnWndPaint);
	public: _SEND_EVENT(OnWndResize);
	public: _SEND_EVENT(OnMouseMove);
	public: _SEND_EVENT(OnMouseLButtonDown);
	public: _SEND_EVENT(OnMouseLButtonUp);
	public: _SEND_EVENT(OnKeyDown);
	public: _SEND_EVENT(OnKeyUp);

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
		Application();
		~Application();

		int		Run(Window & mainWnd);

	private:
		ULONG_PTR	m_gdiplusToken;
	};
}
