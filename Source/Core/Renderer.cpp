#include "Renderer.h"

#include "Scene.h"
#include "Common.h"

static const float gUnitCubeVertices[] =
{
	// up
	-0.5f,  0.5f, -0.5f  ,  0.5f, 0.0f,
	 0.5f,  0.5f,  0.5f  ,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f  ,  0.5f, 1.0f,
	-0.5f,  0.5f, -0.5f  ,  0.5f, 0.0f,
	 0.5f,  0.5f, -0.5f  ,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f  ,  1.0f, 1.0f,

	// down
	-0.5f, -0.5f, -0.5f  ,  0.5f, 0.0f,
	-0.5f, -0.5f,  0.5f  ,  0.5f, 1.0f,
	 0.5f, -0.5f,  0.5f  ,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f  ,  0.5f, 0.0f,
	 0.5f, -0.5f,  0.5f  ,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f  ,  1.0f, 0.0f,

	// right
	 0.5f, -0.5f, -0.5f  ,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f  ,  0.5f, 1.0f,
	 0.5f,  0.5f, -0.5f  ,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f  ,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f  ,  0.5f, 0.0f,
	 0.5f,  0.5f,  0.5f  ,  0.5f, 1.0f,

	// left
	-0.5f, -0.5f, -0.5f  ,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f  ,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f  ,  0.5f, 1.0f,
	-0.5f, -0.5f, -0.5f  ,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f  ,  0.5f, 1.0f,
	-0.5f, -0.5f,  0.5f  ,  0.5f, 0.0f,

	// back
	-0.5f, -0.5f,  0.5f  ,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f  ,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f  ,  0.5f, 1.0f,
	-0.5f, -0.5f,  0.5f  ,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f  ,  0.5f, 1.0f,
	 0.5f, -0.5f,  0.5f  ,  0.5f, 0.0f,

	// front
	-0.5f, -0.5f, -0.5f  ,  0.0f, 0.0f,
	 0.5f,  0.5f, -0.5f  ,  0.5f, 1.0f,
	-0.5f,  0.5f, -0.5f  ,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f  ,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f  ,  0.5f, 0.0f,
	 0.5f,  0.5f, -0.5f  ,  0.5f, 1.0f,
};
static const float gUnitCubeNorms[] =
{
	 0.0f,  1.0f,  0.0f,
	 0.0f, -1.0f,  0.0f,
	 1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	 0.0f,  0.0f,  1.0f,
	 0.0f,  0.0f, -1.0f,
};

static inline int64_t TickToMs2(int64_t tick)
{
	return tick / 1000;
}
static inline double TickToMs(int64_t tick)
{
	return (double)(tick) * 0.001;
}
static inline int64_t MsToTick(double ms)
{
	return (int64_t)(ms * 1000.0);
}

namespace Graphics
{
	void		RenderMainLoop(NativeWindow * pWindow, IRenderer * pRenderer)
	{
		const double dTickCostMin = 1000.0 / 200.0; // 220 FPS
		const int64_t iTickCostMin = MsToTick(dTickCostMin);
		int64_t nFrame;
		int64_t iTickBegin, iTickEnd;
		int64_t iTickCost;
		int64_t iTickFps;

		ASSERT(pWindow);
		ASSERT(pRenderer);

		iTickBegin = iTickEnd = NativeGetTick();
		iTickCost = 0;
		nFrame = 0;

		while ( NativeGetWindowCount() > 0 )
		{
			NativeInputPoll();

			if (nFrame != 0)
			{
				// Count frame N cost (end)
				iTickEnd = NativeGetTick();
				iTickCost = iTickEnd - iTickBegin;

				// Throttle frame rate
				if (iTickCost < iTickCostMin)
				{
					Sleep(TickToMs2(iTickCostMin - iTickCost));
				}

				// Present frame N
				pRenderer->Present();

				// Show frame N debug info
				{
					static double accu = 0.0;
					double cost = TickToMs(max(iTickCostMin, iTickCost));
					double fps = 1000.0 / cost;
					double fcost;

					accu += cost;
					if ( accu > 500.0 ) // update once every 0.5 second
					{
						fcost = TickToMs(iTickCost);

						printf("FPS=%.2lf Cost=%.2lf(ms) Frame=%lld\n",
						       fps,
						       cost,
						       nFrame);

						accu = 0.0;
					}
				}
			}

			// Count frame N+1 cost (begin)
			iTickFps = iTickBegin = NativeGetTick();

			// Reset drawing surface
			pRenderer->Clear();

			// Update and draw frame N+1
			pRenderer->Update(TickToMs(max(iTickCostMin, iTickCost)));
			pRenderer->Draw();

			++nFrame;
		}
	}

