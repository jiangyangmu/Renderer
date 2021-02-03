#pragma once

#include "Buffer.h"
#include "Unknown.h"

namespace Graphics
{
	// ---------------------------------------------------------------
	// Handle
	// ---------------------------------------------------------------

	struct Handle { void * pImpl; void * pParam; };

	// ---------------------------------------------------------------
	// Buffers
	// ---------------------------------------------------------------

	enum class VertexFieldType
	{
		UNKNOWN,
		POSITION,	// Vector3, occur 1 time
		COLOR,		// Vector3, occur 0+ times
		TEXCOORD,	// Vector2, occur 0+ times
		NORMAL,		// Vector3, occur 0+ times
		MATERIAL,	// Vector3, occur 0+ times
		SV_POSITION,
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
		void		ResetBackBuffer(Byte value = 0);
		void		ResetBackBuffer(const Rect & rect, Byte value);
	private:
		void *		FrameBuffer();
	};

	struct DepthStencilBuffer : public Handle
	{
	public:
		void		ResetDepthBuffer(float value = 1.0f);
		void		ResetStencilBuffer(Byte value);
	};

	// ---------------------------------------------------------------
	// State
	// ---------------------------------------------------------------

	enum class DepthWriteMask
	{
		ALL,
		ZERO,
	};

	struct DepthStencilState
	{
		bool		depthEnable;
		bool		stencilEnable;
		DepthWriteMask	depthWriteMask;
		Byte		stencilWriteMask;
	};

	enum class BlendOp
	{
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
		MIN,
		MAX,
	};

	enum class BlendFactor
	{
		ZERO,
		ONE,
		SRC_COLOR,
		DEST_COLOR,
		SRC_ALPHA,
		DEST_ALPHA,
		INV_SRC_COLOR,
		INV_DEST_COLOR,
		// INV_SRC_ALPHA,
		// INV_DEST_ALPHA,
		// SRC_ALPHA_SAT,
	};

	struct BlendState
	{
		bool		blendEnable;
		BlendFactor	srcFactor;
		BlendFactor	dstFactor;
		BlendOp		op;
		BlendFactor	srcFactorAlpha;
		BlendFactor	dstFactorAlpha;
		BlendOp		opAlpha;
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

		bool			Contains(const Rect & other)
		{
			return	left <= other.left &&
				right >= other.right &&
				top <= other.top &&
				bottom >= other.bottom;
		}
	};

	struct RenderTarget : public Handle, public IUnknown
	{
	public:
		Rect			GetRect() const;
		Integer			GetWidth() const;
		Integer			GetHeight() const;
		virtual bool		QueryInterface(Integer iid, void ** ppvObject) override;
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
		void			SetDepthStencilBuffer(DepthStencilBuffer dsb);
		void			SetRenderTarget(RenderTarget target);

		void			RSSetFlipHorizontal(bool bFlipHorizontal);
		void			OMSetDepthStencilState(DepthStencilState st);
		void			OMSetBlendState(BlendState bs);

		DepthStencilBuffer	GetDepthStencilBuffer();
		RenderTarget		GetRenderTarget();

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
		SwapChain		CreateSwapChain(RenderTarget renderTarget);
		DepthStencilBuffer	CreateDepthStencilBuffer(Integer width, Integer height);
		RenderTarget		CreateRenderTarget(IUnknown * pUnknown, const Rect & rect);
		RenderTarget		CreateRenderTarget(Texture2D texture, const Rect & rect);
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