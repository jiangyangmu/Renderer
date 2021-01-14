#include "Resource.h"

#include "RenderWindow.h"
#include "win32/Win32App.h"

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
		BufferIndex		iBuffers[ 2 ];
		bool			bSwapped;
	};

	struct DepthStencil_Desc
	{
		BufferIndex		iDepthBuffer;
		BufferIndex		iStencilBuffer;
	};

	struct VertexBuffer_Desc
	{
		BufferIndex		iVertexBuffer;
		VertexFormat		eFormat;
		Integer			nAllocated;
	};

	struct RenderTarget_Desc
	{
		IUnknown *		pUnknown;
		Rect			rect;
	};

	struct VertexShader_Desc
	{
		void *			pFunc;
	};
	struct PixelShader_Desc
	{
		void *			pFunc;
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

		// shader context
		Matrix4x4		viewTransform;
		Matrix4x4		projTransform;
	};

	struct Device_Impl
	{
		std::vector<Buffer>			buffers;

		std::vector<SwapChain_Desc>		swapChainDescs;
		std::vector<DepthStencil_Desc>		depthStencilDescs;
		std::vector<VertexBuffer_Desc>		vertexBufferDescs;
		//std::vector<IndexBuffer_Desc>		indexBuffers;

		std::vector<VertexShader_Desc>		vertexShaderDescs;
		std::vector<PixelShader_Desc>		pixelShaderDescs;

		std::vector<RenderTarget_Desc>		renderTargetDescs;

		std::vector<Ptr<RenderContext_Impl>>	renderContextImpls;
	};

	struct ShaderContext
	{
		Matrix4x4 * view;
		Matrix4x4 * proj;
	};

	struct VS_DEFAULT_POS_RGB_IN
	{
		Vec3 posWld;
		Vec3 color;
	};
	struct VS_DEFAULT_POS_RGB_OUT
	{
		Vec3 posNDC;
		Vec3 color;
	};
	struct PS_DEFAULT_POS_RGB_OUT
	{
		Vec3 color;
	};

	static inline void			_StoreIndex(Handle * h, const ResourceIndex & i)
	{
		h->pImpl = reinterpret_cast< void * >( i.value );
	}
	static inline void			_LoadIndex(const Handle & h, ResourceIndex * i)
	{
		i->value = reinterpret_cast< Integer >( h.pImpl );
	}

	static inline Integer			_GetVertexSize(VertexFormat format)
	{
		Integer size;
		switch (format)
		{
			case VertexFormat::POS_RGB:
				size = 24;
				break;
			default:
				size = 0;
				break;
		}
		return size;
	}
	static inline Integer			_GetVertexAlignment(VertexFormat format)
	{
		Integer alignment;
		switch ( format )
		{
			case VertexFormat::POS_RGB:
				alignment = 4;
				break;
			default:
				alignment = 1;
				break;
		}
		return alignment;
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
	static inline void *			_GetVertexShader(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		VertexShader_Desc *	pVertexShaderDesc;

		pDevice			= context.pDevice;
		pVertexShaderDesc	= &pDevice->vertexShaderDescs[ context.iVertexShader.value ];

		return pVertexShaderDesc->pFunc;
	}
	static inline void *			_GetPixelShader(RenderContext_Impl & context)
	{
		Device_Impl *		pDevice;
		PixelShader_Desc *	pPixelShaderDesc;

		pDevice			= context.pDevice;
		pPixelShaderDesc	= &pDevice->pixelShaderDescs[ context.iPixelShader.value ];

		return pPixelShaderDesc->pFunc;
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

	static inline void			_ResetBackBuffer(Buffer & b)
	{
		b.SetAll(0);
	}
	static inline void			_ResetDepthBuffer(Buffer & b)
	{
		b.SetAllAs<float>(1.0f);
	}
	static inline void			_ResetStencilBuffer(Buffer & b)
	{
		b.SetAll(1);
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
	static inline SwapChain_Desc		_CreateSwapChain(Device_Impl & device, Integer nWidth, Integer nHeight)
	{
		SwapChain_Desc sc;

		int rowPadding		= ( 4 - ( ( nWidth * 3 ) & 0x3 ) ) & 0x3;
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
	static inline VertexBuffer_Desc		_CreateVertexBuffer(Device_Impl & device, VertexFormat format)
	{
		VertexBuffer_Desc vb;

		vb.iVertexBuffer	= _CreateBuffer(device, 64, 1, _GetVertexSize(format), _GetVertexAlignment(format));
		vb.eFormat		= format;
		vb.nAllocated		= 0;

		return vb;
	}
	static inline RenderTarget_Desc		_CreateRenderTarget(Device_Impl & device, IUnknown * pUnknown, const Rect & rect)
	{
		RenderTarget_Desc target;

		target.pUnknown			= pUnknown;
		target.rect			= rect;

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
		context->viewTransform		= Matrix4x4::Identity();
		context->projTransform		= Matrix4x4::Identity();

		return Ptr<RenderContext_Impl>(context);
	}

	static VS_DEFAULT_POS_RGB_OUT		VS_DEFAULT_POS_RGB(VS_DEFAULT_POS_RGB_IN in, ShaderContext & context)
	{
		VS_DEFAULT_POS_RGB_OUT out;
		out.posNDC = Vec3::Transform(Vec3::Transform(in.posWld, *context.view), *context.proj);
		out.color = in.color;
		return out;
	}
	static PS_DEFAULT_POS_RGB_OUT		PS_DEFAULT_POS_RGB(VS_DEFAULT_POS_RGB_OUT in, ShaderContext & context)
	{
		PS_DEFAULT_POS_RGB_OUT out;
		out.color = in.color;
		return out;
	}

	static inline VertexShader_Desc		_CreateVertexShader(Device_Impl & device)
	{
		VertexShader_Desc vs;
		vs.pFunc = reinterpret_cast< void * >( VS_DEFAULT_POS_RGB );
		return vs;
	}
	static inline PixelShader_Desc		_CreatePixelShader(Device_Impl & device)
	{
		PixelShader_Desc ps;
		ps.pFunc = reinterpret_cast< void * >( PS_DEFAULT_POS_RGB );
		return ps;
	}

	using Vertex = VS_DEFAULT_POS_RGB_IN;
	static inline void			_Rasterize(RenderContext_Impl & context, VertexFormat vertexFormat, Vertex * pVertexBegin, Integer nCount)
	{
		Buffer & frameBuffer = _GetBackBuffer(context);
		Buffer & depthBuffer = _GetDepthBuffer(context);
		Buffer & stencilBuffer = _GetStencilBuffer(context);

		Rect rect = _GetOutputTargetRect(context);
		
		Integer width = rect.right - rect.left;
		Integer height = rect.bottom - rect.top;

		using VS = decltype(&VS_DEFAULT_POS_RGB);
		using PS = decltype(&PS_DEFAULT_POS_RGB);
		using VS_OUT = VS_DEFAULT_POS_RGB_OUT;
		using PS_IN = VS_OUT;

		auto vertexShader = reinterpret_cast<VS>(_GetVertexShader(context));
		auto pixelShader = reinterpret_cast<PS>(_GetPixelShader(context));
		ASSERT(vertexShader);
		ASSERT(pixelShader);

		ShaderContext shaderContext =
		{
			&context.viewTransform,
			&context.projTransform
		};

		for (const Vertex * pVertex = pVertexBegin;
		     nCount >= 3;
		     nCount -= 3, pVertex += 3 )
		{
			// World(Wld) -> Camera(Cam) -> NDC -> Screen(Scn)+Depth -> Raster(Ras)+Depth

			const Vertex & v0 = pVertex[ 0 ];
			const Vertex & v1 = pVertex[ 1 ];
			const Vertex & v2 = pVertex[ 2 ];

			VS_OUT vs0 = vertexShader(v0, shaderContext);
			VS_OUT vs1 = vertexShader(v1, shaderContext);
			VS_OUT vs2 = vertexShader(v2, shaderContext);

			const Vec3 & p0NDC = vs0.posNDC;
			const Vec3 & p1NDC = vs1.posNDC;
			const Vec3 & p2NDC = vs2.posNDC;

			float z0NDCInv = 1.0f / p0NDC.z;
			float z1NDCInv = 1.0f / p1NDC.z;
			float z2NDCInv = 1.0f / p2NDC.z;

			Vec2 p0Scn = { ( p0NDC.x + 1.0f ) * 0.5f, ( 1.0f - p0NDC.y ) * 0.5f };
			Vec2 p1Scn = { ( p1NDC.x + 1.0f ) * 0.5f, ( 1.0f - p1NDC.y ) * 0.5f };
			Vec2 p2Scn = { ( p2NDC.x + 1.0f ) * 0.5f, ( 1.0f - p2NDC.y ) * 0.5f };

			Vec2 p0Ras = { p0Scn.x * width, p0Scn.y * height };
			Vec2 p1Ras = { p1Scn.x * width, p1Scn.y * height };
			Vec2 p2Ras = { p2Scn.x * width, p2Scn.y * height };

			Integer xRasMin = static_cast< Integer >( Min3(p0Ras.x, p1Ras.x, p2Ras.x) );
			Integer xRasMax = static_cast< Integer >( Max3(p0Ras.x, p1Ras.x, p2Ras.x) );
			Integer yRasMin = static_cast< Integer >( Min3(p0Ras.y, p1Ras.y, p2Ras.y) );
			Integer yRasMax = static_cast< Integer >( Max3(p0Ras.y, p1Ras.y, p2Ras.y) );

			// ASSERT(0 <= xRasMin && xRasMin < width);
			// ASSERT(0 <= yRasMin && yRasMin < height);
			if ( xRasMin < 0 || width <= xRasMax || yRasMin < 0 || height <= yRasMax )
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
			for ( yPix = yRasMin, yPixF = yPixMinF; yPix <= yRasMax; ++yPix, yPixF += 1.0f )
			{
				for ( xPix = xRasMin, xPixF = xPixMinF; xPix <= xRasMax; ++xPix, xPixF += 1.0f )
				{
					// Intersection test
					Vec2 pixel = { xPixF, yPixF };
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

					// Depth test
					float * depth = static_cast< float * >( depthBuffer.At(yPix, xPix) );
					if ( *depth <= zNDC )
					{
						continue;
					}
					*depth = zNDC;

					// Stencil test
					bool visible = *static_cast< Byte * >( stencilBuffer.At(yPix, xPix) );
					if ( !visible )
					{
						continue;
					}

					// Vertex properties
					float w0 = zNDC * z0NDCInv * bary0;
					float w1 = zNDC * z1NDCInv * bary1;
					float w2 = zNDC * z2NDCInv * bary2;
					ASSERT(0.0f <= w0 && w0 <= 1.0001f);
					ASSERT(0.0f <= w1 && w1 <= 1.0001f);
					ASSERT(0.0f <= w2 && w2 <= 1.0001f);
					ASSERT(( w0 + w1 + w2 ) <= 1.0001f);

					PS_IN pin;
					switch ( vertexFormat )
					{
						case VertexFormat::POS_RGB:
							pin = PS_IN
							{
								{xPixF, yPixF, zNDC},
								WeightedAdd(v0.color, v1.color, v2.color, w0, w1, w2),
							};
							break;
						default:
							break;
					}

					RGB color = Vec3ToRGB(pixelShader(pin, shaderContext).color);
					ASSERT(color.r >= 0.0f && color.g >= 0.0f && color.b >= 0.0f);
					ASSERT(color.r <= 1.0001f && color.g <= 1.0001f && color.b <= 1.0001f);

					// Draw pixel
					Byte * pixelData = ( Byte * ) frameBuffer.At(rect.top + yPix, rect.left + xPix);
					pixelData[ 0 ] = static_cast< Byte >( color.r * 255.0f );
					pixelData[ 1 ] = static_cast< Byte >( color.g * 255.0f );
					pixelData[ 2 ] = static_cast< Byte >( color.b * 255.0f );
				}
			}
		}
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

		_LoadIndex(*this, &iSwapChainDesc);

		pDevice			= static_cast<Device_Impl *>(pParam);
		pSwapChainDesc		= &pDevice->swapChainDescs[iSwapChainDesc.value];

		pSwapChainDesc->bSwapped = !pSwapChainDesc->bSwapped;
	}
	void			SwapChain::ResetBackBuffer()
	{
		Device_Impl *		pDevice;
		BufferIndex		iSwapChainDesc;
		SwapChain_Desc *	pSwapChainDesc;

		_LoadIndex(*this, &iSwapChainDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pSwapChainDesc		= &pDevice->swapChainDescs[ iSwapChainDesc.value ];

		_ResetBackBuffer(_GetBackBuffer(*pDevice, *pSwapChainDesc));
	}
	void *			SwapChain::FrameBuffer()
	{
		Device_Impl *		pDevice;
		BufferIndex		iSwapChainDesc;
		SwapChain_Desc *	pSwapChainDesc;

		_LoadIndex(*this, &iSwapChainDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pSwapChainDesc		= &pDevice->swapChainDescs[ iSwapChainDesc.value ];

		return _GetFrontBuffer(*pDevice, *pSwapChainDesc).Data();
	}

	void *			VertexRange::At(Integer i)
	{
		ASSERT(0 <= i && i <= nVertexCount);
		return ((Byte *)pVertexBegin) + _GetVertexSize(eVertexFormat) * i;
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

		VertexRange vr;
		vr.nVertexOffset		= pVertexBufferDesc->nAllocated;
		vr.nVertexCount			= nCount;
		vr.eVertexFormat		= pVertexBufferDesc->eFormat;
		vr.pVertexBegin			= pBuffer->At(0, pVertexBufferDesc->nAllocated);

		pVertexBufferDesc->nAllocated	+= nCount;

		return vr;
	}
	void			VertexBuffer::Free(VertexRange v)
	{
		ASSERT(false);
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
	void			DepthStencilBuffer::Reset()
	{
		Device_Impl *		pDevice;
		BufferIndex		iDepthStencilDesc;
		DepthStencil_Desc *	pDepthStencilDesc;

		_LoadIndex(*this, &iDepthStencilDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pDepthStencilDesc	= &pDevice->depthStencilDescs[iDepthStencilDesc.value];

		_ResetDepthBuffer(pDevice->buffers[ pDepthStencilDesc->iDepthBuffer.value ]);
	}

	void			RenderTarget::InitCache()
	{
		Device_Impl *		pDevice;
		DescIndex		iRenderTargetDesc;
		RenderTarget_Desc *	pRenderTargetDesc;
		RenderWindow *		pWindow;

		_LoadIndex(*this, &iRenderTargetDesc);

		pDevice			= static_cast< Device_Impl * >( pParam );
		pRenderTargetDesc	= &pDevice->renderTargetDescs[ iRenderTargetDesc.value ];
		
		ENSURE_TRUE(pRenderTargetDesc->pUnknown->QueryInterface(&pWindow));

		pDescCache		= pRenderTargetDesc;
		pObjectCache		= pWindow;
	}
	Rect			RenderTarget::GetRect()
	{
		if ( !pDescCache )
		{
			InitCache();
		}

		return static_cast< RenderTarget_Desc * >( pDescCache )->rect;
	}
	Integer			RenderTarget::GetWidth()
	{
		if ( !pObjectCache )
		{
			InitCache();
		}

		return static_cast< RenderWindow * >( pObjectCache )->GetWidth();
	}
	Integer			RenderTarget::GetHeight()
	{
		if ( !pObjectCache )
		{
			InitCache();
		}

		return static_cast< RenderWindow * >( pObjectCache )->GetHeight();
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
	void			RenderContext::SetViewTransform(Matrix4x4 m)
	{
		static_cast< RenderContext_Impl * >( pImpl )->viewTransform = m;
	}
	void			RenderContext::SetProjectionTransform(Matrix4x4 m)
	{
		static_cast< RenderContext_Impl * >( pImpl )->projTransform = m;
	}
	void			RenderContext::SetVertexShader(VertexShader vs)
	{
		_LoadIndex(vs,	&static_cast< RenderContext_Impl * >( pImpl )->iVertexShader);
	}
	void			RenderContext::SetPixelShader(PixelShader ps)
	{
		_LoadIndex(ps,	&static_cast< RenderContext_Impl * >( pImpl )->iPixelShader);
	}
	void			RenderContext::SetDepthStencilBuffer(DepthStencilBuffer dsb)
	{
		_LoadIndex(dsb,	&static_cast< RenderContext_Impl * >( pImpl )->iDepthStencilDesc);
	}
	void			RenderContext::SetOutputTarget(RenderTarget target)
	{
		_LoadIndex(target, &static_cast< RenderContext_Impl * >( pImpl )->iRenderTargetDesc);
	}

	RenderTarget		RenderContext::GetOutputTarget()
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
		Buffer *		pVertexBuffer;
		Byte *			pBytes;
		Integer			nVSize;

		_LoadIndex(vb, &iVertexBufferDesc);

		pVertexBufferDesc	= &self->pDevice->vertexBufferDescs[iVertexBufferDesc.value];
		pVertexBuffer		= &self->pDevice->buffers[pVertexBufferDesc->iVertexBuffer.value];

		pBytes			= (Byte *)pVertexBuffer->Data();
		nVSize			= _GetVertexSize(pVertexBufferDesc->eFormat);

		_Rasterize(*self, pVertexBufferDesc->eFormat, (Vertex *)(pBytes + nOffset * nVSize), nCount);
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
	SwapChain		Device::CreateSwapChain(Integer width, Integer height, bool enableAutoResize)
	{
		Device_Impl * self = static_cast<Device_Impl *>(pImpl);

		DescIndex iSwapChain;
		iSwapChain.value = self->swapChainDescs.size();

		self->swapChainDescs.emplace_back(_CreateSwapChain(*self, width, height));

		SwapChain handle;
		_StoreIndex(&handle, iSwapChain);
		handle.pParam = self;
		return handle;
	}
	DepthStencilBuffer	Device::CreateDepthStencilBuffer(Integer width, Integer height, bool enableAutoResize)
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

	VertexShader		Device::CreateVertexShader()
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		ShaderIndex iVertexShader;
		iVertexShader.value = self->vertexShaderDescs.size();

		self->vertexShaderDescs.emplace_back(_CreateVertexShader(*self));

		VertexShader handle;
		_StoreIndex(&handle, iVertexShader);
		handle.pParam = self;
		return handle;
	}
	PixelShader		Device::CreatePixelShader()
	{
		Device_Impl * self = static_cast< Device_Impl * >( pImpl );

		ShaderIndex iPixelShader;
		iPixelShader.value = self->pixelShaderDescs.size();

		self->pixelShaderDescs.emplace_back(_CreatePixelShader(*self));

		PixelShader handle;
		_StoreIndex(&handle, iPixelShader);
		handle.pParam = self;
		return handle;
	}
	VertexBuffer		Device::CreateVertexBuffer(VertexFormat format)
	{
		Device_Impl * self = static_cast<Device_Impl *>(pImpl);

		DescIndex iVertexBufferDesc;
		iVertexBufferDesc.value = self->vertexBufferDescs.size();

		self->vertexBufferDescs.emplace_back(_CreateVertexBuffer(*self, format));

		VertexBuffer handle;
		_StoreIndex(&handle, iVertexBufferDesc);
		handle.pParam = self;
		return handle;
	}
}