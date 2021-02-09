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
	inline BufferIt2D	CreateBufferIt2D(Buffer1 * pBuffer, BufferView2D * pView, u32 nRCount, u32 nCCount)
	{
		// check non-empty, boundary
		BufferIt2D bi2;
		bi2.pData = pBuffer->pData + pView->nOffset;
		bi2.nRCount = nRCount;
		bi2.nCCount = nCCount;
		bi2.nRStride = pView->nRStride;
		bi2.nCStride = pView->nCStride;
		return bi2;
	}

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
	inline bool		BufferIt2DGetInc(BufferIt2D * pIt, BufferIt * pItR)
	{
		if ( pIt->nRCount > 0 )
		{
			--pIt->nRCount;
			pItR->pData = pIt->pData;
			pItR->nCount = pIt->nCCount;
			pItR->nStride = pIt->nCStride;
			pIt->pData += pIt->nRStride;
			return true;
		}
		else
		{
			return false;
		}
	}
	inline bool		BufferIt2DGetIncRaw(BufferIt2D * pIt, void ** ppItemBegin, void ** ppItemEnd)
	{
		if ( pIt->nRCount > 0 )
		{
			--pIt->nRCount;
			*ppItemBegin = pIt->pData;
			pIt->pData += pIt->nRStride;
			*ppItemEnd = pIt->pData;
			return true;
		}
		else
		{
			return false;
		}
	}
}