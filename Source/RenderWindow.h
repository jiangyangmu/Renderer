#pragma once

#include "win32/Win32App.h"

namespace Graphics
{
	class Renderer;

	class RenderWindow : public win32::Window
	{
	public:
		RenderWindow(LPCWSTR lpTitle, HINSTANCE hInstance, int nWidth, int nHeight);

		// Operations

		void			Paint(LONG width, LONG height, LPVOID data);

		// Properties

		void			SetRenderer(Renderer & renderer);

		// Events

	public: _SEND_EVENT(OnAspectRatioChange);
	private: _RECV_EVENT_DECL(RenderWindow, OnWndIdle);
	private: _RECV_EVENT_DECL1(RenderWindow, OnWndResize);
	private: _RECV_EVENT_DECL1(RenderWindow, OnMouseMove);
	private: _RECV_EVENT_DECL1(RenderWindow, OnMouseLButtonDown);
	private: _RECV_EVENT_DECL1(RenderWindow, OnMouseLButtonUp);

	private:

		// Rendering
		Renderer *		m_refRender;

		// FPS control
		LARGE_INTEGER		m_timerFrequence;
		LARGE_INTEGER		m_timerBegin;
		LARGE_INTEGER		m_timerEnd;
		double			m_minFrameCostMS; // each frame spend at least this milliseconds
		double			m_lastFrameCostMS;

		// Statistics
		LONG			m_drawnFrame;
		LONG			m_presentedFrame;

		bool			m_debugMode;
	};
}