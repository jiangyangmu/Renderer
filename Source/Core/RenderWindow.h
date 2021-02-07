#pragma once

#include "../Native/Win32App.h"
#include "Resource.h"

namespace Graphics
{
	class IRenderer;

	class RenderWindow : public win32::Window, public IUnknown
	{
		_INTERFACE_DEFINE_IID(1610280267);
	public:

		RenderWindow(LPCWSTR lpTitle, HINSTANCE hInstance, int nWidth, int nHeight);

		// Operations

		void			Paint(LONG width, LONG height, LPVOID data);
		virtual bool		QueryInterface(Integer guid, void **  ppInterface) override;

		// Properties

		void			SetRenderer(IRenderer & renderer);

		// Events

	private: _RECV_EVENT_DECL(RenderWindow, OnWndIdle);

	private:

		// Rendering
		IRenderer *		m_refRender;

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