#include "TestCases.h"
#include "../Core/Graphics.h"

using namespace Graphics;

extern void		TestGraphics_Buffer0(int argc, char * argv[]);
extern void		TestGraphics_Buffer1(int argc, char * argv[]);
extern void		TestGraphics_Clipping(int argc, char * argv[]);
extern void		TestGraphics_Rasterization(int argc, char * argv[]);

static TestCase		cases[] =
{
	{"buffer0",	TestGraphics_Buffer0},
	{"buffer1",	TestGraphics_Buffer1},
	{"clip",	TestGraphics_Clipping},
	{"raster",	TestGraphics_Rasterization},
};
TestSuitEntry(Graphics)

#define		WINDOW_WIDTH			(1600)
#define		WINDOW_HEIGHT			(1200)
#define		WINDOW_ASPECT_RATIO		(4.0f / 3.0f)
#define		BYTES_PER_PIXEL			(4) // bgra
#define		BYTES_PER_DEPTH			(4) // f32
#define		BYTES_PER_STENCIL		(1) // u8
#define		GREY				(0xff888888)
#define		RED				(0xffff0000)
#define		GREEN				(0xff00ff00)
#define		BLUE				(0xff0000ff)
#define		YELLOW				(0xffffff00)
#define		WHITE				(0xffffffff)

#define		COST(A, B)				(A) = (((A) + (B)) * 231) % 931;
#define		DRAW_LINE1(br, c, x0, y0, x1, y1)	Draw2DLine(&br, (c), (x0) * (0.02f * WINDOW_WIDTH) + WINDOW_WIDTH / 4, (y0) * (0.02f * WINDOW_WIDTH) + WINDOW_HEIGHT / 4, (x1) * (0.02f * WINDOW_WIDTH) + WINDOW_WIDTH / 4, (y1) * (0.02f * WINDOW_WIDTH) + WINDOW_HEIGHT / 4)
#define		DRAW_LINE2(br, c, p0, p1)		DRAW_LINE1(br, c, (p0).x, (p0).y, (p1).x, (p1).y)

struct Attribs
{
	Vector4 pos;
	Vector4 clr;
};
struct Varyings
{
	Vector4 clr;
};
struct Uniform
{
	Matrix44 view;
	Matrix44 proj;
};

NativeWindow *	gpMain				= nullptr;
Buffer1 *	gpBuf[ 3 ]			= { nullptr, nullptr, nullptr, };
int		gClipDrawIndex			= 0; // 0: all n: draw the Nth
int		gClipMode			= 0; // bit0:X, bit1:Y, bit2:Z

static int	Max(int a, int b)
{
	return a >= b ? a : b;
}
static int	Min(int a, int b)
{
	return a <= b ? a : b;
}
static f32	Area(const Vector4 & a, const Vector4 & b, const Vector4 & c)
{
	return fabsf(( c.x - a.x ) * ( b.y - a.y ) - ( c.y - a.y ) * ( b.x - a.x ));
}
static void	KeyboardDown_Clipping(int keycode)
{
	if ('0' <= keycode && keycode <= '9')
	{
		gClipDrawIndex = keycode - '0';
	}
	else if ( 0x60 <= keycode && keycode <= 0x69 ) // num pad
	{
		gClipDrawIndex = keycode - 0x60;
	}
	else
	{
		switch (keycode)
		{
			case 'A': gClipMode = 0; break;
			case 'X': gClipMode = ( gClipMode & ~0x1 ) | ( ( gClipMode ^ 0x1 ) & 0x1 ); break;
			case 'Y': gClipMode = ( gClipMode & ~0x2 ) | ( ( gClipMode ^ 0x2 ) & 0x2 ); break;
			case 'Z': gClipMode = ( gClipMode & ~0x4 ) | ( ( gClipMode ^ 0x4 ) & 0x4 ); break;
			case 'Q': NativeDestroyAllWindows(); break;
			default: break;
		}
	}
}
static void	KeyboardDown_Rasterization(int keycode)
{
	switch ( keycode )
	{
		case 'Q': NativeDestroyAllWindows(); break;
		default: break;
	}
}
static Vector4	VertexShadingFunc(const void * pAttribs, void * pVaryings, const void * pUniform)
{
	const Attribs & attribs = *(const Attribs *)pAttribs;
	const Uniform & uniform = *(const Uniform *)pUniform;
	Varyings & varyings = *(Varyings *)pVaryings;

	varyings.clr = attribs.clr;

	Vector4 clip = V4Transform(V4Transform(attribs.pos, uniform.view), uniform.proj);
	return clip;
}
static Vector4	PixelShadingFunc(void * pVaryings, const void * pUniform)
{
	Varyings & varyings = *(Varyings *)pVaryings;

	Vector4 color = varyings.clr;
	return color;
}