	ROTriangle::ROTriangle(Vector3 pos0, Vector3 rgb0, Vector3 pos1, Vector3 rgb1, Vector3 pos2, Vector3 rgb2)
		: m_vertex { {pos0, rgb0}, {pos1, rgb1}, {pos2, rgb2} }
		, m_vertexRange()
		, m_refVertexBuffer(nullptr)
	{
	}
	ROTriangle::~ROTriangle()
	{
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}
	}
	void		ROTriangle::Initialize(VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROTriangle::Draw(RenderContext & renderContext)
	{
		renderContext.Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROTriangle::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::COLOR;
	}

	ROTexRectangle::ROTexRectangle(Vector3 center, float width, float height, float uMin, float uMax, float vMin, float vMax)
		: m_vertexRange()
		, m_refVertexBuffer(nullptr)
	{
		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;

		Vertex vertices[] =
		{
			{{-halfWidth + center.x, -halfHeight + center.y, center.z}, {uMin, vMin}},
			{{ halfWidth + center.x, -halfHeight + center.y, center.z}, {uMax, vMin}},
			{{ halfWidth + center.x,  halfHeight + center.y, center.z}, {uMax, vMax}},
			{{-halfWidth + center.x, -halfHeight + center.y, center.z}, {uMin, vMin}},
			{{ halfWidth + center.x,  halfHeight + center.y, center.z}, {uMax, vMax}},
			{{-halfWidth + center.x,  halfHeight + center.y, center.z}, {uMin, vMax}},
		};
		memcpy(m_vertex, vertices, sizeof(vertices));
	}
	ROTexRectangle::~ROTexRectangle()
	{
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}
	}
	void		ROTexRectangle::Initialize(VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROTexRectangle::Draw(RenderContext & renderContext)
	{
		renderContext.Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROTexRectangle::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::TEXCOORD;
	}

	ROCube::ROCube(Vector3 center, float size)
		: m_vertexRange()
		, m_refVertexBuffer(nullptr)
	{
		constexpr Integer nCount = sizeof(m_vertex) / sizeof(Vertex);
		constexpr Integer nFields = sizeof(Vertex) / sizeof(float);

		for (Integer i = 0; i < nCount; ++i)
		{
			m_vertex[i] = Vertex
			{
				gUnitCubeVertices[i * nFields] * size + center.x,
				gUnitCubeVertices[i * nFields + 1] * size + center.y,
				gUnitCubeVertices[i * nFields + 2] * size + center.z,
				gUnitCubeVertices[i * nFields + 3],
				gUnitCubeVertices[i * nFields + 4],
			};
		}
	}
	ROCube::~ROCube()
	{
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}
	}
	void		ROCube::Initialize(VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROCube::Draw(RenderContext & renderContext)
	{
		renderContext.Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROCube::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::TEXCOORD;
	}

	ROBlinnPhongCube::ROBlinnPhongCube(Vector3 center, float size)
		: m_vertexRange()
		, m_refVertexBuffer(nullptr)
	{
		constexpr Integer nCount = sizeof(m_vertex) / sizeof(Vertex);

		for ( Integer i = 0; i < nCount; ++i )
		{
			m_vertex[ i ] = Vertex
			{
				gUnitCubeVertices[ i * 5 ] * size + center.x,
				gUnitCubeVertices[ i * 5 + 1 ] * size + center.y,
				gUnitCubeVertices[ i * 5 + 2 ] * size + center.z,
				gUnitCubeNorms[ ( i / 6 ) * 3 ],
				gUnitCubeNorms[ ( i / 6 ) * 3 + 1 ],
				gUnitCubeNorms[ ( i / 6 ) * 3 + 2 ],
			};
		}
	}
	ROBlinnPhongCube::~ROBlinnPhongCube()
	{
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}
	}
	void		ROBlinnPhongCube::Initialize(VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROBlinnPhongCube::Draw(RenderContext & renderContext)
	{
		renderContext.Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROBlinnPhongCube::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::NORMAL;
	}
}