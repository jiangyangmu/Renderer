#pragma once

#include "Buffer.h"

#include <Windows.h>

namespace Graphics
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

	enum class VertexFieldType
	{
		UNKNOWN,
		POSITION,	// Vec3, occur 1 time
		COLOR,		// Vec3, occur 0+ times
		TEXCOORD,	// Vec2, occur 0+ times
		NORMAL,		// Vec3, occur 0+ times
		MATERIAL,	// Vec3, occur 0+ times
	};

	struct VertexField
	{
		Integer		offset;
		VertexFieldType	type;
	};

	struct VertexFormat : Handle
	{
		Integer		Alignment();
		Integer		Size();

		VertexField &	operator [] (Integer i);
	};
	
	struct VertexRange
	{
		Integer		nVertexOffset;
		Integer		nVertexCount;
		VertexFormat	hVertexFormat;
		void *		pVertexBegin;

		VertexRange()
			: nVertexOffset(0)
			, nVertexCount(0)
			, hVertexFormat()
			, pVertexBegin(nullptr)
		{}
		void *		At(Integer i);
		Integer		Offset();
		Integer		Count();
	};

	class VertexBuffer : public Handle
	{
	public:
		VertexRange	Alloc(Integer nCount);
		void		Free(VertexRange v);
		
		VertexFormat	GetVertexFormat();
		Integer		Count();
		void *		Data();
	};

	struct IndexBuffer : public Handle
	{
	};

	struct Texture2D : public Handle
	{
	public:
		void		Sample(float u, float v, float * pColor) const;
	};

	struct Rect;

	struct SwapChain : public Handle
	{
	public:
		void		Swap();
		void		ResetBackBuffer();
		void		ResetBackBuffer(const Rect & rect, Byte value);
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

	typedef void (*VertexShaderFunc)(void * pVSOut, const void * pVSIn, const void * pContext);
	typedef void (*PixelShaderFunc)(void * pPSOut, const void * pPSIn, const void * pContext);

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

	struct Rect
	{
		Integer left;
		Integer right;
		Integer top;
		Integer bottom;
	};

	struct RenderTarget : public Handle, public IUnknown
	{
	public:
		RenderTarget() : pDescCache(nullptr), pObjectCache(nullptr) {}

		Rect			GetRect();
		Integer			GetWidth();
		Integer			GetHeight();
		virtual bool		QueryInterface(Integer iid, void ** ppvObject) override;
	private:
		void			InitCache();

		void *			pDescCache;
		void *			pObjectCache;
	};


	class RenderContext : public Handle
	{
	public:
		void			SetSwapChain(SwapChain sc);

		void			SetVertexShader(VertexShader vs);
		void			SetPixelShader(PixelShader ps);
		void			VSSetConstantBuffer(const void * pBuffer);
		void			PSSetConstantBuffer(const void * pBuffer);
		// textures
		// states
		void			SetDepthStencilBuffer(DepthStencilBuffer dsb);
		void			SetOutputTarget(RenderTarget target);

		RenderTarget		GetOutputTarget();

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
		RenderTarget		CreateRenderTarget(IUnknown * pUnknown, const Rect & rect);
		RenderTarget		CreateRenderTarget(RenderTarget renderTarget, const Rect & rectSub);

		VertexFormat		CreateVertexFormat(VertexFieldType type0);
		VertexFormat		CreateVertexFormat(VertexFieldType type0, VertexFieldType type1);
		VertexFormat		CreateVertexFormat(VertexFieldType type0, VertexFieldType type1, VertexFieldType type2);
		VertexFormat		CreateVertexFormat(VertexFieldType type0, VertexFieldType type1, VertexFieldType type2, VertexFieldType type3);
		VertexFormat		CreateVertexFormat(VertexFieldType type0, VertexFieldType type1, VertexFieldType type2, VertexFieldType type3, VertexFieldType type4);
		VertexBuffer		CreateVertexBuffer(VertexFormat format);
		VertexShader		CreateVertexShader(VertexShaderFunc vs, VertexFormat fmtVSIn, VertexFormat fmtVSOut);
		PixelShader		CreatePixelShader(PixelShaderFunc ps, VertexFormat fmtPSIn, VertexFormat fmtPSOut);

		Texture2D		CreateTexture2D(Integer width, Integer height, Integer elementSize, Integer alignment, Integer rowPadding, const void * pData);

	private:
		void *			pImpl;
	};
}