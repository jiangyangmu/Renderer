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

namespace Graphics
{
	ROTriangle::ROTriangle(Vec3 pos0, Vec3 rgb0, Vec3 pos1, Vec3 rgb1, Vec3 pos2, Vec3 rgb2)
		: m_vertex { {pos0, rgb0}, {pos1, rgb1}, {pos2, rgb2} }
		, m_vertexRange()
		, m_refVertexBuffer(nullptr)
		, m_refContext(nullptr)
	{
	}
	ROTriangle::~ROTriangle()
	{
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}
	}
	void		ROTriangle::Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refContext		= &renderContext;
		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROTriangle::Draw()
	{
		m_refContext->Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROTriangle::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::COLOR;
	}

	ROTexRectangle::ROTexRectangle(Vec3 center, float width, float height)
		: m_vertexRange()
		, m_refVertexBuffer(nullptr)
		, m_refContext(nullptr)
	{
		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;

		Vertex vertices[] =
		{
			{{-halfWidth + center.x, -halfHeight + center.y, center.z}, {0.0f, 0.0f}},
			{{ halfWidth + center.x, -halfHeight + center.y, center.z}, {1.0f, 0.0f}},
			{{ halfWidth + center.x,  halfHeight + center.y, center.z}, {1.0f, 1.0f}},
			{{-halfWidth + center.x, -halfHeight + center.y, center.z}, {0.0f, 0.0f}},
			{{ halfWidth + center.x,  halfHeight + center.y, center.z}, {1.0f, 1.0f}},
			{{-halfWidth + center.x,  halfHeight + center.y, center.z}, {0.0f, 1.0f}},
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
	void		ROTexRectangle::Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refContext		= &renderContext;
		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROTexRectangle::Draw()
	{
		m_refContext->Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROTexRectangle::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::TEXCOORD;
	}

	ROCube::ROCube(Vec3 center, float size)
		: m_vertexRange()
		, m_refVertexBuffer(nullptr)
		, m_refContext(nullptr)
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
	void		ROCube::Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refContext		= &renderContext;
		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROCube::Draw()
	{
		m_refContext->Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROCube::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::TEXCOORD;
	}

	ROBlinnPhongCube::ROBlinnPhongCube(Vec3 center, float size)
		: m_vertexRange()
		, m_refVertexBuffer(nullptr)
		, m_refContext(nullptr)
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
	void		ROBlinnPhongCube::Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer)
	{
		ASSERT(IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refContext		= &renderContext;
		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(sizeof(m_vertex) / sizeof(Vertex));

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(m_vertex));
	}
	void		ROBlinnPhongCube::Draw()
	{
		m_refContext->Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}
	bool		ROBlinnPhongCube::IsVertexFormatCompatible(VertexFormat vertexFormat)
	{
		return vertexFormat.Size() == sizeof(Vertex) &&
			vertexFormat[ 0 ].type == VertexFieldType::POSITION &&
			vertexFormat[ 1 ].type == VertexFieldType::NORMAL;
	}
}