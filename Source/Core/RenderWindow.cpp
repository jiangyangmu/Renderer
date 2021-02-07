#include "RenderWindow.h"

static Graphics::RenderWindow * gpWindow = nullptr;

namespace Graphics
{

	RenderWindow::RenderWindow(NativeWindow * pWindow_)
		: pWindow(pWindow_)
		, pOnMouseMove(nullptr)
		, pOnKeyDown(nullptr)
		, pOnKeyUp(nullptr)
	{
		memset(&cbMouse, 0, sizeof(cbMouse));
		memset(&cbKeyboard, 0, sizeof(cbKeyboard));

		cbMouse.move = &RenderWindow::OnMouseMove;
		cbKeyboard.down = &RenderWindow::OnKeyDown;
		cbKeyboard.up = &RenderWindow::OnKeyUp;
	}


	bool		RenderWindow::QueryInterface(Integer guid, void ** ppInterface)
	{
		if ( _INTERFACE_IID(RenderWindow) == guid )
		{
			*ppInterface = this;
			return true;
		}
		else
		{
			return false;
		}
	}

	void		RenderWindow::RegisterEventListener(OnMouseMoveEventHandler * pOnMouseMove, OnKeyDownEventHandler * pOnKeyDown, OnKeyUpEventHandler * pOnKeyUp)
	{
		this->pOnMouseMove = pOnMouseMove;
		this->pOnKeyDown = pOnKeyDown;
		this->pOnKeyUp = pOnKeyUp;

		NativeRegisterMouseCallbacks(pWindow, &cbMouse);
		NativeRegisterKeyboardCallbacks(pWindow, &cbKeyboard);

		gpWindow = this;
	}


	void		RenderWindow::OnMouseMove(int x, int y)
	{
		win32::MouseEventArgs args;
		args.pixelX = x;
		args.pixelY = y;
		if ( gpWindow && gpWindow->pOnMouseMove )
		{
			(*gpWindow->pOnMouseMove)(gpWindow, args);
		}
	}
	void		RenderWindow::OnKeyDown(int keycode)
	{
		win32::KeyboardEventArgs args;
		args.virtualKeyCode = keycode;
		if ( gpWindow && gpWindow->pOnKeyDown )
		{
			( *gpWindow->pOnKeyDown )( gpWindow, args );
		}
	}
	void		RenderWindow::OnKeyUp(int keycode)
	{
		win32::KeyboardEventArgs args;
		args.virtualKeyCode = keycode;
		if ( gpWindow && gpWindow->pOnKeyUp )
		{
			( *gpWindow->pOnKeyUp )( gpWindow, args );
		}
	}
}