void		TestGraphics_Buffer0(int argc, char * argv[])
{
	if ( argc < 3 )
	{
		printf("Not enough arguments.\n");
		return;
	}

	const int ROUNDS = Max(0, atoi(argv[ 2 ]));
	const int WIDTH = Max(1, atoi(argv[ 0 ]));
	const int HEIGHT = Max(1, atoi(argv[ 1 ]));
	const int STRIDE = 4;
	const int RSTRIDE = STRIDE * WIDTH;
	const int CSTRIDE = STRIDE;
	const int COUNT = WIDTH * HEIGHT;
	const int SIZE = COUNT * STRIDE;

	Buffer1 buf = CreateBuffer(SIZE);

	BufferView bv = CreateBufferView(0, buf.nSize, STRIDE);
	BufferView2D bv2 = CreateBufferView2D(0, buf.nSize, RSTRIDE, CSTRIDE);

	i64 iTicks[ 5 ] = { 0, 0, 0, 0, 0 };
	i64 iCount[ 5 ] = { 0, 0, 0, 0, 0 };
	i64 iChksm[ 5 ] = { 0, 0, 0, 0, 0 };

	i64 iBegin;
	i64 iEnd;
	int * p;
	int * pEnd;
	BufferIt bi;
	BufferIt2D bi2;

	int iBlockSeq[ 2 ][ 5 ] =
	{
		{0, 1, 2, 3, 4},
		{1, 0, 3, 2, 4},
	};
	for ( int iRound = 0; iRound < ROUNDS; ++iRound )
	{
		for ( int iBlock : iBlockSeq[ iRound % 2 ] )
		{
			i64 & iCurrentTicks = iTicks[ iBlock ];
			i64 & iCurrentCount = iCount[ iBlock ];
			i64 & iCurrentChksm = iChksm[ iBlock ];
			switch ( iBlock )
			{
				case 0:
					memset(buf.pData, 24586257, SIZE);
					iBegin = NativeGetTick();
					p = ( int * ) buf.pData;
					for ( int x = 0; x < COUNT; ++x )
					{
						COST(p[ x ], iCurrentCount);
						++iCurrentCount;
					}
					iEnd = NativeGetTick();
					iCurrentTicks += iEnd - iBegin;
					for ( int i = 0; i < SIZE / 8; ++i ) iCurrentChksm ^= ( ( i64 * ) buf.pData )[ i ];
					break;
				case 1:
					memset(buf.pData, 24586257, SIZE);
					iBegin = NativeGetTick();
					bi = CreateBufferIt(&buf, &bv, COUNT);
					while ( BufferItGetInc(&bi, ( void ** ) ( &p )) )
					{
						COST(*p, iCurrentCount);
						++iCurrentCount;
					}
					iEnd = NativeGetTick();
					iCurrentTicks += iEnd - iBegin;
					for ( int i = 0; i < SIZE / 8; ++i ) iCurrentChksm ^= ( ( i64 * ) buf.pData )[ i ];
					break;
				case 2:
					memset(buf.pData, 24586257, SIZE);
					iBegin = NativeGetTick();
					p = ( int * ) buf.pData;
					for ( int y = 0; y < HEIGHT; ++y )
					{
						for ( int x = 0; x < WIDTH; ++x )
						{
							COST(p[ y * WIDTH + x ], iCurrentCount);
							++iCurrentCount;
						}
					}
					iEnd = NativeGetTick();
					iCurrentTicks += iEnd - iBegin;
					for ( int i = 0; i < SIZE / 8; ++i ) iCurrentChksm ^= ( ( i64 * ) buf.pData )[ i ];
					break;
				case 3:
					memset(buf.pData, 24586257, SIZE);
					iBegin = NativeGetTick();
					bi2 = CreateBufferIt2D(&buf, &bv2, 0, WIDTH, 0, HEIGHT);
					while ( BufferIt2DGetInc(&bi2, &bi) )
					{
						while ( BufferItGetInc(&bi, ( void ** ) ( &p )) )
						{
							COST(*p, iCurrentCount);
							++iCurrentCount;
						}
					}
					iEnd = NativeGetTick();
					iCurrentTicks += iEnd - iBegin;
					for ( int i = 0; i < SIZE / 8; ++i ) iCurrentChksm ^= ( ( i64 * ) buf.pData )[ i ];
					break;
				case 4:
					memset(buf.pData, 24586257, SIZE);
					iBegin = NativeGetTick();
					bi2 = CreateBufferIt2D(&buf, &bv2, 0, WIDTH, 0, HEIGHT);
					while ( BufferIt2DGetIncRaw(&bi2, ( void ** ) &p, ( void ** ) &pEnd) )
					{
						while ( p < pEnd )
						{
							COST(*p, iCurrentCount);
							++p;
							++iCurrentCount;
						}
					}
					iEnd = NativeGetTick();
					iCurrentTicks += iEnd - iBegin;
					for ( int i = 0; i < SIZE / 8; ++i ) iCurrentChksm ^= ( ( i64 * ) buf.pData )[ i ];
					break;
				default:
					break;
			}
		}
	}

	DestroyBuffer(&buf);

	printf("1D Raw:    %7.lld ticks (checksum: %lld records: %lld)\n", iTicks[ 0 ], iChksm[ 0 ], iCount[ 0 ]);
	printf("1D Buf:    %7.lld ticks (checksum: %lld records: %lld)\n", iTicks[ 1 ], iChksm[ 1 ], iCount[ 1 ]);
	printf("2D Raw:    %7.lld ticks (checksum: %lld records: %lld)\n", iTicks[ 2 ], iChksm[ 2 ], iCount[ 2 ]);
	printf("2D Buf:    %7.lld ticks (checksum: %lld records: %lld)\n", iTicks[ 3 ], iChksm[ 3 ], iCount[ 3 ]);
	printf("2D BufRaw: %7.lld ticks (checksum: %lld records: %lld)\n", iTicks[ 4 ], iChksm[ 4 ], iCount[ 4 ]);
}

