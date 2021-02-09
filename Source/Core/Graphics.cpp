#include "Graphics.h"
#include "Native.h"

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
}