#include "Resource.h"
#include "_Math.h"

#include "RenderWindow.h"
#include "../Native/Win32App.h"

#include <deque>

#define NUM_MAX_VERTEX_FIELD (5)

namespace Graphics
{
	struct ResourceIndex { Integer value; };
	struct BufferIndex : public ResourceIndex {};
	struct DescIndex : public ResourceIndex {};
	struct ShaderIndex : public ResourceIndex {};

	static const ResourceIndex	NULL_RESOURCE = {};
	static const BufferIndex	NULL_BUFFER = {};
	static const DescIndex		NULL_DESC = {};
	static const ShaderIndex	NULL_SHADER = {};

	class Window_Desc : public win32::Window
	{
	public:
		using Window::Window;
	};

	struct SwapChain_Desc
	{
		DescIndex		iRenderTargetDesc;
		BufferIndex		iBuffers[ 2 ];
		bool			bSwapped;
	};

	struct DepthStencil_Desc
	{
		BufferIndex		iDepthBuffer;
		BufferIndex		iStencilBuffer;
	};

	struct VertexFormat_Desc
	{
		VertexField		vFields[NUM_MAX_VERTEX_FIELD];
		Integer			nFields;
		Integer			nSize;
		Integer			nAlign;
	};

	struct VertexBuffer_Desc
	{
		BufferIndex		iVertexBuffer;
		DescIndex		iVertexFormat;
		Integer			nAllocated;
	};

	struct Texture2D_Desc
	{
		BufferIndex		iTexDataBuffer;
	};

	struct RenderTarget_Desc
	{
		IUnknown *		pUnknown;
		Rect			rect;
	};

	struct VertexShader_Desc
	{
		VertexShaderFunc	pFunc;
		DescIndex		iVSInFormat;
		DescIndex		iVSOutFormat;
	};
	struct PixelShader_Desc
	{
		PixelShaderFunc		pFunc;
		DescIndex		iPSInFormat;
		DescIndex		iPSOutFormat;
	};

	struct Device_Impl;

	struct RenderContext_Impl
	{
		Device_Impl *		pDevice;

		DescIndex		iSwapChainDesc;
		DescIndex		iDepthStencilDesc;
		DescIndex		iRenderTargetDesc;

		ShaderIndex		iVertexShader;
		ShaderIndex		iPixelShader;
		const void *		pVertexShaderData;
		const void *		pPixelShaderData;

		bool			bFlipHorizontal;
		DepthStencilState	stDepthStencil;
		BlendState		stBlend;
	};

	struct Device_Impl
	{
		std::deque<Buffer>			buffers;

		std::vector<SwapChain_Desc>		swapChainDescs;
		std::vector<DepthStencil_Desc>		depthStencilDescs;
		std::vector<VertexFormat_Desc>		vertexFormatDescs;
		std::vector<VertexBuffer_Desc>		vertexBufferDescs;
		//std::vector<IndexBuffer_Desc>		indexBuffers;
		std::vector<Texture2D_Desc>		textureDescs;

		std::vector<VertexShader_Desc>		vertexShaderDescs;
		std::vector<PixelShader_Desc>		pixelShaderDescs;

		std::vector<RenderTarget_Desc>		renderTargetDescs;

		std::vector<Ptr<RenderContext_Impl>>	renderContextImpls;
	};

	struct ShaderContext
	{
		Matrix44 view;
		Matrix44 proj;
	};

	struct VS_DEFAULT_POS_RGB_IN
	{
		Vector3 posWld;
		Vector3 color;
	};
	struct VS_DEFAULT_POS_RGB_OUT
	{
		Vector3 posNDC;
		Vector3 color;
	};
	struct PS_DEFAULT_POS_RGB_OUT
	{
		Vector3 color;
	};

	static inline void			_StoreIndex(Handle * h, const ResourceIndex & i)
	{
		h->pImpl = reinterpret_cast< void * >( i.value );
	}
	static inline void			_LoadIndex(const Handle & h, ResourceIndex * i)
	{
		i->value = reinterpret_cast< Integer >( h.pImpl );
	}

	static inline Integer			_GetVertexFieldAlignment(VertexFieldType type)
	{
		Integer alignment;

		switch ( type )
		{
			case VertexFieldType::POSITION:
			case VertexFieldType::SV_POSITION:
			case VertexFieldType::COLOR:
			case VertexFieldType::TEXCOORD:
			case VertexFieldType::NORMAL:
			case VertexFieldType::MATERIAL:
				alignment = 4;
				break;
			case VertexFieldType::UNKNOWN:
			default:
				ASSERT(false);
				alignment = 0;
				break;
		}

		return alignment;
	}
	static inline Integer			_GetVertexFieldSize(VertexFieldType type)
	{
		Integer size;

		switch ( type )
		{
			case VertexFieldType::TEXCOORD:
				size = 8;
				break;
			case VertexFieldType::POSITION:
			case VertexFieldType::SV_POSITION:
			case VertexFieldType::COLOR:
			case VertexFieldType::NORMAL:
			case VertexFieldType::MATERIAL:
				size = 12;
				break;
			case VertexFieldType::UNKNOWN:
			default:
				ASSERT(false);
				size = 0;
				break;
		}

		return size;
	}

	static inline Device_Impl *		_GetDevice(Handle & h)
	{
		return static_cast< Device_Impl * >( h.pParam );
	}
	static inline const Device_Impl *	_GetDevice(const Handle & h)
	{
		return static_cast< const Device_Impl * >( h.pParam );
	}

