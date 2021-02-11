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

	void		Buffer2DSetAtU32(const BufferRect * pRect, u32 nX, u32 nY, u32 value)
	{
		ASSERT(nY < pRect->nRCount && nX < pRect->nCCount);
		u32 * pData = ( u32 * ) ( pRect->pData + pRect->nRStride * nY + pRect->nCStride * nX );
		*pData = value;
	}

	void		Draw2DLine(const BufferRect * pRect, const u32 color, int x0, int y0, int x1, int y1)
	{
		int dx = abs(x1 - x0);
		int sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0);
		int sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;
		int e2;
		while ( true )
		{
			Buffer2DSetAtU32(pRect, x0, y0, color);
			if ( x0 == x1 && y0 == y1 )
			{
				break;
			}
			e2 = 2 * err;
			if ( e2 >= dy )
			{
				err += dy;
				x0 += sx;
			}
			if ( e2 <= dx )
			{
				err += dx;
				y0 += sy;
			}
		}
	}
}