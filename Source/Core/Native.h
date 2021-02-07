#pragma once

#include <cwchar> // wchar_t, size_t
#include <cstdint> // int64_t

enum NativeBlitMode
{
	NATIVE_BLIT_UNKNOWN	= 0,
	NATIVE_BLIT_BGRA	= 1,
	NATIVE_BLIT_BGR		= 2,
	NATIVE_BLIT_F32		= 3,
	NATIVE_BLIT_U8		= 4,
};

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
NativeWindow *	NativeCreateWindow(const wchar_t * pWindowTitle, int nWidth, int nHeight, int nLeft, int nTop);
void		NativeDestroyWindow(NativeWindow * pWindow);
int		NativeGetWindowCount();
int		NativeWindowGetWidth(NativeWindow * pWindow);
int		NativeWindowGetHeight(NativeWindow * pWindow);
bool		NativeWindowBilt(NativeWindow * pWindow, const void * pSrc, NativeBlitMode mode);

// Input
void		NativeRegisterWindowCallbacks(NativeWindow * pWindow, const NativeWindowCallbacks * pCallbacks);
void		NativeRegisterKeyboardCallbacks(NativeWindow * pWindow, const NativeKeyboardCallbacks * pCallbacks);
void		NativeRegisterMouseCallbacks(NativeWindow * pWindow, const NativeMouseCallbacks * pCallbacks);

void		NativeInputPoll();

// Time
int64_t		NativeGetTick();

// Memory
void *		AlignedMalloc(size_t nSize, size_t nAlign);
void		AlignedFree(void * p);

// Debug
const NativeWindowCallbacks *		NativeDebugGetWindowCallbacks();
const NativeKeyboardCallbacks *		NativeDebugGetKeyboardCallbacks();
const NativeMouseCallbacks *		NativeDebugGetMouseCallbacks();