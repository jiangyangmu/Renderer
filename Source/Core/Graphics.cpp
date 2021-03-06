#include "Graphics.h"
#include "Native.h"
#include "Common.h"

#define MAX_TRIANGLES_CLIP_3D	(64) // 2^6
#define BUFFER_ALIGN_BYTES	(64)

#define BUFFER_2D_SET_IMPL(name, type) \
	Buffer2DSet##name(const BufferRect * pRect, type value) \
	{ \
		type * pBegin; \
		type * pEnd; \
		BufferIt2D it; \
		it.pData	= pRect->pData; \
		it.nRCount	= pRect->nRCount; \
		it.nCCount	= pRect->nCCount; \
		it.nRStride	= pRect->nRStride; \
		it.nCStride	= pRect->nCStride; \
		ASSERT(pRect->nCStride == sizeof(value)); \
		while ( BufferIt2DGetIncRaw(&it, ( void ** ) &pBegin, ( void ** ) &pEnd) ) \
		{ \
			std::uninitialized_fill(pBegin, pEnd, value); \
		} \
	}

#define LERP_POS(dst, src1, src2, t)	F32Lerp((f32 *)&(dst), (const f32 *)&(src1), (const f32 *)&(src2), (t), sizeof(dst) / sizeof(f32))
#define LERP_V(dst, src1, src2, t, n)	F32Lerp((f32 *)(dst), (const f32 *)(src1), (const f32 *)(src2), (t), (n))
#define LERP(src1, src2, t)		V4Lerp((src1), (src2), (t))

