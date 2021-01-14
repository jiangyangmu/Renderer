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
		if ( m_refVertexBuffer )
		{
			m_refVertexBuffer->Free(m_vertexRange);
		}

		m_refContext		= &renderContext;
		m_refVertexBuffer	= &vertexBuffer;
		m_vertexRange		= m_refVertexBuffer->Alloc(3);

		memcpy(m_vertexRange.At(0), &m_vertex, sizeof(Vertex) * 3);
	}
	void		ROTriangle::Draw()
	{
		m_refContext->Draw(*m_refVertexBuffer, m_vertexRange.Offset(), m_vertexRange.Count());
	}

}