void		TestGraphics_Buffer1(int argc, char * argv[])
{
	const u32 W = WINDOW_WIDTH;
	const u32 H = WINDOW_HEIGHT;
	const u32 EDGE = 100;
	const u32 CX[] = { W / 4, W * 3 / 4, W / 4, W * 3 / 4 };
	const u32 CY[] = { H / 4, H / 4, H * 3 / 4, H * 3 / 4 };

	Buffer1 bufColor = CreateBuffer(WINDOW_WIDTH * WINDOW_HEIGHT * BYTES_PER_PIXEL);
	BufferView2D bv2Color = CreateBufferView2D(0, bufColor.nSize, WINDOW_WIDTH * BYTES_PER_PIXEL, BYTES_PER_PIXEL);

	BufferRect brColor = CreateBufferRect(&bufColor, &bv2Color, 0, W, 0, H);
	BufferRect brLT    = CreateBufferRect(&bufColor, &bv2Color, CX[ 0 ] - EDGE, CX[ 0 ] + EDGE, CY[ 0 ] - EDGE, CY[ 0 ] + EDGE);
	BufferRect brRT    = CreateBufferRect(&bufColor, &bv2Color, CX[ 1 ] - EDGE, CX[ 1 ] + EDGE, CY[ 1 ] - EDGE, CY[ 1 ] + EDGE);
	BufferRect brLB    = CreateBufferRect(&bufColor, &bv2Color, CX[ 2 ] - EDGE, CX[ 2 ] + EDGE, CY[ 2 ] - EDGE, CY[ 2 ] + EDGE);
	BufferRect brRB    = CreateBufferRect(&bufColor, &bv2Color, CX[ 3 ] - EDGE, CX[ 3 ] + EDGE, CY[ 3 ] - EDGE, CY[ 3 ] + EDGE);

	Buffer2DSetU32(&brColor, GREY);
	Buffer2DSetU32(&brLT, WHITE);
	Buffer2DSetU32(&brRT, RED);
	Buffer2DSetU32(&brLB, GREEN);
	Buffer2DSetU32(&brRB, BLUE);

	if ( NativeInitialize() )
	{
		NativeWindow * pMain = NativeCreateWindow(L"Buffer Set Test", WINDOW_WIDTH, WINDOW_HEIGHT);

		if ( pMain )
		{
			NativeWindowBilt(pMain, bufColor.pData, NATIVE_BLIT_BGRA);

			while ( NativeGetWindowCount() > 0 )
			{
				NativeInputPoll();
				Sleep(10);
			}
			NativeDestroyWindow(pMain);
		}

		NativeTerminate();
	}
}