#define CLIP_TRIANGLE(axis, w, side, vbytes, pi, vi, ci, po, vo, co) \
	do { \
		for (int tri = 0; tri < ci; tri += 3) \
		{ \
			co += ClipTriangle((axis), (w), (side), (vbytes), pi + tri, ((u8 *)vi) + tri * (vbytes), po + co, ((u8 *)vo) + co * (vbytes)); \
		} \
		std::swap(pi, po); \
		std::swap(vi, vo); \
		ci = co; \
		co = 0; \
	} while (0)

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

	void		BUFFER_2D_SET_IMPL(U8, u8);
	void		BUFFER_2D_SET_IMPL(U32, u32);
	void		BUFFER_2D_SET_IMPL(F32, f32);
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

	int		ClipTriangle(const int axis, const f32 w, const f32 side, const int vbytes, const Vector4 * pi, const void * vi_, Vector4 * po, void * vo_)
	{
		const f32 * vi;
		f32 * vo;
		int vn;
		f32 sdf[ 3 ];
		int in;
		int mp, mn;
		int l, m, r;
		f32 t1, t2;
		int cnt;

		vi = ( const f32 * ) vi_;
		vo = ( f32 * ) vo_;
		vn = vbytes / sizeof(f32);
		in = 0;
		cnt = 0;

		// X = [-W, W], Y = [-W, W], Z = [-N, F]
		sdf[ 0 ] = side * ( V4GetByIndex(pi[ 0 ], axis) - w );
		sdf[ 1 ] = side * ( V4GetByIndex(pi[ 1 ], axis) - w );
		sdf[ 2 ] = side * ( V4GetByIndex(pi[ 2 ], axis) - w );

		if ( sdf[ 0 ] >= 0.0f ) ++in, mp = 0; else mn = 0;
		if ( sdf[ 1 ] >= 0.0f ) ++in, mp = 1; else mn = 1;
		if ( sdf[ 2 ] >= 0.0f ) ++in, mp = 2; else mn = 2;

		switch ( in )
		{
			case 3: // +++: keep

				memcpy(po, pi, sizeof(Vector4) * 3);
				memcpy(vo, vi, vbytes * 3);

				cnt = 3;

				break;

			case 0: // ---: drop

				cnt = 0;

				break;

			case 1: // +--: clip

				m = mp;
				l = ( m + 2 ) % 3;
				r = ( m + 1 ) % 3;
				t1 = sdf[ l ] / ( sdf[ l ] - sdf[ m ] );
				t2 = sdf[ m ] / ( sdf[ m ] - sdf[ r ] );

				// interpolate clip coord
				LERP_POS(po[ 0 ], pi[ l ], pi[ m ], t1);
				po[ 1 ] = pi[ m ];
				LERP_POS(po[ 2 ], pi[ m ], pi[ r ], t2);

				// interpolate varyings
				LERP_V(vo + 0 * vn, vi + l * vn, vi + m * vn, t1, vn);
				memcpy(vo + 1 * vn, vi + m * vn, vbytes);
				LERP_V(vo + 2 * vn, vi + m * vn, vi + r * vn, t2, vn);

				cnt = 3;

				break;

			case 2: // ++-: clip

				m = mn;
				l = ( m + 2 ) % 3;
				r = ( m + 1 ) % 3;
				t1 = sdf[ l ] / ( sdf[ l ] - sdf[ m ] );
				t2 = sdf[ m ] / ( sdf[ m ] - sdf[ r ] );

				// interpolate clip coord
				po[ 0 ] = pi[ r ];
				po[ 1 ] = pi[ l ];
				LERP_POS(po[ 2 ], pi[ l ], pi[ m ], t1);
				po[ 3 ] = po[ 0 ];
				po[ 4 ] = po[ 2 ];
				LERP_POS(po[ 5 ], pi[ m ], pi[ r ], t2);

				// Lerp Varyings
				memcpy(vo + 0 * vn, vi + r * vn, vbytes);
				memcpy(vo + 1 * vn, vi + l * vn, vbytes);
				LERP_V(vo + 2 * vn, vi + l * vn, vi + m * vn, t1, vn);
				memcpy(vo + 3 * vn, vo + 0 * vn, vbytes);
				memcpy(vo + 4 * vn, vo + 2 * vn, vbytes);
				LERP_V(vo + 5 * vn, vi + m * vn, vi + r * vn, t2, vn);

				cnt = 6;

				break;

			default:

				break;
		}

		return cnt;
	}
	int		Clip2DTriangle(const f32 lt, const f32 rt, const f32 bt, const f32 tp, const int vbytes, const Vector4 * pInClipCoord, const void * pInVaryings, Vector4 * pOutClipCoord, void * pOutVaryings)
	{
		static Vector4 bufClipCoord[ 3 * 16 ]; // 16 triangles at maximum
		static f32 bufVaryings[ 3 * 16 * 24 ]; // 24 f32 elements per varyings at maximum

		ASSERT(vbytes < 24 * sizeof(f32));

		Vector4 * pi = bufClipCoord;
		Vector4 * po = pOutClipCoord;
		f32 * vi = bufVaryings;
		f32 * vo = ( f32 * ) pOutVaryings;
		int ci = 3;
		int co = 0;

		memcpy(bufClipCoord, pInClipCoord, 3 * sizeof(Vector4));
		memcpy(bufVaryings, pInVaryings, 3 * vbytes);

		CLIP_TRIANGLE(0, lt, +1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(0, rt, -1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(1, bt, +1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(1, tp, -1.0f, vbytes, pi, vi, ci, po, vo, co);

		if ( pi != pOutClipCoord )
		{
			memcpy(pOutClipCoord, bufClipCoord, ci * sizeof(Vector4));
			memcpy(pOutVaryings, bufVaryings, ci * vbytes);
		}

		return ci;
	}
	int		Clip3DTriangle(const f32 lt, const f32 rt, const f32 bt, const f32 tp, const f32 nr, const f32 fr, const int vbytes, const Vector4 * pInClipCoord, const void * pInVaryings, Vector4 * pOutClipCoord, void * pOutVaryings)
	{
		static Vector4 bufClipCoord[ 3 * 64 ]; // 64 triangles at maximum
		static f32 bufVaryings[ 3 * 64 * 24 ]; // 24 f32 elements per varyings at maximum

		ASSERT(vbytes < 24 * sizeof(f32));

		Vector4 * pi = bufClipCoord;
		Vector4 * po = pOutClipCoord;
		f32 * vi = bufVaryings;
		f32 * vo = ( f32 * ) pOutVaryings;
		int ci = 3;
		int co = 0;

		memcpy(bufClipCoord, pInClipCoord, 3 * sizeof(Vector4));
		memcpy(bufVaryings, pInVaryings, 3 * vbytes);

		CLIP_TRIANGLE(0, lt, +1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(0, rt, -1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(1, bt, +1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(1, tp, -1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(2, nr, +1.0f, vbytes, pi, vi, ci, po, vo, co);
		CLIP_TRIANGLE(2, fr, -1.0f, vbytes, pi, vi, ci, po, vo, co);

		if ( pi != pOutClipCoord )
		{
			memcpy(pOutClipCoord, bufClipCoord, ci * sizeof(Vector4));
			memcpy(pOutVaryings, bufVaryings, ci * vbytes);
		}

		return ci;
	}

	void		Draw3DTriangle(const BufferRect * pRect, Vector4(*pVertexShader)( const void *, void *, const void * ), Vector4(*pPixelShader)( void *, const void * ), const int nAttribsSize, const int nVaryingsSize, const void * pAttribs1, const void * pUniform)
	{
		const u8 * pAttribs;
		u8 * pVaryings;
		Vector4 clipCoord[3];
		Vector4 ndcCoord[3];
		Vector2 scnCoord[3];
		f32 scnDepth[3];

		pAttribs	= ( const u8 * ) pAttribs1;
		pVaryings	= ( u8 * ) malloc(3 * nVaryingsSize + nVaryingsSize);

		// vertex shading
		for (int i = 0; i < 3; ++i)
		{
			clipCoord[i] = pVertexShader(pAttribs + i * nAttribsSize, pVaryings + i * nVaryingsSize, pUniform);
		}

		// TODO: clipping

		// perspective divide
		for (int i = 0; i < 3; ++i)
		{
			ndcCoord[i] = V4Scale(clipCoord[i], 1.0f / clipCoord[i].w);
		}

		// backface culling
		if (V3CrossLH(ndcCoord[1].xyz - ndcCoord[0].xyz, ndcCoord[2].xyz - ndcCoord[0].xyz).z >= 0.0f) {
			return;
		}

		// viewport transformation
		for (int i = 0; i < 3; ++i)
		{
			scnCoord[i].x = ( ndcCoord[i].x + 1.0f ) * 0.5f * pRect->nCCount;
			scnCoord[i].y = ( ndcCoord[i].y + 1.0f ) * 0.5f * pRect->nRCount;
			scnDepth[i] = ( ndcCoord[i].z + 1.0f ) * 0.5f;
		}

		// rasterization
		BBox2 bb2 = BB2Create(scnCoord[0], scnCoord[1], scnCoord[2]);
		Vector2 lt = { bb2.xl + 0.5f, bb2.yl + 0.5f };
		Vector2 rb = { bb2.xh + 0.5f, bb2.yh + 0.5f };
		BaryCoord bc = BaryCoordCreate(scnCoord[0], scnCoord[1], scnCoord[2], lt);
		f32 fX, fY;
		u32 nX, nY;
		for ( fY = lt.y, nY = ( u32 ) lt.y; fY < rb.y; fY += 1.0f, ++nY, BaryCoordIncY(&bc) )
		{
			for ( fX = lt.x, nX = ( u32 ) lt.x; fX < rb.x; fX += 1.0f, ++nX, BaryCoordIncX(&bc) )
			{
				if (!BaryCoordIsInside(&bc))
				{
					continue;
				}

				Vector3 baryCoord = BaryCoordGet(&bc);
				
				// z interpolation
				f32 depth = 1.0f / (baryCoord.x / scnDepth[0] + baryCoord.y / scnDepth[1] + baryCoord.z / scnDepth[2]);

				// TODO: depth stencil test
				
				// attributes interpolation
				Vector3 weight = { depth * baryCoord.x / scnDepth[0], depth * baryCoord.y / scnDepth[1], depth * baryCoord.z / scnDepth[2]};

				for (int i = 0; i < (nVaryingsSize / sizeof(f32)); ++i)
				{
					*(f32 *)(pVaryings + 3 * nVaryingsSize + i * sizeof(f32)) =
						weight.x * *(f32 *)(pVaryings + 0 * nVaryingsSize + i * sizeof(f32)) +
						weight.y * *(f32 *)(pVaryings + 1 * nVaryingsSize + i * sizeof(f32)) +
						weight.z * *(f32 *)(pVaryings + 2 * nVaryingsSize + i * sizeof(f32));
				}

				// pixel shading
				Vector4 color = pPixelShader(pVaryings + 3 * nVaryingsSize, pUniform);
				
				// TODO: blending

				u32 bgra =( ( u32 ) ( color.x * 255.0f ) )
					| ( ( u32 ) ( color.y * 255.0f ) << 8 )
					| ( ( u32 ) ( color.z * 255.0f ) << 16 )
					| ( ( u32 ) ( color.w * 255.0f ) << 24 );

				Buffer2DSetAtU32(pRect, nX, nY, bgra);
			}
		}

		free(pVaryings);
	}
}