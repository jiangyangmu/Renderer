#pragma once

#include "_Math.h"

namespace Graphics
{
	struct Buffer1
	{
		u8 *	pData;
		u32	nSize;
	};
	struct BufferView
	{
		u32	nOffset;
		u32	nSize;
		u32	nStride;
	};
	struct BufferView2D
	{
		u32	nOffset;
		u32	nSize;
		u32	nRStride;
		u32	nCStride;
	};

	struct BufferIt
	{
		u8 *	pData;
		u32	nCount;
		u32	nStride;	// const
	};
	struct BufferIt2D
	{
		u8 *	pData;
		u32	nRCount;
		u32	nCCount;	// const
		u32	nRStride;	// const
		u32	nCStride;	// const
	};
	struct BufferRect
	{
		u8 *	pData;
		u32	nRCount;	// const
		u32	nCCount;	// const
		u32	nRStride;	// const
		u32	nCStride;	// const
	};

	Buffer1			CreateBuffer(u32 nSize);
	void			DestroyBuffer(Buffer1 * pBuffer);

	inline BufferView	CreateBufferView(u32 nOffset, u32 nSize, u32 nStride)
	{
		BufferView bv;
		bv.nOffset = nOffset;
		bv.nSize = nSize;
		bv.nStride = nStride;
		return bv;
	}
	inline BufferView2D	CreateBufferView2D(u32 nOffset, u32 nSize, u32 nRStride, u32 nCStride)
	{
		BufferView2D bv2;
		bv2.nOffset = nOffset;
		bv2.nSize = nSize;
		bv2.nRStride = nRStride;
		bv2.nCStride = nCStride;
		return bv2;
	}

	inline BufferIt		CreateBufferIt(Buffer1 * pBuffer, BufferView * pView, u32 nCount)
	{
		// check non-empty, boundary
		BufferIt bi;
		bi.pData = pBuffer->pData + pView->nOffset;
		bi.nCount = nCount;
		bi.nStride = pView->nStride;
		return bi;
	}
	inline BufferIt2D	CreateBufferIt2D(Buffer1 * pBuffer, BufferView2D * pView, u32 nLeft, u32 nRight, u32 nTop, u32 nBottom)
	{
		// check non-empty, boundary
		BufferIt2D bi2;
		bi2.pData = pBuffer->pData + pView->nOffset + pView->nRStride * nTop + pView->nCStride * nLeft;
		bi2.nRCount = nBottom - nTop;
		bi2.nCCount = nRight - nLeft;
		bi2.nRStride = pView->nRStride;
		bi2.nCStride = pView->nCStride;
		return bi2;
	}
	inline BufferRect	CreateBufferRect(Buffer1 * pBuffer, BufferView2D * pView, u32 nLeft, u32 nRight, u32 nTop, u32 nBottom)
	{
		// check non-empty, boundary
		BufferRect br;
		br.pData = pBuffer->pData + pView->nOffset + pView->nRStride * nTop + pView->nCStride * nLeft;
		br.nRCount = nBottom - nTop;
		br.nCCount = nRight - nLeft;
		br.nRStride = pView->nRStride;
		br.nCStride = pView->nCStride;
		return br;
	}

	void			Buffer2DSetU8(const BufferRect * pRect, u8 value);
	void			Buffer2DSetU32(const BufferRect * pRect, u32 value);
	void			Buffer2DSetF32(const BufferRect * pRect, f32 value);
	void			Buffer2DSetAtU32(const BufferRect * pRect, u32 nX, u32 nY, u32 value);

	inline bool		BufferItGetInc(BufferIt * pIt, void ** ppItem)
	{
		if ( pIt->nCount > 0 )
		{
			--pIt->nCount;
			*ppItem = ( void * ) pIt->pData;
			pIt->pData += pIt->nStride;
			return true;
		}
		else
		{
			return false;
		}
	}
	inline bool		BufferIt2DGetInc(BufferIt2D * pIt2, BufferIt * pIt)
	{
		if ( pIt2->nRCount > 0 )
		{
			--pIt2->nRCount;
			pIt->pData = pIt2->pData;
			pIt->nCount = pIt2->nCCount;
			pIt->nStride = pIt2->nCStride;
			pIt2->pData += pIt2->nRStride;
			return true;
		}
		else
		{
			return false;
		}
	}
	inline bool		BufferIt2DGetIncRaw(BufferIt2D * pIt2, void ** ppItemBegin, void ** ppItemEnd)
	{
		if ( pIt2->nRCount > 0 )
		{
			--pIt2->nRCount;
			*ppItemBegin = pIt2->pData;
			*ppItemEnd = pIt2->pData + pIt2->nCCount * pIt2->nCStride;
			pIt2->pData += pIt2->nRStride;
			return true;
		}
		else
		{
			return false;
		}
	}

	void			Draw2DLine(const BufferRect * pRect, const u32 color, int x0, int y0, int x1, int y1);

	int			ClipTriangle(const int iAxis, const f32 fW, const f32 fSide, const int nVaryingsSize, const Vector4 * pInClipCoord, const void * pInVaryings, Vector4 * pOutClipCoord, void * pOutVaryings);
	int			Clip2DTriangle(const f32 fLeft, const f32 fRight, const f32 fBottom, const f32 fTop, const int nVaryingsSize, const Vector4 * pInClipCoord, const void * pInVaryings, Vector4 * pOutClipCoord, void * pOutVaryings);
	int			Clip3DTriangle(const f32 fLeft, const f32 fRight, const f32 fBottom, const f32 fTop, const f32 fNear, const f32 fFar, const int nVaryingsSize, const Vector4 * pInClipCoord, const void * pInVaryings, Vector4 * pOutClipCoord, void * pOutVaryings);
}