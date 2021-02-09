#include "Graphics.h"
#include "Native.h"
#include "Common.h"

#define BUFFER_ALIGN_BYTES	(64)

namespace Graphics
{
	Buffer1		CreateBuffer(u32 nSize)
	{
		Buffer1 buf;
		buf.pData = ( u8 * ) AlignedMalloc(nSize, BUFFER_ALIGN_BYTES);
		buf.nSize = buf.pData ? nSize : 0;
		return buf;
	}
	void		DestroyBuffer(Buffer1 * pBuffer)
	{
		if (pBuffer->pData)
		{
			AlignedFree(pBuffer->pData);
			pBuffer->pData = nullptr;
			pBuffer->nSize = 0;
		}
	}

#define BUFFER_SET_IMPL(name, type) \
	void		Buffer2DSet##name(BufferIt2D * pIt, type value) \
	{ \
		type * pBegin; \
		type * pEnd; \
		ASSERT(pIt->nCStride == sizeof(value)); \
		while ( BufferIt2DGetIncRaw(pIt, ( void ** ) &pBegin, ( void ** ) &pEnd) ) \
		{ \
			std::uninitialized_fill(pBegin, pEnd, value); \
		} \
	}

	BUFFER_SET_IMPL(U8, u8);
	BUFFER_SET_IMPL(U32, u32);
	BUFFER_SET_IMPL(F32, f32);
}