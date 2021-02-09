#include "TestCases.h"
#include "../Core/Graphics.h"

using namespace Graphics;

extern void		TestGraphics_Buffer0(int argc, char * argv[]);

static TestCase		cases[] =
{
	{"buffer0",	TestGraphics_Buffer0},
};
TestSuitEntry(Graphics)

#define		WINDOW_WIDTH (800)
#define		WINDOW_HEIGHT (600)
#define		BYTES_PER_PIXEL (4) // bgra
#define		BYTES_PER_DEPTH (4) // f32
#define		BYTES_PER_STENCIL (1) // u8
#define		GREY (0xff333333)
#define		RED (0xffff0000)
#define		GREEN (0xff00ff00)
#define		BLUE (0xff0000ff)
#define		WHITE (0xffffffff )

static f32	gVertexData[] =
{
	// pos, color
	0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
};

#define		COST(A, B) (A) = (((A) + (B)) * 231) % 931;

int		Max(int a, int b)
{
	return a >= b ? a : b;
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
						COST(p[x], iCurrentCount);
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