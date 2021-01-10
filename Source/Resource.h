#pragma once

//#include "Common.h"
#include "Graphics.h"
#include <Windows.h>

namespace Graphics
{
namespace v2
{
	// ---------------------------------------------------------------
	// Handle
	// ---------------------------------------------------------------

	struct Handle { void * pImpl; void * pParam; };

	// ---------------------------------------------------------------
	// IUnknown
	// ---------------------------------------------------------------

	// IID: Interface IDentifier
#define _INTERFACE_DEFINE_IID(iid) \
        public: static Integer		__IINTERFACE_IID() { return (iid); }
#define _INTERFACE_IID(interface_) \
	(interface_::__IINTERFACE_IID())
#define _INTERFACE_TEST_IID(interface_, iid) \
	((interface_::__IINTERFACE_IID()) == (iid))

	class IUnknown
	{
	public:
		virtual			~IUnknown() = default;

		virtual bool		QueryInterface(Integer iid, void ** ppvObject)
		{
			return false;
		}
		template <typename T>
		bool			QueryInterface(T ** ppObject)
		{
			return QueryInterface(_INTERFACE_IID(T), (void **)ppObject);
		}
	};

	// ---------------------------------------------------------------
	// Buffers
	// ---------------------------------------------------------------

	enum class VertexFormat
	{
		POS_RGB,
	};
	
	class VertexBuffer : public Handle
	{
	public:
		void *		Data();
	};

	struct IndexBuffer : public Handle
	{
	};

	struct Texture2D : public Handle
	{
	};

	struct SwapChain : public Handle
	{
	public:
		void		Swap();
		void *		FrameBuffer();
	};

	struct DepthStencilBuffer : public Handle
	{
	public:
		void		Reset();
	};

	// ---------------------------------------------------------------
	// Functions
	// ---------------------------------------------------------------

	class VertexShader : public Handle
	{
	};
	class PixelShader : public Handle
	{
	};

	// ---------------------------------------------------------------
	// RenderContext, RenderTaret
	// ---------------------------------------------------------------

	class Window : public Handle
	{
	public:
		static Window		Default();

		HWND			GetHWND();
	};
	struct RenderTarget : public Handle
	{
	public:
		Integer			GetWidth();
		Integer			GetHeight();
	};


	class RenderContext : public Handle
	{
	public:
		void			SetSwapChain(SwapChain sc);

		// Data used in VS, PS
		void			SetViewTransform(Matrix4x4 m);
		void			SetProjectionTransform(Matrix4x4 m);
		void			SetVertexShader(VertexShader vs);
		void			SetPixelShader(PixelShader ps);
		// textures
		// states

		void			SetDepthStencilBuffer(DepthStencilBuffer dsb);
		void			SetOutputTarget(RenderTarget target);

		void			Draw(VertexBuffer vb, Integer nOffset, Integer nCount);
		// void			DrawIndexed(VertexBuffer vb, IndexBuffer ib, Integer nOffset, Integer nCount);
	};

	// ---------------------------------------------------------------
	// Device
	// ---------------------------------------------------------------

	class Device
	{
	public:
		static Device		Default();

		RenderContext		CreateRenderContext();
		SwapChain		CreateSwapChain(Integer width, Integer height, bool enableAutoResize = true);
		DepthStencilBuffer	CreateDepthStencilBuffer(Integer width, Integer height, bool enableAutoResize = true);
		RenderTarget		CreateRenderTarget(HWND hWnd);
		VertexShader		CreateVertexShader();
		PixelShader		CreatePixelShader();
		
		VertexBuffer		CreateVertexBuffer(VertexFormat format);

	private:
		void *			pImpl;
	};
}
}