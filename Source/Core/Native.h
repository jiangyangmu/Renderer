#pragma once

#include <wchar.h> // wchar_t

struct NativeWindowCallbacks
{
	void	( *move )	( int x, int y );
	void	( *resize )	( int width, int height );
	void	( *close )	( );
};

struct NativeKeyboardCallbacks
{
	void	( *down )	( int keycode );
	void	( *up )		( int keycode );
};

struct NativeMouseCallbacks
{
	void	( *move )	( int x, int y );
	void	( *leftdown )	( int x, int y );
	void	( *leftup )	( int x, int y );
	void	( *rightdown )	( int x, int y );
	void	( *rightup )	( int x, int y );
	void	( *middledown )	( int x, int y );
	void	( *middleup )	( int x, int y );
};

struct NativeWindow;

// Native
bool		NativeInitialize();
void		NativeTerminate();

// Window
NativeWindow *	NativeCreateWindow(const wchar_t * pWindowTitle, int nWidth, int nHeight);
void		NativeDestroyWindow(NativeWindow * pWindow);
int		NativeGetWindowCount();

// Input
void		NativeRegisterWindowCallbacks(NativeWindow * pWindow, const NativeWindowCallbacks * pCallbacks);
void		NativeRegisterKeyboardCallbacks(NativeWindow * pWindow, const NativeKeyboardCallbacks * pCallbacks);
void		NativeRegisterMouseCallbacks(NativeWindow * pWindow, const NativeMouseCallbacks * pCallbacks);

void		NativeInputPoll();

// Debug
const NativeWindowCallbacks *		NativeGetDebugWindowCallbacks();
const NativeKeyboardCallbacks *		NativeGetDebugKeyboardCallbacks();
const NativeMouseCallbacks *		NativeGetDebugMouseCallbacks();