void		TestGraphics_Clipping(int argc, char * argv[])
{
	Buffer1 bufColor = CreateBuffer(WINDOW_WIDTH * WINDOW_HEIGHT * BYTES_PER_PIXEL);
	BufferView2D bv2Color = CreateBufferView2D(0, bufColor.nSize, WINDOW_WIDTH * BYTES_PER_PIXEL, BYTES_PER_PIXEL);
	
	BufferRect brXY = CreateBufferRect(&bufColor, &bv2Color, 0, WINDOW_WIDTH / 2, 0, WINDOW_HEIGHT / 2);
	BufferRect brXZ = CreateBufferRect(&bufColor, &bv2Color, WINDOW_WIDTH / 2, WINDOW_WIDTH, 0, WINDOW_HEIGHT / 2);
	BufferRect brZY = CreateBufferRect(&bufColor, &bv2Color, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, WINDOW_HEIGHT);
	
	struct Varyings
	{
		Vector4 color;
		Vector4 norm;
	};
	Vector4 inClipCoord[] =
	{
		{-4.0f, +0.0f, -3.0f, 1.0f},
		{+0.0f, +4.0f, +4.0f, 1.0f},
		{+4.0f, -3.0f, +0.0f, 1.0f},
	};
	Varyings inVaryings[] =
	{
		{inClipCoord[0], inClipCoord[0]},
		{inClipCoord[1], inClipCoord[1]},
		{inClipCoord[2], inClipCoord[2]},
	};
	int inCount = 3;

	Varyings outVaryings[ 64 ];
	Vector4 outClipCoord[ 64 ];
	int outCount = 0;

	f32 lt = -1.0f;
	f32 rt = +1.0f;
	f32 bt = -1.0f;
	f32 tp = +1.0f;
	f32 nr = -1.0f;
	f32 fr = +1.0f;
	f32 fXShift = 0.0f;
	f32 fYShift = 0.0f;
	f32 fZShift = 0.0f;

	int nDrawIndex = 0;
	int nClipMode = -1;

	if ( NativeInitialize() )
	{
		gpMain = NativeCreateWindow(L"Clipping Test (Q:quit, A/X/Y/Z:clip, 0-9:draw)", WINDOW_WIDTH, WINDOW_HEIGHT);

		NativeKeyboardCallbacks cbKeyboard;
		cbKeyboard.down = KeyboardDown_Clipping;
		cbKeyboard.up = nullptr;

		if ( gpMain )
		{
			NativeRegisterKeyboardCallbacks(gpMain, &cbKeyboard);

			while ( NativeGetWindowCount() > 0 )
			{
				NativeInputPoll();

				if (nDrawIndex != gClipDrawIndex - 1 || nClipMode != gClipMode)
				{
					if (nClipMode != gClipMode)
					{
						nClipMode = gClipMode;
						fXShift = ( nClipMode & 0x1 ) ? 1000.0f : 0.0f;
						fYShift = ( nClipMode & 0x2 ) ? 1000.0f : 0.0f;
						fZShift = ( nClipMode & 0x4 ) ? 1000.0f : 0.0f;
						outCount = Clip3DTriangle(lt - fXShift, rt + fXShift,
									  bt - fYShift, tp + fYShift,
									  nr - fZShift, fr + fZShift,
									  sizeof(Varyings),
									  inClipCoord, inVaryings,
									  outClipCoord, outVaryings);
						printf("%d triangles after clipping.\n", outCount / 3);
					}

					nDrawIndex = gClipDrawIndex - 1;

					Buffer2DSetU32(&brXY, GREY);
					Buffer2DSetU32(&brXZ, GREY);
					Buffer2DSetU32(&brZY, GREY);

					// draw clipping planes

					if ( !( nClipMode & 0x1 ) ) DRAW_LINE1(brXY, RED, lt, -5.0f, lt, 5.0f);
					if ( !( nClipMode & 0x1 ) ) DRAW_LINE1(brXY, RED, rt, -5.0f, rt, 5.0f);
					if ( !( nClipMode & 0x2 ) ) DRAW_LINE1(brXY, GREEN, -5.0f, bt, 5.0f, bt);
					if ( !( nClipMode & 0x2 ) ) DRAW_LINE1(brXY, GREEN, -5.0f, tp, 5.0f, tp);

					if ( !( nClipMode & 0x1 ) ) DRAW_LINE1(brXZ, RED, lt, -5.0f, lt, 5.0f);
					if ( !( nClipMode & 0x1 ) ) DRAW_LINE1(brXZ, RED, rt, -5.0f, rt, 5.0f);
					if ( !( nClipMode & 0x4 ) ) DRAW_LINE1(brXZ, BLUE, -5.0f, nr, 5.0f, nr);
					if ( !( nClipMode & 0x4 ) ) DRAW_LINE1(brXZ, BLUE, -5.0f, fr, 5.0f, fr);

					if ( !( nClipMode & 0x4 ) ) DRAW_LINE1(brZY, BLUE, nr, -5.0f, nr, 5.0f);
					if ( !( nClipMode & 0x4 ) ) DRAW_LINE1(brZY, BLUE, fr, -5.0f, fr, 5.0f);
					if ( !( nClipMode & 0x2 ) ) DRAW_LINE1(brZY, GREEN, -5.0f, bt, 5.0f, bt);
					if ( !( nClipMode & 0x2 ) ) DRAW_LINE1(brZY, GREEN, -5.0f, tp, 5.0f, tp);

					// draw original triangle

					DRAW_LINE1(brXY, WHITE, inClipCoord[ 0 ].x, inClipCoord[ 0 ].y, inClipCoord[ 1 ].x, inClipCoord[ 1 ].y);
					DRAW_LINE1(brXY, WHITE, inClipCoord[ 1 ].x, inClipCoord[ 1 ].y, inClipCoord[ 2 ].x, inClipCoord[ 2 ].y);
					DRAW_LINE1(brXY, WHITE, inClipCoord[ 2 ].x, inClipCoord[ 2 ].y, inClipCoord[ 0 ].x, inClipCoord[ 0 ].y);

					DRAW_LINE1(brXZ, WHITE, inClipCoord[ 0 ].x, inClipCoord[ 0 ].z, inClipCoord[ 1 ].x, inClipCoord[ 1 ].z);
					DRAW_LINE1(brXZ, WHITE, inClipCoord[ 1 ].x, inClipCoord[ 1 ].z, inClipCoord[ 2 ].x, inClipCoord[ 2 ].z);
					DRAW_LINE1(brXZ, WHITE, inClipCoord[ 2 ].x, inClipCoord[ 2 ].z, inClipCoord[ 0 ].x, inClipCoord[ 0 ].z);

					DRAW_LINE1(brZY, WHITE, inClipCoord[ 0 ].z, inClipCoord[ 0 ].y, inClipCoord[ 1 ].z, inClipCoord[ 1 ].y);
					DRAW_LINE1(brZY, WHITE, inClipCoord[ 1 ].z, inClipCoord[ 1 ].y, inClipCoord[ 2 ].z, inClipCoord[ 2 ].y);
					DRAW_LINE1(brZY, WHITE, inClipCoord[ 2 ].z, inClipCoord[ 2 ].y, inClipCoord[ 0 ].z, inClipCoord[ 0 ].y);
					
					// draw triangles after clipping
					int vbegin;
					int vend;
					if (0 <= nDrawIndex)
					{
						if (nDrawIndex < outCount / 3)
						{
							vbegin = nDrawIndex * 3;
							vend = vbegin + 3;
							printf("draw triangle #%d\n", nDrawIndex + 1);
						}
						else
						{
							vbegin = 0;
							vend = 0;
							printf("no triangle #%d.\n", nDrawIndex + 1);
						}
					}
					else
					{
						vbegin = 0;
						vend = outCount;
						printf("draw all triangles.\n");
					}
					for ( int tri = vbegin; tri < vend; tri += 3 )
					{
						printf("triangle #%d properties: area=%.2f\n", tri / 3 + 1, Area(outClipCoord[ tri + 0 ], outClipCoord[ tri + 1 ], outClipCoord[ tri + 2 ]));
						for (int i = 0; i < 3; ++i)
						{
							printf("\tpos=(%.2f, %.2f, %.2f, %.2f)", outClipCoord[ tri + i ].x, outClipCoord[ tri + i ].y, outClipCoord[ tri + i ].z, outClipCoord[ tri + i ].w );
							printf("\tvar=(");
							for (int f = 0; f < sizeof(Varyings) / sizeof(f32); ++f)
							{
								printf("%.2f, ", ((f32 *)&outVaryings[ tri + i ])[f]);
							}
							printf(")\n");
						}

						DRAW_LINE1(brXY, YELLOW, outClipCoord[ tri + 0 ].x, outClipCoord[ tri + 0 ].y, outClipCoord[ tri + 1 ].x, outClipCoord[ tri + 1 ].y);
						DRAW_LINE1(brXY, YELLOW, outClipCoord[ tri + 1 ].x, outClipCoord[ tri + 1 ].y, outClipCoord[ tri + 2 ].x, outClipCoord[ tri + 2 ].y);
						DRAW_LINE1(brXY, YELLOW, outClipCoord[ tri + 2 ].x, outClipCoord[ tri + 2 ].y, outClipCoord[ tri + 0 ].x, outClipCoord[ tri + 0 ].y);
						
						DRAW_LINE1(brXZ, YELLOW, outClipCoord[ tri + 0 ].x, outClipCoord[ tri + 0 ].z, outClipCoord[ tri + 1 ].x, outClipCoord[ tri + 1 ].z);
						DRAW_LINE1(brXZ, YELLOW, outClipCoord[ tri + 1 ].x, outClipCoord[ tri + 1 ].z, outClipCoord[ tri + 2 ].x, outClipCoord[ tri + 2 ].z);
						DRAW_LINE1(brXZ, YELLOW, outClipCoord[ tri + 2 ].x, outClipCoord[ tri + 2 ].z, outClipCoord[ tri + 0 ].x, outClipCoord[ tri + 0 ].z);
						
						DRAW_LINE1(brZY, YELLOW, outClipCoord[ tri + 0 ].z, outClipCoord[ tri + 0 ].y, outClipCoord[ tri + 1 ].z, outClipCoord[ tri + 1 ].y);
						DRAW_LINE1(brZY, YELLOW, outClipCoord[ tri + 1 ].z, outClipCoord[ tri + 1 ].y, outClipCoord[ tri + 2 ].z, outClipCoord[ tri + 2 ].y);
						DRAW_LINE1(brZY, YELLOW, outClipCoord[ tri + 2 ].z, outClipCoord[ tri + 2 ].y, outClipCoord[ tri + 0 ].z, outClipCoord[ tri + 0 ].y);
					}

					NativeWindowBilt(gpMain, bufColor.pData, NATIVE_BLIT_BGRA | NATIVE_BLIT_FLIP_V);
				}

				Sleep(10);
			}
			NativeDestroyWindow(gpMain);
		}

		NativeTerminate();
	}
}

