#include "Renderer.h"

#include "Scene.h"
#include "Common.h"

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

	ROCube::ROCube(Vec3 center, float size)
		: m_vertexRange()
		, m_refVertexBuffer(nullptr)
		, m_refContext(nullptr)
	{
		static const float gUnitCubeVertices[] =
		{
			// up
			-0.5f,  0.5f, -0.5f  ,  0.5f, 1.0f,
			 0.5f,  0.5f,  0.5f  ,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f  ,  0.5f, 0.0f,
			-0.5f,  0.5f, -0.5f  ,  0.5f, 1.0f,
			 0.5f,  0.5f, -0.5f  ,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f  ,  1.0f, 0.0f,

			// down
			-0.5f, -0.5f, -0.5f  ,  0.5f, 1.0f,
			-0.5f, -0.5f,  0.5f  ,  0.5f, 0.0f,
			 0.5f, -0.5f,  0.5f  ,  1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f  ,  0.5f, 1.0f,
			 0.5f, -0.5f,  0.5f  ,  1.0f, 0.0f,
			 0.5f, -0.5f, -0.5f  ,  1.0f, 1.0f,

			// right
			 0.5f, -0.5f, -0.5f  ,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f  ,  0.5f, 0.0f,
			 0.5f,  0.5f, -0.5f  ,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f  ,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f  ,  0.5f, 1.0f,
			 0.5f,  0.5f,  0.5f  ,  0.5f, 0.0f,

			// left
			-0.5f, -0.5f, -0.5f  ,  0.0f, 1.0f,
			-0.5f,  0.5f, -0.5f  ,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f  ,  0.5f, 0.0f,
			-0.5f, -0.5f, -0.5f  ,  0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f  ,  0.5f, 0.0f,
			-0.5f, -0.5f,  0.5f  ,  0.5f, 1.0f,

			// back
			-0.5f, -0.5f,  0.5f  ,  0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f  ,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f  ,  0.5f, 0.0f,
			-0.5f, -0.5f,  0.5f  ,  0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f  ,  0.5f, 0.0f,
			 0.5f, -0.5f,  0.5f  ,  0.5f, 1.0f,

			// front
			-0.5f, -0.5f, -0.5f  ,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f  ,  0.5f, 0.0f,
			-0.5f,  0.5f, -0.5f  ,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f  ,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f  ,  0.5f, 1.0f,
			 0.5f,  0.5f, -0.5f  ,  0.5f, 0.0f,
		};

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

}