	static inline SwapChain_Desc *		_GetSwapChainDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.swapChainDescs[ iDesc.value ];
	}
	static inline DepthStencil_Desc *	_GetDepthStencilDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.depthStencilDescs[ iDesc.value ];
	}
	static inline VertexFormat_Desc *	_GetVertexFormatDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.vertexFormatDescs[ iDesc.value ];
	}
	static inline VertexBuffer_Desc *	_GetVertexBufferDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.vertexBufferDescs[ iDesc.value ];
	}
	static inline Texture2D_Desc *		_GetTextureDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.textureDescs[ iDesc.value ];
	}
	static inline VertexShader_Desc *	_GetVertexShaderDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.vertexShaderDescs[ iDesc.value ];
	}
	static inline PixelShader_Desc *	_GetPixelShaderDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.pixelShaderDescs[ iDesc.value ];
	}
	static inline RenderTarget_Desc *	_GetRenderTargetDesc(Device_Impl & device, Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.renderTargetDescs[ iDesc.value ];
	}
	static inline const SwapChain_Desc *	_GetSwapChainDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.swapChainDescs[ iDesc.value ];
	}
	static inline const DepthStencil_Desc *	_GetDepthStencilDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.depthStencilDescs[ iDesc.value ];
	}
	static inline const VertexFormat_Desc *	_GetVertexFormatDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.vertexFormatDescs[ iDesc.value ];
	}
	static inline const VertexBuffer_Desc *	_GetVertexBufferDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.vertexBufferDescs[ iDesc.value ];
	}
	static inline const Texture2D_Desc *	_GetTextureDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.textureDescs[ iDesc.value ];
	}
	static inline const VertexShader_Desc *	_GetVertexShaderDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.vertexShaderDescs[ iDesc.value ];
	}
	static inline const PixelShader_Desc *	_GetPixelShaderDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.pixelShaderDescs[ iDesc.value ];
	}
	static inline const RenderTarget_Desc *	_GetRenderTargetDesc(const Device_Impl & device, const Handle & h)
	{
		DescIndex iDesc;
		_LoadIndex(h, &iDesc);
		return &device.renderTargetDescs[ iDesc.value ];
	}

	static inline Buffer &			_GetBuffer(Device_Impl & device, BufferIndex iBuffer)
	{
		return device.buffers[ iBuffer.value ];
	}
	static inline const Buffer &		_GetBuffer(const Device_Impl & device, BufferIndex iBuffer)
	{
		return device.buffers[ iBuffer.value ];
	}

	static inline Buffer &			_GetFrontBuffer(Device_Impl & device, SwapChain_Desc & swapChainDesc)
	{
		return
			device.buffers[
				swapChainDesc.iBuffers[
					swapChainDesc.bSwapped ? 1 : 0
				].value
			];
	}
	static inline Buffer &			_GetFrontBuffer(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		SwapChain_Desc *	pSwapChainDesc;

		pDevice			= context.pDevice;
		pSwapChainDesc		= &pDevice->swapChainDescs[ context.iSwapChainDesc.value ];
		
		return _GetFrontBuffer(*pDevice, *pSwapChainDesc);
	}
	static inline Buffer &			_GetBackBuffer(Device_Impl & device, SwapChain_Desc & swapChainDesc)
	{
		return
			device.buffers[
				swapChainDesc.iBuffers[
					swapChainDesc.bSwapped ? 0 : 1
				].value
			];
	}
	static inline Buffer &			_GetBackBuffer(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		SwapChain_Desc *	pSwapChainDesc;

		pDevice			= context.pDevice;
		pSwapChainDesc		= &pDevice->swapChainDescs[ context.iSwapChainDesc.value ];

		return _GetBackBuffer(*pDevice, *pSwapChainDesc);
	}
	static inline Buffer &			_GetDepthBuffer(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		DepthStencil_Desc *	pDepthStencilDesc;

		pDevice			= context.pDevice;
		pDepthStencilDesc	= &pDevice->depthStencilDescs[ context.iDepthStencilDesc.value ];

		return pDevice->buffers[ pDepthStencilDesc->iDepthBuffer.value ];
	}
	static inline Buffer &			_GetStencilBuffer(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		DepthStencil_Desc *	pDepthStencilDesc;

		pDevice			= context.pDevice;
		pDepthStencilDesc	= &pDevice->depthStencilDescs[ context.iDepthStencilDesc.value ];

		return pDevice->buffers[ pDepthStencilDesc->iStencilBuffer.value ];
	}
	static inline VertexShader_Desc *	_GetVertexShaderDesc(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		VertexShader_Desc *	pVertexShaderDesc;

		pDevice			= context.pDevice;
		pVertexShaderDesc	= &pDevice->vertexShaderDescs[ context.iVertexShader.value ];

		return pVertexShaderDesc;
	}
	static inline PixelShader_Desc *	_GetPixelShaderDesc(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		PixelShader_Desc *	pPixelShaderDesc;

		pDevice			= context.pDevice;
		pPixelShaderDesc	= &pDevice->pixelShaderDescs[ context.iPixelShader.value ];

		return pPixelShaderDesc;
	}
	static inline Buffer &			_GetVertexBuffer(VertexBuffer vb)
	{
		Device_Impl *		pDevice;
		BufferIndex		iVertexBufferDesc;
		VertexBuffer_Desc *	pVertexBufferDesc;

		_LoadIndex(vb, &iVertexBufferDesc);

		pDevice			= static_cast< Device_Impl * >( vb.pParam );
		pVertexBufferDesc	= &pDevice->vertexBufferDescs[ iVertexBufferDesc.value ];

		return pDevice->buffers[ pVertexBufferDesc->iVertexBuffer.value ];
	}
	static inline VertexFormat_Desc *	_GetVertexFormatDesc(VertexFormat hVertexFormat)
	{
		Device_Impl *		pDevice;
		DescIndex		iVertexFormatDesc;
		VertexFormat_Desc *	pVertexFormatDesc;

		_LoadIndex(hVertexFormat, &iVertexFormatDesc);

		pDevice			= static_cast< Device_Impl * >( hVertexFormat.pParam );
		pVertexFormatDesc	= &pDevice->vertexFormatDescs[ iVertexFormatDesc.value ];

		return pVertexFormatDesc;
	}
	static inline void			_GetVertexBufferDesc(VertexBuffer vb, VertexBuffer_Desc ** ppVertexBufferDesc, Buffer ** ppBuffer)
	{
		Device_Impl *		pDevice;
		BufferIndex		iVertexBufferDesc;
		VertexBuffer_Desc *	pVertexBufferDesc;

		_LoadIndex(vb, &iVertexBufferDesc);

		pDevice			= static_cast< Device_Impl * >( vb.pParam );
		pVertexBufferDesc	= &pDevice->vertexBufferDescs[ iVertexBufferDesc.value ];

		if (ppVertexBufferDesc)
		{
			*ppVertexBufferDesc	= pVertexBufferDesc;
		}
		if (ppBuffer)
		{
			*ppBuffer		= &pDevice->buffers[ pVertexBufferDesc->iVertexBuffer.value ];
		}
	}
	static inline Rect			_GetOutputTargetRect(RenderContext_Impl & context)
	{
		Device_Impl * pDevice;
		RenderTarget_Desc * pRenderTargetDesc;

		pDevice				= context.pDevice;
		pRenderTargetDesc		= &pDevice->renderTargetDescs[ context.iRenderTargetDesc.value ];

		return pRenderTargetDesc->rect;
	}

	static inline void			_ResetBackBuffer(Buffer & b, Byte value)
	{
		b.SetAll(value);
	}
	static inline void			_ResetDepthBuffer(Buffer & b, float value = 1.0f)
	{
		b.SetAllAs<float>(value);
	}
	static inline void			_ResetStencilBuffer(Buffer & b, Byte value = 0xff)
	{
		b.SetAll(value);
		/*
		Integer xMid = stencilBuffer.Width() / 2;
		Integer yMid = stencilBuffer.Height() / 2;
		for ( Integer y = 0; y < stencilBuffer.Height(); ++y )
		{
			for ( Integer x = 0; x < stencilBuffer.Width(); ++x )
			{
				*static_cast< Byte * >( b.At(y, x) ) = ( ( x - xMid ) * ( x - xMid ) + ( y - yMid ) * ( y - yMid ) ) < 100 * 100 ? 1 : 0;
			}
		}
		*/
	}

	static inline BufferIndex		_CreateBuffer(Device_Impl & device, Integer width, Integer height, Integer elementSize, Integer alignment = 1, Integer rowPadding = 0)
	{
		BufferIndex iBuffer;
		iBuffer.value = device.buffers.size();

		device.buffers.emplace_back(width, height, elementSize, alignment, rowPadding);

		return iBuffer;
	}
	static inline BufferIndex		_CreateBuffer(Device_Impl & device, Integer width, Integer height, Integer elementSize, Integer alignment, Integer rowPadding, const void * pData)
	{
		BufferIndex iBuffer;
		iBuffer.value = device.buffers.size();

		device.buffers.emplace_back(width, height, elementSize, alignment, rowPadding);
		
		if (pData)
		{
			memcpy(device.buffers.back().Data(),
			       pData,
			       device.buffers.back().SizeInBytes());
		}
		else
		{
			memset(device.buffers.back().Data(),
			       0,
			       device.buffers.back().SizeInBytes());
		}

		return iBuffer;
	}
	static inline SwapChain_Desc		_CreateSwapChain(Device_Impl & device, DescIndex iRenderTargetDesc)
	{
		RenderTarget_Desc * pRenderTargetDesc;

		pRenderTargetDesc	= &device.renderTargetDescs[ iRenderTargetDesc.value ];

		Integer nWidth		= pRenderTargetDesc->rect.right - pRenderTargetDesc->rect.left;
		Integer nHeight		= pRenderTargetDesc->rect.bottom - pRenderTargetDesc->rect.top;
		Integer rowPadding	= ( 4 - ( ( nWidth * 3 ) & 0x3 ) ) & 0x3;

		SwapChain_Desc sc;

		sc.iRenderTargetDesc	= iRenderTargetDesc;
		sc.iBuffers[0]		= _CreateBuffer(device, nWidth, nHeight, 3, 4, rowPadding);
		sc.iBuffers[1]		= _CreateBuffer(device, nWidth, nHeight, 3, 4, rowPadding);
		sc.bSwapped		= false;

		return sc;
	}
	static inline DepthStencil_Desc		_CreateDepthStencilBuffer(Device_Impl & device, Integer nWidth, Integer nHeight)
	{
		DepthStencil_Desc dsb;

		dsb.iDepthBuffer	= _CreateBuffer(device, nWidth, nHeight, 4, 1, 0);
		dsb.iStencilBuffer	= _CreateBuffer(device, nWidth, nHeight, 1, 1, 0);

		_ResetStencilBuffer(device.buffers[dsb.iStencilBuffer.value]);

		return dsb;
	}
	static inline VertexBuffer_Desc		_CreateVertexBuffer(Device_Impl & device, DescIndex iVertexFormatDesc)
	{
		VertexFormat_Desc * pVertexFormatDesc;
		VertexBuffer_Desc vb;

		pVertexFormatDesc	= &device.vertexFormatDescs[ iVertexFormatDesc.value ];

		vb.iVertexBuffer	= _CreateBuffer(device, 1024, 1, pVertexFormatDesc->nSize, pVertexFormatDesc->nAlign);
		vb.iVertexFormat	= iVertexFormatDesc;
		vb.nAllocated		= 0;

		return vb;
	}
	static inline Texture2D_Desc		_CreateTexture2D(Device_Impl & device, BufferIndex iBuffer)
	{
		Texture2D_Desc textureDesc;

		textureDesc.iTexDataBuffer = iBuffer;

		return textureDesc;
	}
	static inline RenderTarget_Desc		_CreateRenderTarget(Device_Impl & device, IUnknown * pUnknown, const Rect & rect)
	{
		RenderTarget_Desc target;

		target.pUnknown			= pUnknown;
		target.rect			= rect;

		RenderWindow * pWindow;
		Buffer * pBuffer;
		Rect rectOrigin;
		if ( pUnknown->QueryInterface(&pWindow) )
		{
			rectOrigin.left		= 0;
			rectOrigin.right	= pWindow->GetWidth();
			rectOrigin.top		= 0;
			rectOrigin.bottom	= pWindow->GetHeight();
		}
		else
		{
			ENSURE_TRUE(pUnknown->QueryInterface(&pBuffer));

			rectOrigin.left		= 0;
			rectOrigin.right	= pBuffer->Width();
			rectOrigin.top		= 0;
			rectOrigin.bottom	= pBuffer->Height();
		}
		ENSURE_TRUE(rectOrigin.Contains(rect));

		return target;
	}
	static inline Ptr<RenderContext_Impl>	_CreateRenderContext(Device_Impl & device)
	{
		RenderContext_Impl * context = new RenderContext_Impl;

		context->pDevice		= &device;
		context->iSwapChainDesc		= NULL_DESC;
		context->iDepthStencilDesc	= NULL_DESC;
		context->iRenderTargetDesc	= NULL_DESC;
		context->iVertexShader		= NULL_SHADER;
		context->iPixelShader		= NULL_SHADER;
		context->pVertexShaderData	= nullptr;
		context->pPixelShaderData	= nullptr;

		context->bFlipHorizontal	= false;
		
		context->stDepthStencil.depthEnable		= true;
		context->stDepthStencil.stencilEnable		= true;
		context->stDepthStencil.depthWriteMask		= DepthWriteMask::ALL;
		context->stDepthStencil.stencilWriteMask	= 0;
		
		context->stBlend.blendEnable	= false;
		context->stBlend.srcFactor	= BlendFactor::ONE;
		context->stBlend.dstFactor	= BlendFactor::ZERO;
		context->stBlend.op		= BlendOp::ADD;
		context->stBlend.srcFactorAlpha	= BlendFactor::ONE;
		context->stBlend.dstFactorAlpha	= BlendFactor::ZERO;
		context->stBlend.opAlpha	= BlendOp::ADD;

		return Ptr<RenderContext_Impl>(context);
	}

	static inline VertexShader_Desc		_CreateVertexShader(Device_Impl & device, VertexShaderFunc vs, VertexFormat fmtVSIn, VertexFormat fmtVSOut)
	{
		VertexShader_Desc vertexShaderDesc;
		vertexShaderDesc.pFunc = vs;
		_LoadIndex(fmtVSIn, &vertexShaderDesc.iVSInFormat);
		_LoadIndex(fmtVSOut, &vertexShaderDesc.iVSOutFormat);
		return vertexShaderDesc;
	}
	static inline PixelShader_Desc		_CreatePixelShader(Device_Impl & device, PixelShaderFunc ps, VertexFormat fmtPSIn, VertexFormat fmtPSOut)
	{
		PixelShader_Desc pixelShaderDesc;
		pixelShaderDesc.pFunc = ps;
		_LoadIndex(fmtPSIn, &pixelShaderDesc.iPSInFormat);
		_LoadIndex(fmtPSOut, &pixelShaderDesc.iPSOutFormat);
		return pixelShaderDesc;
	}

	static inline VertexFormat_Desc		_VertexFormat_Create()
	{
		VertexFormat_Desc desc;

		for (VertexField & field : desc.vFields)
		{
			field.offset	= 0;
			field.type	= VertexFieldType::UNKNOWN;
		}
		desc.nFields	= 0;
		desc.nSize	= 0;
		desc.nAlign	= 0;

		return desc;
	}
	static inline void			_VertexFormat_AddField(VertexFormat_Desc * pDesc, VertexFieldType type)
	{
		ASSERT(pDesc && pDesc->nFields < NUM_MAX_VERTEX_FIELD);

		Integer nFieldAlign	= _GetVertexFieldAlignment(type);
		Integer nFieldSize	= _GetVertexFieldSize(type);
		Integer nFieldOffset	= AlignCeiling(pDesc->nSize, nFieldAlign);

		VertexField & field	= pDesc->vFields[pDesc->nFields];
		
		field.offset	= nFieldOffset;
		field.type	= type;
		pDesc->nAlign	= Max(pDesc->nAlign, nFieldAlign);
		pDesc->nSize	= AlignCeiling(pDesc->nSize + nFieldSize, pDesc->nAlign);
		pDesc->nFields	+= 1;
	}
	static inline void			_VertexFormat_Check(VertexFormat_Desc * pDesc)
	{
		/*
		ASSERT(pDesc && pDesc->nFields < NUM_MAX_VERTEX_FIELD);

		Integer nPositionField = 0;
		for (Integer i = 0; i < pDesc->nFields; ++i)
		{
			if (pDesc->vFields[i].type == VertexFieldType::POSITION)
				++nPositionField;
		}

		ASSERT(nPositionField == 1);
		*/
	}
	static inline bool			_VertexFormat_IsEqual(const VertexFormat_Desc * pLeft, const VertexFormat_Desc * pRight)
	{
		return memcmp(pLeft, pRight, sizeof(VertexFormat_Desc)) == 0;
	}

	static inline void			_Rasterize(RenderContext_Impl & context, const VertexFormat_Desc & vertexFormat, void * pVertexBegin, Integer nCount)
	{
		Buffer & frameBuffer = _GetBackBuffer(context);
		Buffer & depthBuffer = _GetDepthBuffer(context);
		Buffer & stencilBuffer = _GetStencilBuffer(context);

		bool depthEnable = context.stDepthStencil.depthEnable;
		bool stencilEnable = context.stDepthStencil.stencilEnable;
		bool depthWrite = (context.stDepthStencil.depthWriteMask == DepthWriteMask::ALL);
		Byte stencilWriteMask = context.stDepthStencil.stencilWriteMask;
		BlendState blendState = context.stBlend;

		bool flipHorizontal = context.bFlipHorizontal;

		Rect rect = _GetOutputTargetRect(context);
		
		Integer width = rect.right - rect.left;
		Integer height = rect.bottom - rect.top;

		VertexShader_Desc * pVSDesc = _GetVertexShaderDesc(context);
		PixelShader_Desc * pPSDesc = _GetPixelShaderDesc(context);

		auto vertexShader = pVSDesc->pFunc;
		auto pixelShader = pPSDesc->pFunc;
		ASSERT(vertexShader);
		ASSERT(pixelShader);

		Device_Impl * pDevice		= context.pDevice;

		VertexFormat_Desc * pVSFmtIn	= &pDevice->vertexFormatDescs[ pVSDesc->iVSInFormat.value ];
		VertexFormat_Desc * pVSFmtOut	= &pDevice->vertexFormatDescs[ pVSDesc->iVSOutFormat.value ];
		VertexFormat_Desc * pPSFmtIn	= &pDevice->vertexFormatDescs[ pPSDesc->iPSInFormat.value ];
		VertexFormat_Desc * pPSFmtOut	= &pDevice->vertexFormatDescs[ pPSDesc->iPSOutFormat.value ];

		ASSERT(vertexFormat.nFields >= 1 && vertexFormat.vFields[ 0 ].type == VertexFieldType::POSITION);
		ASSERT(_VertexFormat_IsEqual(&vertexFormat, pVSFmtIn));
		ASSERT(pPSFmtIn->nFields >= 2 && pPSFmtIn->vFields[ 0 ].type == VertexFieldType::POSITION && pPSFmtIn->vFields[ 1 ].type == VertexFieldType::SV_POSITION);
		ASSERT(pPSFmtOut->nFields == 1 && pPSFmtOut->vFields[ 0 ].type == VertexFieldType::COLOR);

		Byte * pVSIn			= ( Byte * ) pVertexBegin;
		Byte * pVSOut			= ( Byte * ) _aligned_malloc(pVSFmtOut->nSize * 3, pVSFmtOut->nAlign);
		Byte * pPSIn			= ( Byte * ) _aligned_malloc(pPSFmtIn->nSize, pPSFmtIn->nAlign);
		Byte * pPSOut			= ( Byte * ) _aligned_malloc(pPSFmtOut->nSize, pPSFmtOut->nAlign);
		const void * pVSData		= context.pVertexShaderData;
		const void * pPSData		= context.pPixelShaderData;
		
		for (;
		     nCount >= 3;
		     nCount -= 3, pVSIn += 3 * vertexFormat.nSize )
		{
			// World(Wld) -> Camera(Cam) -> NDC -> Screen(Scn)+Depth -> Raster(Ras)+Depth

			const void * pVSIn0 = pVSIn;
			const void * pVSIn1 = pVSIn + pVSFmtIn->nSize;
			const void * pVSIn2 = pVSIn + pVSFmtIn->nSize * 2;

			Byte * pVSOut0 = pVSOut;
			Byte * pVSOut1 = pVSOut + pVSFmtOut->nSize;
			Byte * pVSOut2 = pVSOut + pVSFmtOut->nSize * 2;

			vertexShader(pVSOut0, pVSIn0, pVSData);
			vertexShader(pVSOut1, pVSIn1, pVSData);
			vertexShader(pVSOut2, pVSIn2, pVSData);

			const Vector3 & p0Cam = reinterpret_cast< Vector3 * >( pVSOut0 )[ 0 ];
			const Vector3 & p1Cam = reinterpret_cast< Vector3 * >( pVSOut1 )[ 0 ];
			const Vector3 & p2Cam = reinterpret_cast< Vector3 * >( pVSOut2 )[ 0 ];

			const Vector3 & p0NDC = reinterpret_cast< Vector3 * >( pVSOut0 )[ 1 ];
			const Vector3 & p1NDC = reinterpret_cast< Vector3 * >( pVSOut1 )[ 1 ];
			const Vector3 & p2NDC = reinterpret_cast< Vector3 * >( pVSOut2 )[ 1 ];

			if (p0Cam.z <= 0.0f || p1Cam.z <= 0.0f || p2Cam.z <= 0.0f)
			{
				continue;
			}

			float z0CamInv = 1.0f / p0Cam.z;
			float z1CamInv = 1.0f / p1Cam.z;
			float z2CamInv = 1.0f / p2Cam.z;

			float z0NDCInv = 1.0f / p0NDC.z;
			float z1NDCInv = 1.0f / p1NDC.z;
			float z2NDCInv = 1.0f / p2NDC.z;

			Vector2 p0Scn = { ( p0NDC.x + 1.0f ) * 0.5f, ( 1.0f - p0NDC.y ) * 0.5f };
			Vector2 p1Scn = { ( p1NDC.x + 1.0f ) * 0.5f, ( 1.0f - p1NDC.y ) * 0.5f };
			Vector2 p2Scn = { ( p2NDC.x + 1.0f ) * 0.5f, ( 1.0f - p2NDC.y ) * 0.5f };

			Vector2 p0Ras = { p0Scn.x * width, p0Scn.y * height };
			Vector2 p1Ras = { p1Scn.x * width, p1Scn.y * height };
			Vector2 p2Ras = { p2Scn.x * width, p2Scn.y * height };

			Integer xRasMin = static_cast< Integer >( Min3(p0Ras.x, p1Ras.x, p2Ras.x) );
			Integer xRasMax = static_cast< Integer >( Max3(p0Ras.x, p1Ras.x, p2Ras.x) );
			Integer yRasMin = static_cast< Integer >( Min3(p0Ras.y, p1Ras.y, p2Ras.y) );
			Integer yRasMax = static_cast< Integer >( Max3(p0Ras.y, p1Ras.y, p2Ras.y) );

			xRasMin = Bound(( Integer ) 0, xRasMin, width);
			xRasMax = Bound(( Integer ) 0, xRasMax, width);
			yRasMin = Bound(( Integer ) 0, yRasMin, height);
			yRasMax = Bound(( Integer ) 0, yRasMax, height);
			if (xRasMax <= xRasMin || yRasMax <= yRasMin)
			{
				continue;
			}

			float areaInv = EdgeFunction(p0Ras, p1Ras, p2Ras);
			areaInv = ( areaInv < 0.0001f ) ? 1000.0f : 1.0f / areaInv;
			ASSERT(areaInv >= 0.0f);

			Integer xPix, yPix;
			float xPixF, yPixF;
			float xPixMinF = static_cast< float >( xRasMin );
			float yPixMinF = static_cast< float >( yRasMin );
			yPixF = static_cast< float >( yRasMin );
			for ( yPix = yRasMin, yPixF = yPixMinF; yPix < yRasMax; ++yPix, yPixF += 1.0f )
			{
				for ( xPix = xRasMin, xPixF = xPixMinF; xPix < xRasMax; ++xPix, xPixF += 1.0f )
				{
					// Intersection test
					Vector2 pixel = { xPixF, yPixF };

					float e0 = EdgeFunction(p1Ras, p2Ras, pixel);
					float e1 = EdgeFunction(p2Ras, p0Ras, pixel);
					float e2 = EdgeFunction(p0Ras, p1Ras, pixel);
					if ( e0 < 0 || e1 < 0 || e2 < 0 || ( e0 == 0 && e1 == 0 && e2 == 0 ) )
					{
						continue;
					}

					// Barycentric coordinate
					float bary0 = e0 * areaInv;
					float bary1 = e1 * areaInv;
					float bary2 = e2 * areaInv;
					ASSERT(0.0f <= bary0 && bary0 <= 1.0001f);
					ASSERT(0.0f <= bary1 && bary1 <= 1.0001f);
					ASSERT(0.0f <= bary2 && bary2 <= 1.0001f);
					ASSERT(( bary0 + bary1 + bary2 ) <= 1.0001f);

					// Z
					float zNDC = 1.0f / ( z0NDCInv * bary0 + z1NDCInv * bary1 + z2NDCInv * bary2 );
					// ASSERT(0.0f <= zNDC && zNDC <= 1.0001f);
					if ( !( 0.0f <= zNDC && zNDC <= 1.0001f ) )
					{
						continue;
					}

					Integer xPix2 = flipHorizontal ? ( width - xPix - 1 ) : xPix;

					// Depth test
					float * depth = static_cast< float * >( depthBuffer.At(rect.top + yPix, rect.left + xPix2) );
					if ( depthEnable && *depth <= zNDC )
					{
						continue;
					}

					// Stencil test
					Byte * stencil = static_cast< Byte * >( stencilBuffer.At(rect.top + yPix, rect.left + xPix2) );
					if ( stencilEnable && *stencil == 0 )
					{
						continue;
					}

					if ( depthWrite ) *depth = zNDC;
					if ( stencilWriteMask ) *stencil |= stencilWriteMask;

					// Vertex properties
					float zCam = 1.0f / ( z0CamInv * bary0 + z1CamInv * bary1 + z2CamInv * bary2 );
					float w0 = zCam * z0CamInv * bary0;
					float w1 = zCam * z1CamInv * bary1;
					float w2 = zCam * z2CamInv * bary2;
					ASSERT(0.0f <= w0 && w0 <= 1.0001f);
					ASSERT(0.0f <= w1 && w1 <= 1.0001f);
					ASSERT(0.0f <= w2 && w2 <= 1.0001f);
					ASSERT(( w0 + w1 + w2 ) <= 1.0001f);

					void * pVSField0;
					void * pVSField1;
					void * pVSField2;
					void * pPSField;
					for ( const VertexField & field : pPSFmtIn->vFields )
					{
						pVSField0 = pVSOut0 + field.offset;
						pVSField1 = pVSOut1 + field.offset;
						pVSField2 = pVSOut2 + field.offset;
						pPSField = pPSIn + field.offset;
						switch ( field.type )
						{
							case VertexFieldType::SV_POSITION:
								*static_cast< Vector3 * >( pPSField ) = { xPixF, yPixF, zNDC };
								break;
							case VertexFieldType::POSITION:
							case VertexFieldType::COLOR:
							case VertexFieldType::NORMAL:
							case VertexFieldType::MATERIAL:
								*static_cast< Vector3 * >( pPSField ) = WeightedAdd(*static_cast< Vector3 * >( pVSField0 ),
														 *static_cast< Vector3 * >( pVSField1 ),
														 *static_cast< Vector3 * >( pVSField2 ),
														 w0,
														 w1,
														 w2);
								break;
							case VertexFieldType::TEXCOORD:
								*static_cast< Vector2 * >( pPSField ) = WeightedAdd(*static_cast< Vector2 * >( pVSField0 ),
														 *static_cast< Vector2 * >( pVSField1 ),
														 *static_cast< Vector2 * >( pVSField2 ),
														 w0,
														 w1,
														 w2);
								break;
							case VertexFieldType::UNKNOWN:
							default:
								break;
						}
					}
					pixelShader(pPSOut, pPSIn, pPSData);

					Vector3 color = (*reinterpret_cast<Vector3 *>(pPSOut));
					ASSERT(color.x >= 0.0f && color.y >= 0.0f && color.z >= 0.0f);
					ASSERT(color.x <= 1.0001f && color.y <= 1.0001f && color.z <= 1.0001f);

					// Blend test
					Byte * bgr = ( Byte * ) frameBuffer.At(rect.top + yPix, rect.left + xPix2);
					if (blendState.blendEnable)
					{
						bgr[0] = bgr[0] / 2 + static_cast< Byte >( color.x * 255.0f * 0.5f );
						bgr[1] = bgr[1] / 2 + static_cast< Byte >( color.y * 255.0f * 0.5f );
						bgr[2] = bgr[2] / 2 + static_cast< Byte >( color.z * 255.0f * 0.5f );
						continue;
					}

					// Draw depth
					/*
					float fDepth = Bound(0.0f, *depth, 1.0f);
					fDepth *= fDepth;
					fDepth *= fDepth;
					fDepth *= fDepth;
					color = {fDepth, fDepth, fDepth};
					*/

					// Draw stencil
					/*
					float fStencil = (*stencil ? 1.0f : 0.0f);
					color = {fStencil, fStencil, fStencil};
					*/

					// Draw pixel
					Byte * pixelData = ( Byte * ) frameBuffer.At(rect.top + yPix, rect.left + xPix2);
					pixelData[ 0 ] = static_cast< Byte >( color.x * 255.0f );
					pixelData[ 1 ] = static_cast< Byte >( color.y * 255.0f );
					pixelData[ 2 ] = static_cast< Byte >( color.z * 255.0f );
				}
			}
		}

		_aligned_free(pVSOut);
		_aligned_free(pPSIn);
		_aligned_free(pPSOut);
	}

	Window			Window::Default()
	{
		static Window_Desc * pWindow = nullptr;
		if (!pWindow)
		{
			pWindow = new Window_Desc(L"Default", GetModuleHandle(NULL), 800, 600);
		}

		Window w;
		w.pImpl = (void *)pWindow;
		return w;
	}
	HWND			Window::GetHWND()
	{
		return static_cast<Window_Desc *>(pImpl)->GetHWND();
	}

	void			SwapChain::Swap()
	{
		Device_Impl *		pDevice;
		BufferIndex		iSwapChainDesc;
		SwapChain_Desc *	pSwapChainDesc;
		RenderTarget_Desc *	pRenderTargetDesc;
		RenderWindow *		pWindow;
		Buffer *		pBuffer;

		_LoadIndex(*this, &iSwapChainDesc);

		pDevice				= static_cast<Device_Impl *>(pParam);
		pSwapChainDesc			= &pDevice->swapChainDescs[iSwapChainDesc.value];
		pSwapChainDesc->bSwapped	= !pSwapChainDesc->bSwapped;

		pRenderTargetDesc		= &pDevice->renderTargetDescs[ pSwapChainDesc->iRenderTargetDesc.value ];

		Buffer & buffer	= _GetBackBuffer(*pDevice, *pSwapChainDesc);
		Integer nWidth	= buffer.Width();
		Integer nHeight	= buffer.Height();
		ASSERT(buffer.ElementSize() == 3);
		if ( pRenderTargetDesc->pUnknown->QueryInterface(&pWindow) )
		{
			ASSERT(pWindow->GetWidth() == nWidth &&
			       pWindow->GetHeight() == nHeight);
			
			NativeWindowBilt(pWindow->GetWindow(), FrameBuffer(), NATIVE_BLIT_BGR);
		}
		else
		{
			ENSURE_TRUE(pRenderTargetDesc->pUnknown->QueryInterface(&pBuffer));

			ASSERT(pBuffer->Width() == nWidth && pBuffer->Height() == nHeight);

			memcpy(pBuffer->Data(),
			       FrameBuffer(),
			       pBuffer->SizeInBytes());
		}
	}
	void			SwapChain::ResetBackBuffer(Byte value)
	{
		Device_Impl *		pDevice;
		DescIndex		iSwapChainDesc;
		SwapChain_Desc *	pSwapChainDesc;

		_LoadIndex(*this, &iSwapChainDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pSwapChainDesc		= &pDevice->swapChainDescs[ iSwapChainDesc.value ];

		_ResetBackBuffer(_GetBackBuffer(*pDevice, *pSwapChainDesc), value);
	}
	void			SwapChain::ResetBackBuffer(const Rect & rect, Byte value)
	{
		Device_Impl *		pDevice;
		DescIndex		iSwapChainDesc;
		SwapChain_Desc *	pSwapChainDesc;

		_LoadIndex(*this, &iSwapChainDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pSwapChainDesc		= &pDevice->swapChainDescs[ iSwapChainDesc.value ];

		Buffer & backBuffer	= _GetBackBuffer(*pDevice, *pSwapChainDesc);
		Integer nBytesPerRow	= (rect.right - rect.left) * backBuffer.ElementSize();
		for (Integer row = rect.top; row < rect.bottom; ++row)
		{
			FillMemory(backBuffer.At(row, rect.left),
				   nBytesPerRow,
				   value);
		}
	}
	void *			SwapChain::FrameBuffer()
	{
		Device_Impl *		pDevice;
		DescIndex		iSwapChainDesc;
		SwapChain_Desc *	pSwapChainDesc;

		_LoadIndex(*this, &iSwapChainDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pSwapChainDesc		= &pDevice->swapChainDescs[ iSwapChainDesc.value ];

		return _GetFrontBuffer(*pDevice, *pSwapChainDesc).Data();
	}

	Integer			VertexFormat::Alignment()
	{
		return _GetVertexFormatDesc(*this)->nAlign;
	}
	Integer			VertexFormat::Size()
	{
		return _GetVertexFormatDesc(*this)->nSize;
	}
	VertexField &		VertexFormat::operator[](Integer i)
	{
		return _GetVertexFormatDesc(*this)->vFields[i];
	}

	void *			VertexRange::At(Integer i)
	{
		ASSERT(0 <= i && i <= nVertexCount);

		return ((Byte *)pVertexBegin) + hVertexFormat.Size() * i;
	}
	Integer			VertexRange::Offset()
	{
		return nVertexOffset;
	}
	Integer			VertexRange::Count()
	{
		return nVertexCount;
	}

	VertexRange		VertexBuffer::Alloc(Integer nCount)
	{
		VertexBuffer_Desc * pVertexBufferDesc;
		Buffer * pBuffer;

		_GetVertexBufferDesc(*this, &pVertexBufferDesc, &pBuffer);
		
		ASSERT((pVertexBufferDesc->nAllocated + nCount) <= pBuffer->ElementCount());

		VertexFormat hVertexFormat;
		_StoreIndex(&hVertexFormat, pVertexBufferDesc->iVertexFormat);
		hVertexFormat.pParam = this->pParam;

		VertexRange vr;
		vr.nVertexOffset		= pVertexBufferDesc->nAllocated;
		vr.nVertexCount			= nCount;
		vr.hVertexFormat		= hVertexFormat;
		vr.pVertexBegin			= pBuffer->At(0, pVertexBufferDesc->nAllocated);

		pVertexBufferDesc->nAllocated	+= nCount;

		return vr;
	}
	void			VertexBuffer::Free(VertexRange v)
	{
		//ASSERT(false);
	}
	VertexFormat		VertexBuffer::GetVertexFormat()
	{
		Device_Impl * pDevice;
		VertexBuffer_Desc * pVertexBufferDesc;

		pDevice		= _GetDevice(*this);

		_GetVertexBufferDesc(*this, &pVertexBufferDesc, nullptr);

		VertexFormat handle;
		_StoreIndex(&handle, pVertexBufferDesc->iVertexFormat);
		handle.pParam = pDevice;
		return handle;
	}

	Integer			VertexBuffer::Count()
	{
		VertexBuffer_Desc * pVertexBufferDesc;
		_GetVertexBufferDesc(*this, &pVertexBufferDesc, nullptr);
		return pVertexBufferDesc->nAllocated;
	}
	void *			VertexBuffer::Data()
	{
		return _GetVertexBuffer(*this).Data();
	}

	void			DepthStencilBuffer::ResetDepthBuffer(float value)
	{
		Device_Impl *		pDevice;
		BufferIndex		iDepthStencilDesc;
		DepthStencil_Desc *	pDepthStencilDesc;

		_LoadIndex(*this, &iDepthStencilDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pDepthStencilDesc	= &pDevice->depthStencilDescs[ iDepthStencilDesc.value ];

		_ResetDepthBuffer(pDevice->buffers[ pDepthStencilDesc->iDepthBuffer.value ], value);
	}
	void			DepthStencilBuffer::ResetStencilBuffer(Byte value)
	{
		Device_Impl *		pDevice;
		BufferIndex		iDepthStencilDesc;
		DepthStencil_Desc *	pDepthStencilDesc;

		_LoadIndex(*this, &iDepthStencilDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pDepthStencilDesc	= &pDevice->depthStencilDescs[ iDepthStencilDesc.value ];

		_ResetStencilBuffer(pDevice->buffers[ pDepthStencilDesc->iStencilBuffer.value ], value);
	}

	void			Texture2D::Sample(float u, float v, float * pColor) const
	{
		//ASSERT(0.0f <= u && u <= 1.0001f);
		//ASSERT(0.0f <= v && v <= 1.0001f);
		//ASSERT(u + v <= 2.0f);

		const Device_Impl * pDevice		= _GetDevice(*this);
		const Texture2D_Desc * pTextureDesc	= _GetTextureDesc(*pDevice, *this);

		const Buffer & texData			= _GetBuffer(*pDevice, pTextureDesc->iTexDataBuffer);

		LONG width = texData.Width();
		LONG height = texData.Height();
		LONG col = static_cast< LONG >( width * u ) % width;
		LONG row = static_cast< LONG >( height * v ) % height;
		
		col = Bound(( LONG ) 0, col, width - 1);
		row = Bound(( LONG ) 0, row, height - 1);

		const Byte * bgra	= ( Byte * ) texData.At(row, col);

		pColor[ 0 ] = static_cast< float >( bgra[ 0 ] ) / 255.f;
		pColor[ 1 ] = static_cast< float >( bgra[ 1 ] ) / 255.f;
		pColor[ 2 ] = static_cast< float >( bgra[ 2 ] ) / 255.f;
	}

	Rect			RenderTarget::GetRect() const
	{
		return _GetRenderTargetDesc(*static_cast< Device_Impl * >( pParam ), *this)->rect;
	}
	Integer			RenderTarget::GetWidth() const
	{
		const Rect & rect = _GetRenderTargetDesc(*static_cast< Device_Impl * >( pParam ), *this)->rect;

		return rect.right - rect.left;
	}
	Integer			RenderTarget::GetHeight() const
	{
		const Rect & rect = _GetRenderTargetDesc(*static_cast< Device_Impl * >( pParam ), *this)->rect;

		return rect.bottom - rect.top;
	}
	bool			RenderTarget::QueryInterface(Integer iid, void ** ppvObject)
	{
		Device_Impl *		pDevice;
		DescIndex		iRenderTargetDesc;
		RenderTarget_Desc *	pRenderTargetDesc;

		_LoadIndex(*this, &iRenderTargetDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pRenderTargetDesc	= &pDevice->renderTargetDescs[ iRenderTargetDesc.value ];

		ASSERT(pRenderTargetDesc->pUnknown);

		return pRenderTargetDesc->pUnknown->QueryInterface(iid, ppvObject);
	}

	void			RenderContext::SetSwapChain(SwapChain sc)
	{
		_LoadIndex(sc,	&static_cast< RenderContext_Impl * >( pImpl )->iSwapChainDesc);
	}
	void			RenderContext::SetVertexShader(VertexShader vs)
	{
		_LoadIndex(vs,	&static_cast< RenderContext_Impl * >( pImpl )->iVertexShader);
	}
	void			RenderContext::SetPixelShader(PixelShader ps)
	{
		_LoadIndex(ps,	&static_cast< RenderContext_Impl * >( pImpl )->iPixelShader);
	}
	void			RenderContext::VSSetConstantBuffer(const void * pBuffer)
	{
		static_cast< RenderContext_Impl * >( pImpl )->pVertexShaderData = pBuffer;
	}
	void			RenderContext::PSSetConstantBuffer(const void * pBuffer)
	{
		static_cast< RenderContext_Impl * >( pImpl )->pPixelShaderData = pBuffer;
	}
	void			RenderContext::SetDepthStencilBuffer(DepthStencilBuffer dsb)
	{
		_LoadIndex(dsb,	&static_cast< RenderContext_Impl * >( pImpl )->iDepthStencilDesc);
	}
	void			RenderContext::SetRenderTarget(RenderTarget target)
	{
		_LoadIndex(target, &static_cast< RenderContext_Impl * >( pImpl )->iRenderTargetDesc);
	}
	void			RenderContext::RSSetFlipHorizontal(bool bFlipHorizontal)
	{
		static_cast< RenderContext_Impl * >( pImpl )->bFlipHorizontal = bFlipHorizontal;
	}
	void			RenderContext::OMSetDepthStencilState(DepthStencilState st)
	{
		static_cast< RenderContext_Impl * >( pImpl )->stDepthStencil = st;
	}
	void			RenderContext::OMSetBlendState(BlendState bs)
	{
		static_cast< RenderContext_Impl * >( pImpl )->stBlend = bs;
	}
	DepthStencilBuffer	RenderContext::GetDepthStencilBuffer()
	{
		Device_Impl * pDevice = static_cast< Device_Impl * >( pParam );

		DepthStencilBuffer dsb;
		_StoreIndex(&dsb, static_cast< RenderContext_Impl * >( pImpl )->iDepthStencilDesc);
		dsb.pParam = pDevice;
		return dsb;
	}
	RenderTarget		RenderContext::GetRenderTarget()
	{
		Device_Impl * pDevice = static_cast< Device_Impl * >( pParam );

		RenderTarget target;
		_StoreIndex(&target, static_cast< RenderContext_Impl * >( pImpl )->iRenderTargetDesc);
		target.pParam = pDevice;
		return target;
	}
	void			RenderContext::Draw(VertexBuffer vb, Integer nOffset, Integer nCount)
	{
		RenderContext_Impl * self = static_cast< RenderContext_Impl * >( pImpl );

		DescIndex		iVertexBufferDesc;
		VertexBuffer_Desc *	pVertexBufferDesc;
		VertexFormat_Desc *	pVertexFormatDesc;
		Buffer *		pVertexBuffer;
		Byte *			pBytes;
		Integer			nVSize;

		_LoadIndex(vb, &iVertexBufferDesc);

		pVertexBufferDesc	= &self->pDevice->vertexBufferDescs[iVertexBufferDesc.value];
		pVertexBuffer		= &self->pDevice->buffers[pVertexBufferDesc->iVertexBuffer.value];
		pVertexFormatDesc	= &self->pDevice->vertexFormatDescs[pVertexBufferDesc->iVertexFormat.value];

		pBytes			= (Byte *)pVertexBuffer->Data();
		nVSize			= pVertexFormatDesc->nSize;

		_Rasterize(*self, *pVertexFormatDesc, (pBytes + nOffset * nVSize), nCount);
	}

	Device			Device::Default()
	{
		static Device_Impl deviceImpl;

		Device device;
		device.pImpl = &deviceImpl;
		return device;
	}	
	RenderContext		Device::CreateRenderContext()
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		Ptr<RenderContext_Impl> pRenderContextImpl = _CreateRenderContext(*self);
		RenderContext_Impl * pImpl = pRenderContextImpl.get();
		self->renderContextImpls.emplace_back(std::move(pRenderContextImpl));

		RenderContext handle;
		handle.pImpl = pImpl;
		handle.pParam = self;
		return handle;
	}
	SwapChain		Device::CreateSwapChain(RenderTarget renderTarget)
	{
		Device_Impl * self = static_cast<Device_Impl *>(pImpl);

		DescIndex iSwapChain;
		iSwapChain.value = self->swapChainDescs.size();

		DescIndex iRenderTargetDesc;
		_LoadIndex(renderTarget, &iRenderTargetDesc);
		self->swapChainDescs.emplace_back(_CreateSwapChain(*self, iRenderTargetDesc));

		SwapChain handle;
		_StoreIndex(&handle, iSwapChain);
		handle.pParam = self;
		return handle;
	}
	DepthStencilBuffer	Device::CreateDepthStencilBuffer(Integer width, Integer height)
	{
		Device_Impl * self = static_cast<Device_Impl *>(pImpl);

		DescIndex iDepthStencil;
		iDepthStencil.value = self->depthStencilDescs.size();

		self->depthStencilDescs.emplace_back(_CreateDepthStencilBuffer(*self, width, height));

		DepthStencilBuffer handle;
		_StoreIndex(&handle, iDepthStencil);
		handle.pParam = self;
		return handle;
	}
	RenderTarget		Device::CreateRenderTarget(IUnknown * pUnknown, const Rect & rect)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		DescIndex iRenderTargetDesc;
		iRenderTargetDesc.value = self->renderTargetDescs.size();

		self->renderTargetDescs.emplace_back(_CreateRenderTarget(*self, pUnknown, rect));

		RenderTarget handle;
		_StoreIndex(&handle, iRenderTargetDesc);
		handle.pParam = self;
		return handle;
	}
	RenderTarget		Device::CreateRenderTarget(Texture2D texture, const Rect & rect)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		Texture2D_Desc * pTextureDesc;
		Buffer * pBuffer;
		
		pTextureDesc	= _GetTextureDesc(*self, texture);
		pBuffer		= &_GetBuffer(*self, pTextureDesc->iTexDataBuffer);

		return CreateRenderTarget(pBuffer, rect);
	}
	RenderTarget		Device::CreateRenderTarget(RenderTarget renderTarget, const Rect & rectSub)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );
		
		DescIndex iOldRenderTargetDesc;
		DescIndex iRenderTargetDesc;
		IUnknown * pUnknown;
		Rect rect;
		
		rect				= renderTarget.GetRect();

		ASSERT(rect.left <= rectSub.left && rectSub.left < rectSub.right && rectSub.right <= rect.right);
		ASSERT(rect.top <= rectSub.top && rectSub.top < rectSub.bottom && rectSub.bottom <= rect.bottom);

		iRenderTargetDesc.value		= self->renderTargetDescs.size();

		_LoadIndex(renderTarget, &iOldRenderTargetDesc);
		
		pUnknown			= self->renderTargetDescs[ iOldRenderTargetDesc.value ].pUnknown;
		
		self->renderTargetDescs.emplace_back(_CreateRenderTarget(*self, pUnknown, rectSub));

		RenderTarget handle;
		_StoreIndex(&handle, iRenderTargetDesc);
		handle.pParam = self;
		return handle;
	}

	VertexFormat		Device::CreateVertexFormat(VertexFieldType type0)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		VertexFormat_Desc vertexFormatDesc = _VertexFormat_Create();
		_VertexFormat_AddField(&vertexFormatDesc, type0);
		_VertexFormat_Check(&vertexFormatDesc);

		DescIndex iVertexFormatDesc;
		iVertexFormatDesc.value = self->vertexFormatDescs.size();

		self->vertexFormatDescs.emplace_back(vertexFormatDesc);

		VertexFormat handle;
		_StoreIndex(&handle, iVertexFormatDesc);
		handle.pParam = self;
		return handle;
	}
	VertexFormat		Device::CreateVertexFormat(VertexFieldType type0, VertexFieldType type1)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		VertexFormat_Desc vertexFormatDesc = _VertexFormat_Create();
		_VertexFormat_AddField(&vertexFormatDesc, type0);
		_VertexFormat_AddField(&vertexFormatDesc, type1);
		_VertexFormat_Check(&vertexFormatDesc);

		DescIndex iVertexFormatDesc;
		iVertexFormatDesc.value = self->vertexFormatDescs.size();

		self->vertexFormatDescs.emplace_back(vertexFormatDesc);

		VertexFormat handle;
		_StoreIndex(&handle, iVertexFormatDesc);
		handle.pParam = self;
		return handle;
	}
	VertexFormat		Device::CreateVertexFormat(VertexFieldType type0, VertexFieldType type1, VertexFieldType type2)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		VertexFormat_Desc vertexFormatDesc = _VertexFormat_Create();
		_VertexFormat_AddField(&vertexFormatDesc, type0);
		_VertexFormat_AddField(&vertexFormatDesc, type1);
		_VertexFormat_AddField(&vertexFormatDesc, type2);
		_VertexFormat_Check(&vertexFormatDesc);

		DescIndex iVertexFormatDesc;
		iVertexFormatDesc.value = self->vertexFormatDescs.size();

		self->vertexFormatDescs.emplace_back(vertexFormatDesc);

		VertexFormat handle;
		_StoreIndex(&handle, iVertexFormatDesc);
		handle.pParam = self;
		return handle;
	}
	VertexFormat		Device::CreateVertexFormat(VertexFieldType type0, VertexFieldType type1, VertexFieldType type2, VertexFieldType type3)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		VertexFormat_Desc vertexFormatDesc = _VertexFormat_Create();
		_VertexFormat_AddField(&vertexFormatDesc, type0);
		_VertexFormat_AddField(&vertexFormatDesc, type1);
		_VertexFormat_AddField(&vertexFormatDesc, type2);
		_VertexFormat_AddField(&vertexFormatDesc, type3);
		_VertexFormat_Check(&vertexFormatDesc);

		DescIndex iVertexFormatDesc;
		iVertexFormatDesc.value = self->vertexFormatDescs.size();

		self->vertexFormatDescs.emplace_back(vertexFormatDesc);

		VertexFormat handle;
		_StoreIndex(&handle, iVertexFormatDesc);
		handle.pParam = self;
		return handle;
	}
	VertexFormat		Device::CreateVertexFormat(VertexFieldType type0, VertexFieldType type1, VertexFieldType type2, VertexFieldType type3, VertexFieldType type4)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		VertexFormat_Desc vertexFormatDesc = _VertexFormat_Create();
		_VertexFormat_AddField(&vertexFormatDesc, type0);
		_VertexFormat_AddField(&vertexFormatDesc, type1);
		_VertexFormat_AddField(&vertexFormatDesc, type2);
		_VertexFormat_AddField(&vertexFormatDesc, type3);
		_VertexFormat_AddField(&vertexFormatDesc, type4);
		_VertexFormat_Check(&vertexFormatDesc);

		DescIndex iVertexFormatDesc;
		iVertexFormatDesc.value = self->vertexFormatDescs.size();

		self->vertexFormatDescs.emplace_back(vertexFormatDesc);

		VertexFormat handle;
		_StoreIndex(&handle, iVertexFormatDesc);
		handle.pParam = self;
		return handle;
	}
	VertexBuffer		Device::CreateVertexBuffer(VertexFormat hVertexFormat)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		DescIndex iVertexFormatDesc;
		_LoadIndex(hVertexFormat, &iVertexFormatDesc);

		DescIndex iVertexBufferDesc;
		iVertexBufferDesc.value = self->vertexBufferDescs.size();

		self->vertexBufferDescs.emplace_back(_CreateVertexBuffer(*self, iVertexFormatDesc));

		VertexBuffer handle;
		_StoreIndex(&handle, iVertexBufferDesc);
		handle.pParam = self;
		return handle;
	}
	VertexShader		Device::CreateVertexShader(VertexShaderFunc vs, VertexFormat fmtVSIn, VertexFormat fmtVSOut)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		ShaderIndex iVertexShader;
		iVertexShader.value = self->vertexShaderDescs.size();

		self->vertexShaderDescs.emplace_back(_CreateVertexShader(*self, vs, fmtVSIn, fmtVSOut));

		VertexShader handle;
		_StoreIndex(&handle, iVertexShader);
		handle.pParam = self;
		return handle;
	}
	PixelShader		Device::CreatePixelShader(PixelShaderFunc ps, VertexFormat fmtPSIn, VertexFormat fmtPSOut)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		ShaderIndex iPixelShader;
		iPixelShader.value = self->pixelShaderDescs.size();

		self->pixelShaderDescs.emplace_back(_CreatePixelShader(*self, ps, fmtPSIn, fmtPSOut));

		PixelShader handle;
		_StoreIndex(&handle, iPixelShader);
		handle.pParam = self;
		return handle;
	}

	Texture2D		Device::CreateTexture2D(Integer width, Integer height, Integer elementSize, Integer alignment, Integer rowPadding, const void * pData)
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		BufferIndex iBuffer	= _CreateBuffer(*self, width, height, elementSize, alignment, rowPadding, pData);

		DescIndex iTextureDesc;
		iTextureDesc.value = self->textureDescs.size();

		self->textureDescs.emplace_back(_CreateTexture2D(*self, iBuffer));

		Texture2D handle;
		_StoreIndex(&handle, iTextureDesc);
		handle.pParam = self;
		return handle;
	}
}