void		TestGraphics_Rasterization(int argc, char * argv[])
{
	Buffer1 bufColor = CreateBuffer(WINDOW_WIDTH * WINDOW_HEIGHT * BYTES_PER_PIXEL);
	BufferView2D bv2Color = CreateBufferView2D(0, bufColor.nSize, WINDOW_WIDTH * BYTES_PER_PIXEL, BYTES_PER_PIXEL);
	BufferRect brColor = CreateBufferRect(&bufColor, &bv2Color, 0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

	Buffer2DSetU32(&brColor, GREY);

	Attribs vertices[3] =
	{
		{
			{-1.0f, -1.0f, 3.0f, 1.0f},
			{0.0f, 0.0f, 1.0f, 1.0f},
		},
		{
			{0.0f, 1.0f, 3.0f, 1.0f},
			{0.0f, 1.0f, 0.0f, 1.0f},
		},
		{
			{1.0f, -1.0f, 3.0f, 1.0f},
			{1.0f, 0.0f, 0.0f, 1.0f},
		},
	};
	Uniform uniform =
	{
		M44Identity(),
		M44PerspectiveFovLH(ConvertToRadians(90), WINDOW_ASPECT_RATIO, 0.1f, 1000.0f),
	};

	Draw3DTriangle(&brColor, VertexShadingFunc, PixelShadingFunc, sizeof(Attribs), sizeof(Varyings), &vertices, &uniform);

	if ( NativeInitialize() )
	{
		gpMain = NativeCreateWindow(L"Rasterization Test (Q:quit)", WINDOW_WIDTH, WINDOW_HEIGHT);

		NativeKeyboardCallbacks cbKeyboard;
		cbKeyboard.down = KeyboardDown_Rasterization;
		cbKeyboard.up = nullptr;

		if ( gpMain )
		{
			NativeRegisterKeyboardCallbacks(gpMain, &cbKeyboard);
			NativeWindowBilt(gpMain, bufColor.pData, NATIVE_BLIT_BGRA | NATIVE_BLIT_FLIP_V);

			while ( NativeGetWindowCount() > 0 )
			{
				NativeInputPoll();

				Sleep(10);
			}
			NativeDestroyWindow(gpMain);
		}

		NativeTerminate();
	}
}