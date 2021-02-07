#pragma once

#include "Native.h"
#include "Resource.h"

namespace Graphics
{
	class IRenderer;

	class RenderWindow : public IUnknown
	{
		_INTERFACE_DEFINE_IID(1610280267);
	public:
		explicit		RenderWindow(NativeWindow * pWindow_);

		virtual bool		QueryInterface(Integer guid, void **  ppInterface) override;
		void			RegisterEventListener(OnMouseMoveEventHandler * pOnMouseMove, OnKeyDownEventHandler * pOnKeyDown, OnKeyUpEventHandler * pOnKeyUp);

		NativeWindow *		GetWindow()
		{
			return pWindow;
		}
		Integer			GetWidth()
		{
			return NativeWindowGetWidth(pWindow);
		}
		Integer			GetHeight()
		{
			return NativeWindowGetHeight(pWindow);
		}

	private:
		static void		OnMouseMove(int x, int y);
		static void		OnKeyDown(int keycode);
		static void		OnKeyUp(int keycode);

		NativeWindow *			pWindow;
		NativeMouseCallbacks		cbMouse;
		NativeKeyboardCallbacks		cbKeyboard;

		OnMouseMoveEventHandler *	pOnMouseMove;
		OnKeyDownEventHandler *		pOnKeyDown;
		OnKeyUpEventHandler *		pOnKeyUp;
	};
}